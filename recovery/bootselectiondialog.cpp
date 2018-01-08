/* Boot selection menu
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

#include "bootselectiondialog.h"
#include "ui_bootselectiondialog.h"
#include "config.h"
#include "json.h"
#include "util.h"
#include "ceclistener.h"
#include <stdio.h>
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


class SleepSimulator{
     QMutex localMutex;
     QWaitCondition sleepSimulator;
public:
    SleepSimulator()
    {
        localMutex.lock();
    }
    void sleep(unsigned long sleepMS)
    {
        sleepSimulator.wait(&localMutex, sleepMS);
    }
    void CancelSleep()
    {
        sleepSimulator.wakeAll();
    }
};

BootSelectionDialog::BootSelectionDialog(const QString &drive, const QString &defaultPartition, bool stickyBoot, bool dsi, QWidget *parent) :
    QDialog(parent),
    _countdown(11),
    _dsi(dsi),
    ui(new Ui::BootSelectionDialog),
    _inSelection(false),
    _drive(drive.toAscii())
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

    connect(cec, SIGNAL(keyPress(int)), this, SLOT(onKeyPress(int)));

    /* Also mount recovery partition as it may contain icons we need */
    if (QProcess::execute("mount -t vfat -o ro "+partdev(drive, 1)+" /mnt") != 0)
    {
        /* Not fatal if this fails */
    }
    QVariantList installed_os = Json::loadFromFile("/settings/installed_os.json").toList();
    QSize currentsize = ui->list->iconSize();

    QSettings settings("/settings/noobs.conf", QSettings::IniFormat, this);
    int oldSticky = settings.value("sticky_boot", 800).toInt();

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
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
         }
    }

    QString cmdline = getFileContents("/proc/cmdline");
    if (cmdline.contains("no_cursor"))
    {
        QApplication::setOverrideCursor(Qt::BlankCursor);
#ifdef Q_WS_QWS
        QWSServer::setCursorVisible( false );
#endif
    }

    if (ui->list->count() != 0)
    {
        // If default boot partition set then boot to that after 5 seconds
        QSettings settings("/settings/noobs.conf", QSettings::IniFormat, this);
        int partition = settings.value("default_partition_to_boot", defaultPartition).toInt();
        // But if we have a stickyBoot partition, use that instead.
        if (oldSticky != 800)
            partition=oldSticky;

        if (partition != 800)
        {
            cec->clearKeyPressed();

            //Check for bootmenutimeout override (default==11)
            QString cmdline = getFileContents("/proc/cmdline");
            QStringList args = cmdline.split(QChar(' '),QString::SkipEmptyParts);
            foreach (QString s, args)
            {
                if (s.contains("bootmenutimeout"))
                {
                    QStringList params = s.split(QChar('='));
                    _countdown = params.at(1).toInt() +1;
                }
            }

            // Start timer
            qDebug() << "Starting " << _countdown-1 << " second timer before booting into partition" << partition;
            _timer.setInterval(1000);
            connect(&_timer, SIGNAL(timeout()), this, SLOT(countdown()));
            _timer.start();
            countdown();
            ui->list->installEventFilter(this);

            // Select OS booted previously
            QString partnrStr = QString::number(partition);
            QString stickynrStr = QString::number(oldSticky);

            qDebug() << "partnrStr = " << partnrStr;
            qDebug() << "stickynrStr = " << stickynrStr;

            QRegExp partnrRx("([0-9]+)$");
            for (int i=0; i<ui->list->count(); i++)
            {
                QVariantMap m = ui->list->item(i)->data(Qt::UserRole).toMap();
                QString bootpart = m.value("partitions").toList().first().toString();
                qDebug() << "Partition " << i << " = " << bootpart;
                if (partnrRx.indexIn(bootpart) != -1)
                {
                    qDebug() << "partnrRx = " << partnrRx.cap(1);
                    if (partnrRx.cap(1) == partnrStr)
                    {
                        qDebug() << "Previous OS " << bootpart << " at " <<i;
                        ui->list->setCurrentRow(i);
                    }
                    if (partnrRx.cap(1) == stickynrStr)
                    {
                        qDebug() << "found sticky at " << i;
                        ui->list->item(i)->setCheckState(Qt::Checked);
                    }
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
    if (stickyBoot && (oldSticky!=800))
    {
        // StickyBoot is set, and user has not intervened, so boot it directly.
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
    qDebug() << "Booting partition" << partition << "DSI=" << _dsi;
    setRebootPartition(partition);
    if (_dsi)
        updateConfig4dsi(partition);
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
    QRegExp partnrRx("([0-9]+)$");
    if (partnrRx.indexIn(partition) == -1)
    {
        QMessageBox::critical(this, "noobs.conf corrupt", "Not a valid partition: "+partition);
        return;
    }
    int partitionNr    = partnrRx.cap(1).toInt();
    int oldpartitionNr = settings.value("default_partition_to_boot", 0).toInt();

    if (partitionNr != oldpartitionNr)
    {
        // Save OS boot choice as the new default
        QProcess::execute("mount -o remount,rw /settings");
        settings.setValue("default_partition_to_boot", partitionNr);
        settings.sync();
        QProcess::execute("mount -o remount,ro /settings");
    }

    /* Identify if any sticky checks have been set */
    int count = ui->list->count();
    int stickyBoot = 800;
    for (int i=0; i<count; i++)
    {
        QListWidgetItem *row = ui->list->item(i);
        Qt::CheckState state = row->checkState();
        if (state == Qt::Checked)
        {
            QVariantMap m = row->data(Qt::UserRole).toMap();
            QByteArray partition = m.value("partitions").toList().first().toByteArray();
            QRegExp partnrRx("([0-9]+)$");
            if (partnrRx.indexIn(partition) != -1)
            {
                stickyBoot = partnrRx.cap(1).toInt();
            }
        }
    }
    int oldSticky = settings.value("sticky_boot", 800).toInt();
    if (stickyBoot != oldSticky)
    {
        QProcess::execute("mount -o remount,rw /settings");
        if (stickyBoot != 800)
        {
            // Save sticky boot choice as the new default
            settings.setValue("sticky_boot", stickyBoot);
        }
        else
        {
            //delete sticky boot entry
            settings.remove("sticky_boot");
        }
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
    //qDebug() << event->type();
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Enter)
    {
        stopCountdown();
    }

    return QDialog::eventFilter(obj, event);
}

void BootSelectionDialog::stopCountdown()
{
    _timer.stop();
    setWindowTitle(tr("PINN - Select OS to boot"));
}

void BootSelectionDialog::countdown()
{
    setWindowTitle(tr("PINN - Previously selected OS will boot in %1 seconds").arg(--_countdown));
    if (cec->hasKeyPressed())
        stopCountdown();
    if (_countdown == 0)
    {
        _timer.stop();
        bootPartition();
    }
}

void BootSelectionDialog::updateConfig4dsi(QByteArray partition)
{
    QProcess *update = new QProcess(this);
    qDebug() <<"Configuring dsi";
    update->start(QString("sh -c \"tvservice -n\""));
    update->waitForFinished(4000);
    update->setProcessChannelMode(QProcess::MergedChannels);
    bool bHDMI=true;
    QString status = update->readAll();
    QByteArray qba = status.toAscii();
    const char * result= qba.constData();

    if (status.length())
    {
        if (strncmp("[E] No device present",result,21)==0)
            bHDMI=false;
    }
    else
        bHDMI=false;

    qDebug() << "tvservice name: "<< status << " " << bHDMI << " "<< status.length();

    QByteArray partstr = _drive;
    QString driveStr(_drive);
    if (driveStr.right(1).at(0).isDigit())
        partstr.append("p");
    partstr.append(partition);
    qDebug() << partstr;
    QProcess::execute("mkdir -p /tmp/3");

    QString mntcmd = "mount "+partstr+" /tmp/3";

    QProcess::execute(mntcmd);
    if (bHDMI)
    {   //HDMI attached
        qDebug() << "HDMI selected";
        QProcess::execute("sh -c \"cp /tmp/3/config.hdmi /tmp/3/config.txt\"");
    }
    else
    {   //Assume DSI
        qDebug() << "DSI selected";
        QProcess::execute("sh -c \"cp /tmp/3/config.dsi /tmp/3/config.txt\"");
    }
    fflush(stderr);
    sync();

    SleepSimulator s;
    s.sleep(1000);

    QProcess::execute("umount "+partstr);
}

/* Key on TV remote pressed */
void BootSelectionDialog::onKeyPress(int cec_code)
{
#ifdef Q_WS_QWS
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    int key=0;
    QPoint p = QCursor::pos();
    switch (cec_code)
    {
/* MOUSE SIMULATION */
    case CEC_User_Control_Select:
    {
        QWidget* widget = dynamic_cast<QWidget*>(QApplication::widgetAt(QCursor::pos()));
        if (widget)
        {
            QPoint pos = QCursor::pos();
            QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress,widget->mapFromGlobal(pos), Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
            QCoreApplication::sendEvent(widget,event);
            QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonRelease,widget->mapFromGlobal(pos), Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
            QCoreApplication::sendEvent(widget,event1);
            qApp->processEvents();
        }
    }
    case CEC_User_Control_Left:
        p.rx()-=10;
        QCursor::setPos(p);
        break;
    case CEC_User_Control_Right:
        p.rx()+=10;
        QCursor::setPos(p);
        break;
    case CEC_User_Control_Up:
        p.ry()-=10;
        QCursor::setPos(p);
        break;
    case CEC_User_Control_Down:
        p.ry()+=10;
        QCursor::setPos(p);
        break;
/* ARROW KEY SIMULATION */
    case CEC_User_Control_Play:
        key = Qt::Key_Enter;
        break;
    case CEC_User_Control_Exit:
        key = Qt::Key_Escape;
        break;
    case CEC_User_Control_ChannelUp:
        key = Qt::Key_Up;
        break;
    case CEC_User_Control_ChannelDown:
        key = Qt::Key_Down;
        break;
    default:
        break;
    }

    if (key)
    {
        // key press
        QWSServer::sendKeyEvent(0, key, modifiers, true, false);
        // key release
        QWSServer::sendKeyEvent(0, key, modifiers, false, false);
    }
#else
    qDebug() << "onKeyPress" << key;
#endif
}

void BootSelectionDialog::on_list_itemChanged(QListWidgetItem *item)
{
    //Make the checkboxes act like radio boxes - only 1 can be set
    if (!_inSelection)
    {
        _inSelection=true;
        Qt::CheckState state = item->checkState();
        int count = ui->list->count();
        for (int i=0; i<count; i++)
        {
            QListWidgetItem *row = ui->list->item(i);
            row->setCheckState(Qt::Unchecked);
        }
        item->setCheckState(state);
        _inSelection=false;
    }
}

void BootSelectionDialog::on_pushButton_clicked()
{
    accept();
}
