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
#include "countdownfilter.h"
#include "json.h"
#include "mydebug.h"
#include "sleepsimulator.h"
#include "util.h"
#include "ceclistener.h"
#include "joystick.h"

#include <stdio.h>
#include <QAbstractButton>
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
// #include <QWSServer>
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

BootSelectionDialog::BootSelectionDialog(
        const QString &drive,               //PINN drive where boot partitions are stored for DSI mod
        const QString &defaultPartition,    //Could be provided on cmdline. Only used if no previoous OS had been used.
        bool stickyBoot,                    //Use Stickyboot i.e. when NOT going through recovery menu
        bool dsi,                           //Adapt config.txt according to fitted monitor
        QWidget *parent) :
    QDialog(parent),
    _countdown(11),
    _dsi(dsi),
    ui(new Ui::BootSelectionDialog),
    _inSelection(false),
    _drive(drive.toLatin1())
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

    _altered=false;

    Kinput::setWindow("bootSelection");
    Kinput::setMenu("any");
    connect(cec, SIGNAL(keyPress(int,int)), this, SLOT(onKeyPress(int,int)));
    connect(joy, SIGNAL(joyPress(int,int)), this, SLOT(onJoyPress(int,int)));

    /* Also mount recovery partition as it may contain icons we need */
    if (QProcess::execute("mount -t vfat -o ro "+partdev(drive, 1)+" /mnt") != 0)
    {
        /* Not fatal if this fails */
    }
    installed_os = Json::loadFromFile("/settings/installed_os.json").toList();
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
        QString name = CORE(m.value("name").toString());
        if (canBootOs(name, m))
        {
            name=NICKNAME(m.value("name").toString());
            QListWidgetItem *item = new QListWidgetItem(icon, name+"\n"+m.value("description").toString(), ui->list);
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

    //Check for bootmenutimeout override (default==11)
    QStringList args = cmdline.split(QChar(' '),QString::SkipEmptyParts);
    foreach (QString s, args)
    {
        if (s.contains("bootmenutimeout"))
        {
            stickyBoot=false;
            QStringList params = s.split(QChar('='));
            _countdown = params.at(1).toInt() +1;
        }
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
            joy->clearKeyPressed();

            // Start timer
            qDebug() << "Starting " << _countdown-1 << " second timer before booting into partition" << partition;

            connect(&_counter, SIGNAL(countdownTick(int)), this, SLOT(countdown(int)));
            connect(&_counter, SIGNAL(countdownExpired()), this, SLOT(countdownExpired()) );
            ui->list->installEventFilter(&_counter);
            _counter.startCountdown(_countdown);

            // Select OS booted previously
            QString partnrStr = QString::number(partition);
            QString stickynrStr = QString::number(oldSticky);

            qDebug() << "partnr = " << partition;
            qDebug() << "stickynr = " << oldSticky;

            for (int i=0; i<ui->list->count(); i++)
            {
                QVariantMap m = ui->list->item(i)->data(Qt::UserRole).toMap();
                QByteArray ba_bootpart = m.value("partitions").toList().first().toString().toLocal8Bit();
                int bootpart = extractPartitionNumber( ba_bootpart );
                qDebug() << "Partition " << i << " = " << bootpart;

                if (bootpart == partition)
                {
                    qDebug() << "Previous OS " << bootpart << " at " <<i;
                    ui->list->setCurrentRow(i);
                }
                if (bootpart == oldSticky)
                {
                    qDebug() << "found sticky at " << i;
                    ui->list->item(i)->setCheckState(Qt::Checked);
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

void BootSelectionDialog::bootPartition()   //Boots whatever partition is set in default_partition_to_boot
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

    if (_altered)
    {
        QProcess::execute("mount -o remount,rw /settings");
        Json::saveToFile("/settings/installed_os.json", installed_os);
        QProcess::execute("mount -o remount,ro /settings");
    }

    QSettings settings("/settings/noobs.conf", QSettings::IniFormat, this);

    QVariantMap m = item->data(Qt::UserRole).toMap();
    QByteArray partition = m.value("partitions").toList().first().toByteArray();

    int partitionNr = extractPartitionNumber(partition); //Handle the int/hex partition number issue

    int oldpartitionNr = settings.value("default_partition_to_boot", 0).toInt();

    if (partitionNr != oldpartitionNr)
    {
        // Save OS boot choice as the new default
        QProcess::execute("mount -o remount,rw /settings");
        settings.setValue("default_partition_to_boot", partitionNr);
        settings.sync();
        QProcess::execute("mount -o remount,ro /settings");
    }

    /* Identify if any sticky checks have been set or changed */
    int count = ui->list->count();
    int stickyBoot = 800;
    for (int i=0; i<count; i++)
    {
        QListWidgetItem *row = ui->list->item(i);
        Qt::CheckState state = row->checkState();
        if (state == Qt::Checked)
        {
            QVariantMap m = row->data(Qt::UserRole).toMap();
            QByteArray partition = m.value("partitions").toList().first().toString().toLocal8Bit();

            int p = extractPartitionNumber(partition);
            if (p!= 800)
            {
                stickyBoot = p;
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

void BootSelectionDialog::setDisplayMode()
{
    MSG();

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
    MSG();
}

void BootSelectionDialog::countdown(int count)
{
    if (count)
        setWindowTitle(QString(tr("PINN - Previously selected OS will boot in %1 seconds")).arg(count));
    else
        setWindowTitle(tr("PINN - Select OS to boot"));

    if (cec->hasKeyPressed())
        _counter.stopCountdown();
}

void BootSelectionDialog::countdownExpired()
{
    QTimer::singleShot(1, this, SLOT(accept()));
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
    QByteArray qba = status.toLatin1();
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
void BootSelectionDialog::onKeyPress(int cec_code, int value)
{
    cec->process_cec(cec_code,value);
}
#if 1
void BootSelectionDialog::onJoyPress(int cec_code,int value)
{
    joy->process_joy(cec_code,value);
}
#endif
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
    qDebug()<<"Click!";
    accept();
}

void BootSelectionDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    Q_UNUSED(button);
    reject();
}

void BootSelectionDialog::on_pb_Up_clicked()
{
    int row = ui->list->currentRow();
    _counter.stopCountdown();
    if(row>0)
    {
        _altered=true;
        QListWidgetItem* pTemp = ui->list->takeItem(row);
        ui->list->insertItem(row-1,pTemp);
        ui->list->setCurrentRow(row-1);

        QVariant entry = installed_os.takeAt(row);
        installed_os.insert(row-1,entry);
    }
}

void BootSelectionDialog::on_pb_Down_clicked()
{
    int row = ui->list->currentRow();
    _counter.stopCountdown();
    if(row<ui->list->count()-1){
        _altered=true;
        QListWidgetItem* pTemp = ui->list->takeItem(row);
        ui->list->insertItem(row+1,pTemp);
        ui->list->setCurrentRow(row+1);

        QVariant entry = installed_os.takeAt(row);
        installed_os.insert(row+1,entry);
    }
}
