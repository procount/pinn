/* Boot selection menu
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 * Modified by @procount (c) 2019
 *
 * See LICENSE.txt for license details
 *
 */

#include "bootselectiondialog.h"
#include "ui_bootselectiondialog.h"
#include "ceclistener.h"
#include "config.h"
#include "json.h"
#include "util.h"
#include "ceclistener.h"
#include "joystick.h"

#include <QDebug>
#include <unistd.h>
#include <QDir>
#include <QMessageBox>

#include <QProcess>
#include <QListWidgetItem>
#include <QPushButton>
#include <QTimer>
#include <QSettings>
#include <QDesktopWidget>
#include <QScreen>
#include <QWSServer>
#include <QRegExp>
#include <QDebug>
#include <QMouseEvent>

#ifdef RASPBERRY_CEC_SUPPORT
extern "C" {
#include <interface/vmcs_host/vc_cecservice.h>
}
#endif

extern CecListener * cec;
extern joystick * joy;

BootSelectionDialog::BootSelectionDialog(const QString &drive, const QString &defaultPartition, QWidget *parent) :
    QDialog(parent),
    _countdown(11),
    ui(new Ui::BootSelectionDialog)
{
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    ui->setupUi(this);
    QRect s = QApplication::desktop()->screenGeometry();
    if (s.height() < 500)
        resize(s.width()-10, s.height()-100);
    QDir dir;
    dir.mkdir("/settings");

    if (QProcess::execute("mount -o remount,ro /settings") != 0
        && QProcess::execute("mount -t ext4 -o ro "+partdev(drive, SETTINGS_PARTNR)+" /settings") != 0)
    {
        QMessageBox::critical(this, tr("Cannot display boot menu"), tr("Error mounting settings partition"));
        return;
    }

    Kinput::setWindow("bootSelection");
    Kinput::setMenu("any");
    connect(cec, SIGNAL(keyPress(int,int)), this, SLOT(onKeyPress(int,int)));

    connect(joy, SIGNAL(joyPress(int,int)), this, SLOT(onJoyPress(int,int)));

    /* Also mount /dev/mmcblk0p1 as it may contain icons we need */
    if (QProcess::execute("mount -t vfat -o ro /dev/mmcblk0p1 /mnt") != 0)
    {
        /* Not fatal if this fails */
    }

    QVariantList installed_os = Json::loadFromFile("/settings/installed_os.json").toList();
    QSize currentsize = ui->list->iconSize();

    foreach (QVariant v, installed_os)
    {
        QVariantMap m = v.toMap();
        QString iconfilename = m.value("icon").toString();
        QIcon icon;

        if (!iconfilename.isEmpty() && QFile::exists(iconfilename))
        {
            icon = QIcon(iconfilename);
            QList<QSize> avs = icon.availableSizes();
            if (avs.isEmpty())
            {
                /* Icon file corrupt */
                icon = QIcon();
            }
            else
            {
                QSize iconsize = avs.first();

                if (iconsize.width() > currentsize.width() || iconsize.height() > currentsize.height())
                {
                    /* Make all icons as large as the largest icon we have */
                    currentsize = QSize(qMax(iconsize.width(), currentsize.width()),qMax(iconsize.height(), currentsize.height()));
                    ui->list->setIconSize(currentsize);
                }
            }
        }
        if (canBootOs(m.value("name").toString(), m))
        {
            QListWidgetItem *item = new QListWidgetItem(icon, m.value("name").toString()+"\n"+m.value("description").toString(), ui->list);
            item->setData(Qt::UserRole, m);
        }
    }

    if (ui->list->count() != 0)
    {
        // If default boot partition set then boot to that after 5 seconds
        QSettings settings("/settings/noobs.conf", QSettings::IniFormat, this);
        int partition = settings.value("default_partition_to_boot", defaultPartition).toInt();

        if (partition != 800)
        {
            cec->clearKeyPressed();
            joy->clearKeyPressed();

            // Start timer
            qDebug() << "Starting 10 second timer before booting into partition" << partition;
            _timer.setInterval(1000);
            connect(&_timer, SIGNAL(timeout()), this, SLOT(countdown()));
            _timer.start();
            countdown();
            ui->list->installEventFilter(this);

            // Select OS booted previously
            QString partnrStr = QString::number(partition);
            QRegExp partnrRx("([0-9]+)$");
            for (int i=0; i<ui->list->count(); i++)
            {
                QVariantMap m = ui->list->item(i)->data(Qt::UserRole).toMap();
                QString bootpart = m.value("partitions").toList().first().toString();
                if (partnrRx.indexIn(bootpart) != -1 && partnrRx.cap(1) == partnrStr)
                {
                    qDebug() << "Previous OS at" << bootpart;
                    ui->list->setCurrentRow(i);
                    break;
                }
            }
        }
        else
        {
            ui->list->setCurrentRow(0);
        }
    }
    if (ui->list->count() == 1)
    {
        // Only one OS, boot that
        qDebug() << "accepting";
        QTimer::singleShot(1, this, SLOT(accept()));
    }

}

BootSelectionDialog::~BootSelectionDialog()
{
    delete ui;
}

void BootSelectionDialog::bootPartition()
{
    QSettings settings("/settings/noobs.conf", QSettings::IniFormat, this);
    QByteArray partition = settings.value("default_partition_to_boot", 800).toByteArray();
    qDebug() << "Booting partition" << partition;
    setRebootPartition(partition);
    QDialog::accept();
}

void BootSelectionDialog::accept()
{
    QListWidgetItem *item = ui->list->currentItem();
    if (!item)
        return;

    QSettings settings("/settings/noobs.conf", QSettings::IniFormat, this);
    QVariantMap m = item->data(Qt::UserRole).toMap();
    QByteArray partition = m.value("partitions").toList().first().toByteArray();

    int partitionNr;
    QRegExp parttype("^PARTUUID");
    if (parttype.indexIn(partition) == -1)
    {
        // SD card style /dev/mmcblk0pDD
        QRegExp partnrRx("([0-9]+)$");
        if (partnrRx.indexIn(partition) == -1)
        {
            QMessageBox::critical(this, "installed_os.json corrupt", "Not a valid partition: "+partition);
            return;
        }
        partitionNr    = partnrRx.cap(1).toInt();
    }
    else
    {
        // USB style PARTUUID=000dbedf-XX
        QRegExp partnrRx("([0-9a-f][0-9a-f])$");
        if (partnrRx.indexIn(partition) == -1)
        {
            QMessageBox::critical(this, "installed_os.json corrupt", "Not a valid partition: "+partition);
            return;
        }
        bool ok;
        partitionNr    = partnrRx.cap(1).toInt(&ok, 16);
    }

    int oldpartitionNr = settings.value("default_partition_to_boot", 0).toInt();

    if (partitionNr != oldpartitionNr)
    {
        // Save OS boot choice as the new default
        QProcess::execute("mount -o remount,rw /settings");
        settings.setValue("default_partition_to_boot", partitionNr);
        settings.sync();
        QProcess::execute("mount -o remount,ro /settings");
    }

    bootPartition();
}

void BootSelectionDialog::on_list_activated(const QModelIndex &)
{
    accept();
}

void BootSelectionDialog::setDisplayMode()
{
#ifdef Q_WS_QWS
    QString cmd, mode;
    QSettings settings("/settings/noobs.conf", QSettings::IniFormat, this);

    /* Restore saved display mode */
    int modenr = settings.value("display_mode", 0).toInt();

    switch (modenr)
    {
    case 1:
        cmd  = "-e \'DMT 4 DVI\'";
        mode = tr("HDMI safe mode");
        break;
    case 2:
        cmd  = "-c \'PAL 4:3\'";
        mode = tr("composite PAL mode");
        break;
    case 3:
        cmd  = "-c \'NTSC 4:3\'";
        mode = tr("composite NTSC mode");
        break;

    default:
        return;
    }

    // Trigger framebuffer resize
    QProcess *presize = new QProcess(this);
    presize->start(QString("sh -c \"tvservice -o; tvservice %1;\"").arg(cmd));
    presize->waitForFinished(4000);

    // Update screen resolution with current value (even if we didn't
    // get what we thought we'd get)
    QProcess *update = new QProcess(this);
    update->start(QString("sh -c \"tvservice -s | cut -d , -f 2 | cut -d \' \' -f 2 | cut -d x -f 1;tvservice -s | cut -d , -f 2 | cut -d \' \' -f 2 | cut -d x -f 2\""));
    update->waitForFinished(4000);
    update->setProcessChannelMode(QProcess::MergedChannels);

    QTextStream stream(update);
    int xres = stream.readLine().toInt();
    int yres = stream.readLine().toInt();
    int oTop = 0, oBottom = 0, oLeft = 0, oRight = 0;
    getOverscan(oTop, oBottom, oLeft, oRight);
    QScreen::instance()->setMode(xres-oLeft-oRight, yres-oTop-oBottom, 16);

    // Update UI item locations
    QRect s = QApplication::desktop()->screenGeometry();
    if (s.height() < 400)
        resize(s.width()-10, s.height()-100);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    // Refresh screen
    qApp->processEvents();
    QWSServer::instance()->refresh();
#endif
}

bool BootSelectionDialog::eventFilter(QObject *obj, QEvent *event)
{
    qDebug() << event->type();
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Enter)
    {
        stopCountdown();
    }

    return QDialog::eventFilter(obj, event);
}

void BootSelectionDialog::stopCountdown()
{
    _timer.stop();
    setWindowTitle(tr("Select OS to boot"));
}

void BootSelectionDialog::countdown()
{
    setWindowTitle(tr("Previously selected OS will boot in %1 seconds").arg(--_countdown));
    if (cec->hasKeyPressed() || joy->hasKeyPressed())
        stopCountdown();
    if (_countdown == 0)
    {
        _timer.stop();
        bootPartition();
    }
}

/* Key on TV remote pressed */
void BootSelectionDialog::onKeyPress(int cec_code, int value)
{
    cec->process_cec(cec_code,value);
}

void BootSelectionDialog::onJoyPress(int cec_code,int value)
{
    joy->process_joy(cec_code,value);
}
