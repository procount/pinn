#include "config.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "backupdialog.h"
#include "backupthread.h"
#include "multiimagewritethread.h"
#include "multiimagedownloadthread.h"
#include "initdrivethread.h"
#include "fullfatthread.h"
#include "confeditdialog.h"
#include "progressslideshowdialog.h"
#include "languagedialog.h"
#include "json.h"
#include "util.h"
#include "twoiconsdelegate.h"
#include "wifisettingsdialog.h"
#include "passwd.h"
#include "piclonedialog.h"
#include "piclonethread.h"
#include "builddata.h"
#include "ceclistener.h"
#include "joystick.h"
#include "osgroup.h"
#include "fscheck.h"
#include "repair.h"
#include "countdownfilter.h"
#include "replace.h"
#include "renamedialog.h"
#include "splash.h"
#include "datetimedialog.h"
#include "iconcache.h"
#include "termsdialog.h"
#include "simulate.h"
#include "dlginstall.h"
#include "sleepsimulator.h"
#include "adjustsizes.h"

#define LOCAL_DBG_ON   0
#define LOCAL_DBG_FUNC 0
#define LOCAL_DBG_OUT  0
#define LOCAL_DBG_MSG  0

#include "mydebug.h"

#include <QByteArray>
#include <QDateTime>
#include <QMessageBox>
#include <QProgressDialog>
#include <QMap>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
//#include <QSplashScreen>
#include <QDesktopWidget>
#include <QSettings>
#include <QShortcut>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtDBus/QDBusConnection>
#include <QVariantMap>
#include <QHostInfo>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <sys/time.h>

#ifdef RASPBERRY_CEC_SUPPORT
extern "C" {
#include <interface/vmcs_host/vc_cecservice.h>
}
#endif

#ifdef Q_WS_QWS
#include <QWSServer>
#endif

extern CecListener * cec;
extern simulate * sim;
extern joystick * joy1;
extern joystick * joy2;

extern QStringList downloadRepoUrls;
extern QString repoList;

int fontsize=11;

/* Main window
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

//TODO Change to enums
#define TOOLBAR_MAIN 0
#define TOOLBAR_ARCHIVAL 1
#define TOOLBAR_MAINTENANCE 2
#define NUM_TOOLBARS 3

/* time in ms to poll for new disks */
#define POLLTIME 1000

#define MYCOUNT 1

/* Flag to keep track wheter or not we already repartitioned. */
bool MainWindow::_partInited = false;

/* Flag to keep track of current display mode. */
int MainWindow::_currentMode = 0;

extern bool timedReboot;
void MainWindow::tick(int secs)
{
    Q_UNUSED(secs);
    update_window_title();
}

void MainWindow::expired(void)
{
    //Close any open messageboxes
    int numDialogs=0;

    numDialogs=closeDialogs();

    if (numDialogs)
    {
        QTimer::singleShot(1000,this,SLOT(expired()));
        return;
    }

    timedReboot=true;
    close();
    QApplication::quit();  //@@ test?
}

MainWindow::MainWindow(const QString &drive, const QString &defaultDisplay, KSplash *splash, bool noobsconfig, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _qpd(NULL), _qpssd(NULL), _qpdup(NULL), _kcpos(0), _defaultDisplay(defaultDisplay),
    _silent(false), _allowSilent(false), _showAll(false), _fixate(false), _splash(splash), _settings(NULL),
    _hasWifi(false), _numInstalledOS(0), _numBootableOS(0), _devlistcount(0), _netaccess(NULL), _displayModeBox(NULL), _drive(drive),
    _bootdrive(drive), _noobsconfig(noobsconfig), _numFilesToCheck(0), _eDownloadMode(MODE_INSTALL), _proc(NULL)
{
    TRACE

    timeset=false;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setContextMenuPolicy(Qt::NoContextMenu);
    update_window_title();
    _kc << 0x01000013 << 0x01000013 << 0x01000015 << 0x01000015 << 0x01000012
        << 0x01000014 << 0x01000012 << 0x01000014 << 0x42 << 0x41;
    _info=NULL;
    _infoDelay=0;
    _menuLabel = new QLabel();
    _menuLabel->setText(menutext(TOOLBAR_MAIN));
    ui->advToolBar->addWidget(_menuLabel);
    QPalette p = _menuLabel->palette();
    if (p.color(QPalette::WindowText) != Qt::darkBlue)
    {
        p.setColor(QPalette::WindowText, Qt::darkBlue);
        _menuLabel->setPalette(p);
    }
    QFont font;
    font.setItalic(true);
    font.setBold(true);
    _menuLabel->setFont(font);
    _availableImages = 0;
    _selectImages = 0;
    _waitforImages = 0;
    _processedImages = 0;
    _numListsToDownload=0;
    _skipformat=false;


    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->advToolBar->addWidget(spacer);

    _checkLabel = new QLabel();
    _checkLabel->setText("");
    ui->toolBar_2->addWidget(_checkLabel);

    toolbars.append(ui->toolBar_1);
    toolbars.append(ui->toolBar_2);
    toolbars.append(ui->toolBar_3);

    toolbar_index=TOOLBAR_MAIN;
    ui->toolBar_1->setVisible(toolbar_index==TOOLBAR_MAIN);
    ui->toolBar_2->setVisible(toolbar_index==TOOLBAR_ARCHIVAL);
    ui->toolBar_3->setVisible(toolbar_index==TOOLBAR_MAINTENANCE);
    ui->groupBox->setVisible(toolbar_index==TOOLBAR_MAIN);
    ui->groupBoxUsb->setVisible(toolbar_index==TOOLBAR_ARCHIVAL);

    //====================
    QShortcut *foo2 = new QShortcut(ui->toolBar_2);
    foo2->setKey(Qt::Key_Escape);
    connect(foo2, SIGNAL(activated()), this, SLOT(on_actionCancel_triggered()));

    QShortcut *foo3 = new QShortcut(ui->toolBar_3);
    foo3->setKey(Qt::Key_Escape);
    connect(foo3, SIGNAL(activated()), this, SLOT(on_actionCancel_triggered()));
    //====================

    QString reserve ="+0";
    QString provision="0";
    _provision=0;
    int forceruninstaller=0;

    QString cmdline = getFileContents("/proc/cmdline");

    _networkTimeout=8000;
    QStringList args = cmdline.split(QChar(' '),QString::SkipEmptyParts);
    foreach (QString s, args)
    {
        if (s.contains("remotetimeout"))
        {
            QStringList params = s.split(QChar('='));

            connect (&counter, SIGNAL(countdownTick(int)), this, SLOT(tick(int)));
            connect (&counter, SIGNAL(countdownExpired()), this, SLOT(expired()));
            //connect (&counter, SIGNAL(countdownStopped()), this, SLOT(countdownStopped()));
            installEventFilter(&counter);
            counter.startCountdown( params.at(1).toInt() +1);
        }
        if (s.contains("networktimeout"))
        {
            QStringList params = s.split(QChar('='));
            _networkTimeout = 1000 * params.at(1).toInt();
        }
        if (s.contains("reserve"))
        {
            QStringList params = s.split(QChar('='));
            reserve = params.at(1);
        }
        if (s.contains("provision"))
        {
            QStringList params = s.split(QChar('='));
            provision = params.at(1);
            _provision = provision.toUInt();
        }
        if (s.contains("forceruninstaller"))
        {
            forceruninstaller = 1;
        }
        if (s.contains("skipformat"))
        {
            _skipformat=true;
        }
    }

    ug = new OsGroup(this, ui, !cmdline.contains("no_group"));

    _currentsize = QSize(40,40);
    ug->list->setIconSize(_currentsize); //ALL?? set each list?
    ug->listInstalled->setIconSize(_currentsize); //ALL?? set each list?

    connect(ug->list, SIGNAL(currentRowChanged(int)), this, SLOT(on_list_currentRowChanged(void)));
    connect(ug->list, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(on_list_doubleClicked(const QModelIndex&)));
    connect(ug->list, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(on_list_itemChanged(QListWidgetItem *)));

    ug->list->setItemDelegate(new TwoIconsDelegate(this));
    ug->list->installEventFilter(this);

    ug->setFocus();

    ui->advToolBar->setVisible(true);

    _ipaddress=QHostAddress();

    QRect s = QApplication::desktop()->screenGeometry();
    int w = s.width()-10;
    int h = s.height() - 100;

    w =qMin(w,800);
    h =qMin(h,600);
    resize(w,h);

    _nav.setContext("mainwindow", "Main Menu");

    if (cec)
    {
        connect(cec, SIGNAL(keyPress(int,int)), this, SLOT(onKeyPress(int,int)));
    }
    if (joy1)
    {
        connect(joy1, SIGNAL(joyPress(int,int)), this, SLOT(onJoyPress(int,int)));
        connect(joy1, SIGNAL(joyEvent(int,int,int)), this, SLOT(onJoyEvent(int,int,int)));
        connect(joy1, SIGNAL(joyDebug(QString)), this, SLOT(onJoyDebug(QString)));
    }
    if (joy2)
    {
        connect(joy2, SIGNAL(joyPress(int,int)), this, SLOT(onJoyPress(int,int)));
        connect(joy2, SIGNAL(joyEvent(int,int,int)), this, SLOT(onJoyEvent(int,int,int)));
        connect(joy2, SIGNAL(joyDebug(QString)), this, SLOT(onJoyDebug(QString)));
    }


    if (qApp->arguments().contains("-runinstaller") && !_partInited)
    {
        bool doinstall=true;

        // Check for Settings partition and backup conf files
        QByteArray settingsPartition = partdev(_bootdrive, SETTINGS_PARTNR);
        QByteArray pinnPartition = partdev(_bootdrive, 1);
        if (QFile::exists(settingsPartition))
        {
            if (QProcess::execute("mount -t ext4 "+settingsPartition+" /settings") == 0)
            {
                if (QFile::exists("/settings/installed_os.json"))
                {

                    if ( !_silent && !forceruninstaller && (QMessageBox::question(this,
                        tr("RUNINSTALLER - Initialise Drive"),
                        tr("There are existing OSes installed.\nAre you sure you want to initialise the drive?"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No))
                    {
                        doinstall = false; /* Don't re-install */
                    }

                    QProcess::execute("mount -t vfat "+pinnPartition+" /mnt");
                    if (doinstall)
                    {
                        backupWpa();
                        backupDhcp();
                        sync();
                    }
                    else
                    {
                        QString cmdlinefilename = "/mnt/cmdline.txt";
                        if (!QFile::exists(cmdlinefilename))
                            cmdlinefilename = "/mnt/cmdline.txt";

                        /* Remove "runinstaller" from cmdline.txt */
                        QFile f(cmdlinefilename);
                        if (f.open(f.ReadOnly))
                        {
                            QByteArray line = f.readAll().trimmed();
                            line = line.replace("runinstaller", "").trimmed();
                            f.close();
                            f.open(f.WriteOnly);
                            f.write(line);
                            f.close();
                        }
                    }
                    QProcess::execute("umount /mnt");
                }
                QProcess::execute("umount /settings");
            }
        }

        if (doinstall)
        {
            /* Repartition SD card first */
            _partInited = true;
            setEnabled(false);
            _qpd = new QProgressDialog( tr("Setting up SD card"), QString(), 0, 0, this);
            _qpd->setWindowModality(Qt::WindowModal);
            _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

            InitDriveThread *idt = new InitDriveThread(_bootdrive, this, reserve, _provision);
            connect(idt, SIGNAL(statusUpdate(QString)), _qpd, SLOT(setLabelText(QString)));
            connect(idt, SIGNAL(completed()), _qpd, SLOT(deleteLater()));
            connect(idt, SIGNAL(error(QString)), this, SLOT(onQpdError(QString)));
            connect(idt, SIGNAL(query(QString, QString, QMessageBox::StandardButton*)),
                    this, SLOT(onQuery(QString, QString, QMessageBox::StandardButton*)),
                    Qt::BlockingQueuedConnection);

            idt->start();
            _qpd->exec();
            setEnabled(true);
        }
    }

    /* Make sure the SD card is ready, and partition table is read by Linux */
    QByteArray settingsPartition = partdev(_bootdrive, SETTINGS_PARTNR);
    if (!QFile::exists(settingsPartition))
    {
        _qpd = new QProgressDialog( tr("Waiting for SD card (settings partition)"), QString(), 0, 0, this);
        _qpd->setWindowModality(Qt::WindowModal);
        _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        _qpd->show();

        while (!QFile::exists(settingsPartition))
        {
            QApplication::processEvents(QEventLoop::WaitForMoreEvents, 250);
        }
        _qpd->hide();
        _qpd->deleteLater();
    }

    _qpd = new QProgressDialog( tr("Mounting settings partition"), QString(), 0, 0, this);
    _qpd->setWindowModality(Qt::WindowModal);
    _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    _qpd->show();
    QApplication::processEvents();

    ug->list->clear();
    if (QProcess::execute("mount -t ext4 "+settingsPartition+" /settings") != 0)
    {
        _qpd->hide();

        if (QMessageBox::question(this,
                                  tr("Error mounting settings partition"),
                                  tr("Persistent settings partition seems corrupt. Reformat?"),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
        {
            QProcess::execute("umount /settings");
            if (QProcess::execute("/usr/sbin/mkfs.ext4 "+settingsPartition) != 0
                || QProcess::execute("mount "+settingsPartition+" /settings") != 0)
            {
                QMessageBox::critical(this, tr("Reformat failed"), tr("SD card might be damaged"), QMessageBox::Close);
            }

            rebuildInstalledList();
        }
    }
    _qpd->hide();
    _qpd->deleteLater();
    _qpd = NULL;

    QProcess::execute("mkdir -p /settings/dropbear");
    if (!QFile::exists("/settings/.ssh"))
    {
        QProcess::execute("mkdir -p /settings/.ssh");
        QProcess::execute("chmod 700 /settings/.ssh");
    }


    QProcess::execute("mount -o ro -t vfat "+partdev(_bootdrive, 1)+" /mnt");

    _model = getFileContents("/proc/device-tree/model");

    setModelInfo();

    //if (_model.contains("Pi 5"))
    {
        extern QApplication * gApp;

        fontsize=12;
        updateFont(fontsize);
    }

    if (!cmdline.contains("no_overrides"))
        loadOverrides("/mnt/overrides.json");

    if (QFile::exists("/mnt/os_list_v3.json"))
    {
        /* We have a local os_list_v3.json for testing purposes */
        _repo = "/mnt/os_list_v3.json";
        qDebug() << "Using local /mnt/os_list_v3.json";

        /* We need a somewhat accurate date for https to work. Normally we retrieve that from the repo server,
           but since we are in testing mode, just set date to last modification time of our local file */
        if (QDate::currentDate().year() < 2016)
        {
            QFileInfo fi(_repo);

            struct timeval tv;
            tv.tv_sec = fi.lastModified().toTime_t();
            tv.tv_usec = 0;
            settimeofday(&tv, NULL);
        }
    }
    else if (cmdline.contains("repo="))
    {
        QByteArray searchFor = "repo=";
        int searchForLen = searchFor.length();
        int pos = cmdline.indexOf(searchFor);
        int end;

        if (cmdline.length() > pos+searchForLen && cmdline.at(pos+searchForLen) == '"')
        {
            /* Value between quotes */
            searchForLen++;
            end = cmdline.indexOf('"', pos+searchForLen);
        }
        else
        {
            end = cmdline.indexOf(' ', pos+searchForLen);
        }
        if (end != -1)
            end = end-pos-searchForLen;
        _repo = cmdline.mid(pos+searchForLen, end);
    }
    else if (!cmdline.contains ("no_default_source"))
    {	//Only add if not excluded
        _repo = DEFAULT_REPO_SERVER;
    }

    if (QFile::exists("/mnt/repo_list.json"))
    {
        /* We have a local repo_list.json for testing purposes */
        qDebug() << "Using local repo_list.json";
        repoList = "/mnt/repo_list.json";
    }

    _usbimages = !cmdline.contains("disableusbimages");
    if (!_usbimages)
        _processedImages |= ALLUSB;
    _sdimages  = !cmdline.contains("disablesdimages");
    if (!_sdimages)
        _processedImages |= ALLSD;
    _showAll   = cmdline.contains("showall");
    _fixate    = cmdline.contains("fixate");

    if (cmdline.contains("select="))
    {
        QByteArray searchFor = "select=";
        int searchForLen = searchFor.length();
        int pos = cmdline.indexOf(searchFor);
        int end;

        if (cmdline.length() > pos+searchForLen && cmdline.at(pos+searchForLen) == '"')
        {
            /* Value between quotes */
            searchForLen++;
            end = cmdline.indexOf('"', pos+searchForLen);
        }
        else
        {
            end = cmdline.indexOf(' ', pos+searchForLen);
        }
        if (end != -1)
            end = end-pos-searchForLen;
        QString selection  = cmdline.mid(pos+searchForLen, end);

        qDebug()<<"Processing Selection: " <<selection;

        QStringList args = selection.split(",", QString::SkipEmptyParts);
        foreach (QString arg, args)
        {
            if (arg=="allsd")
            {
                _selectImages |= ALLSD;
                _waitforImages  |= ALLSD;
            }
            else if (arg=="waitsd")
            {
                _waitforImages |=ALLSD;
            }

            else if (arg=="allusb")
            {
                _selectImages |= ALLUSB;
                _waitforImages  |= ALLUSB;
            }
            else if (arg=="waitusb")
            {
                _waitforImages |= ALLUSB;
            }

            else if (arg=="allnetwork")
            {
                _selectImages |= ALLNETWORK;
                _waitforImages  |= ALLNETWORK;
            }
            else if (arg=="waitnetwork")
            {
                _waitforImages |= ALLNETWORK;
            }
            else if (arg=="waitall")
            {
                _waitforImages |= ALLNETWORK | ALLUSB | ALLSD;
            }
            else if (arg=="allinstalled")
            {
                _waitforImages |= ALLINSTALLED;
            }
            else
            {
                _selectOsList << arg;
            }
        }
    }

    copyWpa();
    copyDhcp();

    if (cmdline.contains("silentinstall"))
    {
        /* If silentinstall is specified, auto-install single image in /os */
        _allowSilent = true;
    }

    //ALWAYS start networking (for silentinstall of remote images)
    startNetworking();

    //Background.sh was here

    /* Disable online help buttons until network is functional */
    ui->actionBrowser->setEnabled(false);
    QTimer::singleShot(1, this, SLOT(populate()));

    ui->targetLabel->setHidden(true);
    ui->targetCombo->setHidden(true);
    connect(&_piDrivePollTimer, SIGNAL(timeout()), SLOT(pollForNewDisks()));
    _piDrivePollTimer.start(POLLTIME);
    ug->setFocus();

}

void MainWindow::setModelInfo()
{
    ui->modelname->setText(_model);

    //Revision code: NOQu uuWu FMMM CCCC PPPP TTTT TTTT RRRR
    uint rev = readBoardRevision();
    ui->memory->setText("");
    uint mem = rev>>20 & 0x07;
    switch (mem)
    {
        case 0:
            ui->memory->setText("256MB");
            break;
        case 1:
            ui->memory->setText("512MB");
            break;
        case 2:
            ui->memory->setText("1GB");
            break;
        case 3:
            ui->memory->setText("2GB");
            break;
        case 4:
            ui->memory->setText("4GB");
            break;
        case 5:
            ui->memory->setText("8GB");
            break;
        default:
            ui->memory->setText("");
            break;
    }

}

MainWindow::~MainWindow()
{
    if (cec)
        disconnect(cec, SIGNAL(keyPress(int,int)), this, SLOT(onKeyPress(int,int)));
    if (joy1)
        disconnect(joy1, SIGNAL(joyPress(int,int)), this, SLOT(onJoyPress(int,int)));
    if (joy2)
        disconnect(joy2, SIGNAL(joyPress(int,int)), this, SLOT(onJoyPress(int,int)));

    QProcess::execute("umount /mnt");
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (cec)
        disconnect(cec, SIGNAL(keyPress(int,int)), this, SLOT(onKeyPress(int,int)));
    if (joy1)
        disconnect(joy1, SIGNAL(joyPress(int,int)), this, SLOT(onJoyPress(int,int)));
    if (joy2)
        disconnect(joy2, SIGNAL(joyPress(int,int)), this, SLOT(onJoyPress(int,int)));
    event->accept();
}

QString MainWindow::menutext(int index)
{
    static const char* menutext_strings[]= {
        QT_TR_NOOP("Main Menu"),
        QT_TR_NOOP("Archival"),
        QT_TR_NOOP("Maintenance")
    };
    index %= NUM_TOOLBARS; //Keep it in range
    return tr(menutext_strings[index]);
}


/* Discover which images we have, and fill in the list */
void MainWindow::populate()
{

    /* Ask user to wait while list is populated */
    if (!_allowSilent)
    {
        int timeout = _networkTimeout;
        if (timeout>0)
        {
            _qpd = new QProgressDialog(tr("Please wait while PINN initialises"), QString(), 0, 0, this);
            _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            _qpd->show();

            _qpd->installEventFilter(&counter);

            if (getFileContents("/settings/wpa_supplicant.conf").contains("ssid="))
            {
                /* Longer timeout if we have a wifi network configured */
                timeout += 4000;
            }
            QTimer::singleShot(timeout, this, SLOT(hideDialogIfNoNetwork()));
            _time.start();
        }
    }

    _settings = new QSettings("/settings/noobs.conf", QSettings::IniFormat, this);

    /* Restore saved display mode */
    qDebug() << "Default display mode is " << _defaultDisplay;
    int mode = _settings->value("display_mode", _defaultDisplay).toInt();
    if (mode)
    {
        displayMode(mode, true);
    }
    _settings->setValue("display_mode", _defaultDisplay);
    _settings->sync();

    // Fill in list of images
    addInstalledImages();
    repopulate();
    recalcAvailableMB();
    updateNeeded();

    ug->setDefaultItems();

    if (_allowSilent && !_selectImages && !_numInstalledOS && ug->count() == 1)
    {
        // No OS installed, perform silent installation
        qDebug() << "Performing silent installation";
        _silent = true;
        ug->list->item(0)->setCheckState(Qt::Checked);
        _eDownloadMode = MODE_INSTALL; //Just to be sure!
        on_actionWrite_image_to_disk_triggered();
        _numInstalledOS = 1;
        _numBootableOS = 1; //@@Assume user has installed a bootable OS.
    }
    ui->actionCancel->setEnabled(_numBootableOS > 0);
}

void MainWindow::repopulate()
{

    QMap<QString,QVariantMap> images;

    QIcon localIcon(":/icons/hdd.png");

    if ( _sdimages )
    {
        images= listImages();
        _availableImages |= ALLSD;
    }

    foreach (QVariant v, images.values())
    {
        QVariantMap m = v.toMap();
        bool bInstalled=false;
        addImage(m,localIcon,bInstalled);
    }

    //@@Add dummy icons?

    if (_numBootableOS)
    {
        ui->actionCancel->setEnabled(true);
        if (_fixate)
        {
            ug->list->setEnabled(false);
        }
    }
    updateInstalledStatus();
    filterList();
    ug->showTab(DEFGROUP);
    ug->setDefaultItems();
    _processedImages |= ALLSD;
}

/* Whether this OS should be displayed in the list of installable OSes */
bool MainWindow::canInstallOs(const QString &name, const QVariantMap &values)
{

    /* Can't simply pull "name" from "values" because in some JSON files it's "os_name" and in others it's "name" */

    /* If it's not bootable, it isn't really an OS, so is always installable */
    if (!canBootOs(name, values))
    {
        return true;
    }

    /* RISC_OS needs a matching riscos_offset */
    if (nameMatchesRiscOS(name))
    {
        if (!values.contains(RISCOS_OFFSET_KEY) || (values.value(RISCOS_OFFSET_KEY).toInt() != RISCOS_OFFSET))
        {
            return false;
        }
    }

    /* Display OS in list if it is supported or "showall" is specified in cmdline.txt */
    if (_showAll)
    {
        return true;
    }
    else
    {
        return isSupportedOs(name, values);
    }
}

/* Whether this OS is supported */
bool MainWindow::isSupportedOs(const QString &name, const QVariantMap &values)
{

    /* Can't simply pull "name" from "values" because in some JSON files it's "os_name" and in others it's "name" */
    /* If it's not bootable, it isn't really an OS, so is always supported */
    if (!canBootOs(name, values))
    {
        return true;
    }

    if (values.contains("supported_models"))
    {
        QStringList supportedModels = values.value("supported_models").toStringList();

        foreach (QString m, supportedModels)
        {
            /* Check if the full formal model name (e.g. "Raspberry Pi 2 Model B Rev 1.1")
             * contains the string we are told to look for (e.g. "Pi 2") */
            if (_model.contains(m, Qt::CaseInsensitive))
            {
                return true;
            }
        }
        return false;
    }

    return true;
}

QMap<QString, QVariantMap> MainWindow::listImages(const QString &folder)
{
    TRACE
    QMap<QString,QVariantMap> images;
    /* Local image folders */
    QDir dir(folder, "", QDir::Name, QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList list = dir.entryList();

    foreach (QString image,list)
    {
        DBG(image);
        QString imagefolder = folder+"/"+image;
        if (!QFile::exists(imagefolder+"/os.json"))
            continue;
        QVariantMap osv = Json::loadFromFile(imagefolder+"/os.json").toMap();

        OverrideJson(osv); //Allow overriding of supported_models by putting this here.

        if (folder == "/mnt/os")
            osv["source"] = SOURCE_SDCARD;
        else
            osv["source"] = SOURCE_USB;

        QString basename = osv.value("name").toString();
        if (canInstallOs(basename, osv))
        {
            if (QFile::exists(imagefolder+"/flavours.json"))
            {
                QVariantMap v = Json::loadFromFile(imagefolder+"/flavours.json").toMap();
                QVariantList fl = v.value("flavours").toList();

                foreach (QVariant f, fl)
                {
                    QVariantMap fm  = f.toMap();
                    if (fm.contains("name"))
                    {
                        QVariantMap item = osv;
                        QString name        = fm.value("name").toString();
                        if (CORE(name) == RECOMMENDED_IMAGE)
                            item["recommended"] = true;
                        item["name"]        = name;
                        item["description"] = fm.value("description").toString();

                        if (fm.contains("icon"))
                            item["icon"]=fm.value("icon").toString();
                        item["folder"] = imagefolder;
                        images[name] = item;
                    }
                }
            }
            else
            {
                QString name = basename;
                if (CORE(name) == RECOMMENDED_IMAGE)
                    osv["recommended"] = true;
                osv["folder"] = imagefolder;
                images[name] = osv;
            }
        }
        DBG(osv);
    }
    DBG("-------");
    for (QMap<QString,QVariantMap>::iterator i = images.begin(); i != images.end(); i++)
    {
        DBG(i.value().value("name").toString());
        if (!i.value().contains("nominal_size"))
        {
            /* Calculate nominal_size based on information inside partitions.json */
            int nominal_size = 0;
            QVariantMap pv = Json::loadFromFile(i.value().value("folder").toString()+"/partitions.json").toMap();
            QVariantList pvl = pv.value("partitions").toList();

            foreach (QVariant v, pvl)
            {
                QVariantMap pv = v.toMap();
                nominal_size += pv.value("partition_size_nominal").toInt();
                nominal_size += 1; /* Overhead per partition for EBR */
            }

            i.value().insert("nominal_size", nominal_size);
        }
    }

    return images;
}

void MainWindow::updateInstalledStatus()
{

    _numBootableOS = ug->updateInstalledStatus();
    qDebug() << "Number of bootables = "<<_numBootableOS;
    //@@ Maybe add: _numInstalledOS = ug->listInstalled->count();
    //@@if (ug->listInstalled->count()>1)
    if (_numBootableOS)
    {
        ui->actionCancel->setEnabled(true);
        if (_fixate)
        {
            ug->list->setEnabled(false);
        }
    }

    /* Giving items without icon a dummy icon to make them have equal height and text alignment */
    QPixmap dummyicon = QPixmap(_currentsize.width(), _currentsize.height());
    dummyicon.fill();

    QList<QListWidgetItem *> all;
    all = ug->allItems();

    for (int i=0; i< ug->count(); i++)
    {
        if (all.value(i)->icon().isNull())
        {
            all.value(i)->setIcon(dummyicon);
        }
    }

    for (int i=0; i< ug->listInstalled->count(); i++)
    {
        if (ug->listInstalled->item(i)->icon().isNull())
        {
            ug->listInstalled->item(i)->setIcon(dummyicon);
        }
    }

}


/* Iterates over the installed images and adds each one to the ug->listinstalled and ug->list lists */
void MainWindow::addInstalledImages()
{

    if (ug->listInstalled->count())
        ug->listInstalled->clear();

    createPinnEntry();

    if (_settings)
    {
        _numInstalledOS=0;
        _numBootableOS=0;
        QVariantList i = Json::loadFromFile("/settings/installed_os.json").toList();
        foreach (QVariant v, i)
        {
            _numInstalledOS++;
            QVariantMap m = v.toMap();
            QString name = m.value("name").toString();
            if (CORE(name) == RECOMMENDED_IMAGE)
                m["recommended"] = true;
            if (m.value("bootable").toBool() == true)
                _numBootableOS++;
            m["installed"]=true;
            m["source"] = SOURCE_INSTALLED_OS;

            if (m.contains("supports_backup"))
            {
                if (m.value("supports_backup","false").toString()=="update")
                    m["supports_backup"]="update";
                else if (m.value("supports_backup").toBool()==true)
                    m["supports_backup"]=true;
                else
                    m["supports_backup"]=false;
            }

            bool bInstalled=true;
            QIcon localIcon(":/icons/hdd_usb_unmount.png");
            addImage(m, localIcon, bInstalled);
        }
    }
}


void MainWindow::on_actionWrite_image_to_disk_triggered()
{

    _eDownloadMode = MODE_INSTALL;

    bool allSupported = true;
    bool gotAllSource = true;
    bool bPartuuids=true;
    QString unsupportedOses;
    QString missingOses;
    QString selectedOSes;
    QString nonpartuuids;

    QList<QListWidgetItem *> selected = selectedItems();

    //We must have at least one OS selected.
    if (selected.count()<1)
    {
        qDebug() << "No OSes selected to install";
        return;
    }

    if (_numBootableOS)
    {
        dlgInstall dlg;
        if (!_silent)
        {
            dlg.setWindowModality(Qt::WindowModal);
            if (QDialog::Rejected == dlg.exec())
                return;
        }
    }

    _newList.clear();
    /* Get list of all selected OSes and see if any are unsupported */
    foreach (QListWidgetItem *item, selected)
    {
        QVariantMap entry = item->data(Qt::UserRole).toMap();
        QString name = entry.value("name").toString();
        selectedOSes += "\n" + name;

        _newList.append(entry);
        if (!isSupportedOs(name, entry))
        {
            allSupported = false;
            unsupportedOses += "\n" + name;
        }
        if (entry.value("source")==SOURCE_INSTALLED_OS)
        {
            gotAllSource = false;
            missingOses += "\n" + name;
        }
        if ((entry.value("use_partuuid")==false) && ((_bootdrive!="/dev/mmcblk0") || (_drive!="/dev/mmcblk0")))
        {
            nonpartuuids += "\n" + name;
            bPartuuids = false;
        }
    }

    if (!gotAllSource)
    {
        if (!_silent)
            QMessageBox::warning(this,
                                 tr("ERROR"),
                                 tr("Error: Some OSes are not available:\n")+missingOses,
                                 QMessageBox::Close);
        return;
    }

    if (bPartuuids == false)
    {
        if ( !_silent && QMessageBox::warning(this,
                                    tr("Confirm"),
                                    tr("Warning: Partial USB support. The following OSes can only be executed from USB when it is /dev/sda and may fail to boot or function correctly if that is not the case:\n") + nonpartuuids + tr("\n\nDo you want to continue?"),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
            return;
    }

    if (_newList.count())
    {
        if (_silent || allSupported || QMessageBox::warning(this,
                                    tr("Confirm"),
                                    tr("Warning: incompatible Operating System(s) detected. The following OSes aren't supported on this revision of Raspberry Pi and may fail to boot or function correctly:") + unsupportedOses,
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
        {
            prepareMetaFiles();
        }
    }
}

void MainWindow::on_actionReinstall_triggered()
{
    _eDownloadMode = MODE_REINSTALL;
    doReinstall();
}


void MainWindow::doReinstall()
{
    _newList.clear();
    QList<QListWidgetItem *> installedList;

    installedList   = ug->selectedInstalledItems();

    //Check if user wants to manually update PINN
    QListWidgetItem *item = installedList.at(0);
    QVariantMap installedMap = item->data(Qt::UserRole).toMap();
    if (installedMap.value("name").toString() =="PINN")
    {
        if (installedList.count()==1)
        {
            if (QFile::exists("/tmp/pinn-lite.zip"))
            {
                _qpdup = new QProgressDialog( QString(tr("Downloading Update")), QString(), 0, 0, this);
                _qpdup->setWindowModality(Qt::WindowModal);
                _qpdup->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                _qpdup->setWindowTitle("Updating PINN");
                _qpdup->show();

                int error = updatePinn();

                if (_qpdup)
                {
                    _qpdup->hide();
                    _qpdup->deleteLater();
                    _qpdup = NULL;
                }

                if (!error)
                {
                    //Reboot back into PINN
                    QByteArray partition("1");
                    setRebootPartition(partition);

                    // Reboot
                    reboot();
                }
            }

            //Only check upgrades to PINN if it is the ONLY Os to be reinstalled
            //Because it causes a reboot
            if (requireNetwork())
                checkForUpdates( EUPDATEMANUAL );
            return;
        }
        //Otherwise ignore PINN if there are more selected
        installedList.removeFirst();
    }

    foreach (QListWidgetItem *item, installedList)
    {
        QVariantMap installedEntry = item->data(Qt::UserRole).toMap();
        QString name = CORE(installedEntry.value("name").toString());

        //Look for the new version
        QListWidgetItem *witem = findItemByName(name);
        if (witem)
        {
            QVariantMap new_details = witem->data(Qt::UserRole).toMap();
            if (new_details.value("source").toString() == SOURCE_INSTALLED_OS)
            {
                onError(name + tr(" is not available.\nPlease provide it locally or connect to the internet."));
                return;
            }
            new_details["existingOS"] = installedEntry;
            _newList.append(new_details);
        }
    }
    if (_newList.count())
    {
        prepareMetaFiles();
    }
    else
    {
        if (!_silent)
            QMessageBox::warning(this,
                                 tr("ReInstall OSes"),
                                 tr("Warning: No OSes selected or available\n"),
                                 QMessageBox::Close);

    }
}

void MainWindow::prepareMetaFiles()
{

    QString warning;

    _numMetaFilesToDownload=0;

    QString mode;
    switch(_eDownloadMode)
    {
    case MODE_INSTALL:
        mode =tr("install");
        break;
    case MODE_REINSTALL:
        mode =tr("reinstall");
        break;
    case MODE_REPLACE:
        mode =tr("replace");
        break;
    default:
        mode="?";
        break;
    }

    if (_eDownloadMode == MODE_INSTALL)
    {
        QString driveType;
        driveType = (_drive == "/dev/mmcblk0") ? tr("SD card") : tr("USB drive");
        warning = tr("Warning: this will %1 the selected Operating System(s) to %2. All existing data on the %3 will be deleted.").arg(mode,_drive,driveType);
    }
    else
    {
        warning = tr("Warning: this will %1 the selected Operating System(s) on %2. The partitions of the selected OSes will first be erased.").arg(mode,_drive);
    }

    if ( _silent || QMessageBox::warning(this,
                            tr("Confirm"),
                            warning,
                            QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        setEnabled(false);

        if ( MODE_INSTALL == _eDownloadMode)
        {
            //Remove any old OS meta files from previous installs.
            QProcess::execute("rm -rf /settings/os");
        }

        if (_networkStatusPollTimer.isActive())
            _networkStatusPollTimer.stop();

        foreach (QVariantMap newOS, _newList)
        {
            QDir d;

            QString folder = "/settings/os/"+ CORE(newOS.value("name").toString());
            folder.replace(' ', '_');
            if (!d.exists(folder))
                d.mkpath(folder);

            if ((!newOS.contains("folder")) || (newOS.value("folder").toString().startsWith("/settings")))
            {

                downloadMetaFile(newOS.value("os_info").toString(), folder+"/os.json");
                downloadMetaFile(newOS.value("partitions_info").toString(), folder+"/partitions.json");

                QString urlpath = newOS.value("os_info").toString().left(newOS.value("os_info").toString().lastIndexOf('/'));
                downloadMetaFile(urlpath+"/release_notes.txt", "-"+folder+"/release_notes.txt"); //'-' indicates optional
                downloadMetaFile(urlpath+"/terms", "-"+folder+"/terms"); //'-' indicates optional


                if (newOS.contains("marketing_info"))
                    downloadMetaFile(newOS.value("marketing_info").toString(), folder+"/marketing.tar");

                if (newOS.contains("partition_setup"))
                    downloadMetaFile(newOS.value("partition_setup").toString(), folder+"/partition_setup.sh");

                if (newOS.contains("icon"))
                    downloadMetaFile(newOS.value("icon").toString(), folder+"/icon.png");
            }
            else
            { //Copy files from local storage to /settings folder
                QString local = newOS.value("folder").toString();

                QString cmd;
                cmd = "cp " + local+"/os.json "+folder;
                QProcess::execute(cmd);
                cmd = "cp "+ local+"/partitions.json "+folder;
                QProcess::execute(cmd);
                cmd = "cp "+ local+"/release_notes.txt "+folder;
                QProcess::execute(cmd);
                cmd = "cp "+ local+"/terms "+folder;
                QProcess::execute(cmd);
                cmd = "cp "+ local+"/partition_setup.sh "+folder;
                QProcess::execute(cmd);
                if (QFile::exists(local+"/marketing.tar" ))
                {
                    cmd = "cp "+ local+"/marketing.tar "+folder;
                    QProcess::execute(cmd);

                    QString marketingTar = folder+"/marketing.tar";
                    /* Extract tarball with slides */
                    QProcess::execute("tar xf "+marketingTar+" -C "+folder);
                    QFile::remove(marketingTar);
                }
                cmd = "cp -r "+ local+"/slides_vga/ "+folder;
                QProcess::execute(cmd);

                //Icon gets copied at end of processing if installed from network or USB.
            }

        }

        if (_numMetaFilesToDownload == 0)
        {
            /* All OSes selected are local */
            if (_eDownloadMode == MODE_INSTALL)
                startImageWrite();
            else
                startImageReinstall();
        }
        else if (!_silent)
        {
            _qpd = new QProgressDialog(tr("The %1 process will begin shortly.").arg(mode), QString(), 0, 0, this);
            _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            _qpd->show();
        }
    }
}

void MainWindow::on_actionDownload_triggered()
{

    _eDownloadMode = MODE_DOWNLOAD;

    //@@ maybe here decide if to download to /mnt or /settings and only mount that one rw

    _local = "/tmp/media/"+partdev(_osdrive,1);

    //if (QProcess::execute("mount -o remount,rw /dev/"+partdev(_osdrive,1)+" "+_local) != 0)
    // The NTFS driver can't remount, so we'll just umount & mount again
    QProcess::execute("umount /dev/"+partdev(_osdrive,1));
    QProcess::execute("mount  /dev/"+partdev(_osdrive,1)+" "+_local);

    if (_silent || QMessageBox::warning(this,
                                        tr("Confirm"),
                                        tr("Warning: this will download the selected Operating System(s)."),
                                        QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        /* See if any of the OSes are unsupported */
        bool allSupported = true;
        QString unsupportedOses;
        QList<QListWidgetItem *> selected = selectedItems();
        //@@ Check for unsupported (undownloadable) OSes
        if (_silent || allSupported || QMessageBox::warning(this,
                                        tr("Confirm"),
                                        tr("Warning: incompatible Operating System(s) detected. The following OSes aren't supported on this revision of Raspberry Pi and may fail to boot or function correctly:") + unsupportedOses,
                                        QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
        {
            setEnabled(false);
            _numMetaFilesToDownload = 0;

            QList<QListWidgetItem *> selected = selectedItems();
            foreach (QListWidgetItem *item, selected)
            {
                QVariantMap entry = item->data(Qt::UserRole).toMap();
                //if (!entry.contains("folder")) //If no folder, it must be a downloadable OS
                if (entry.value("source").toString() == SOURCE_NETWORK) // only network OS are downloadable.
                {
                    QDir d;
                    QString osname = entry.value("name").toString();

                    QFileInfo fi = entry.value("os_info").toString();   //full URL to os.json

                    QString folder = _local+"/os/"+osname;
                    folder.replace(' ', '_');
                    if (!d.exists(folder))
                        d.mkpath(folder);

                    QString path = folder + QString("/error.log");
                    QFile f(path);
                    if (f.exists())
                    {
                        f.remove();
                    }

                    //Is this the same as fi.path()?
                    QString urlpath = entry.value("os_info").toString().left(entry.value("os_info").toString().lastIndexOf('/'));

                    //Required fields
                    downloadMetaFile(entry.value("os_info").toString(), folder+"/os.json");
                    downloadMetaFile(entry.value("partitions_info").toString(), folder+"/partitions.json");

                    //Try and download flavours, but not an error if they don't exist
                    downloadMetaFile( fi.path() +"/flavours.json",  "-"+folder+"/flavours.json");
                    downloadMetaFile( fi.path() +"/flavours.tar.xz", "-"+folder+"/flavours.tar.xz");

                    downloadMetaFile(urlpath+"/release_notes.txt", "-" + folder+"/release_notes.txt"); //'-' indicates optional
                    downloadMetaFile(urlpath+"/terms", "-" + folder+"/terms"); //'-' indicates optional
                    if (entry.contains("marketing_info"))
                    {
                        downloadMetaFile(entry.value("marketing_info").toString(), folder+"/marketing.tar");
                    }
                    else {
                        downloadMetaFile(urlpath+"/marketing.tar", "-" + folder+"/marketing.tar"); //'-' indicates optional
                    }

                    if (entry.contains("partition_setup"))
                    {
                        downloadMetaFile(entry.value("partition_setup").toString(), folder+"/partition_setup.sh");
                    }
                    else {
                        downloadMetaFile(urlpath+"/partition_setup.sh", "-" + folder+"/partition_setup.sh"); //'-' indicates optional
                    }

                    if (entry.contains("icon"))
                    {
                        //Extract icon filename from URL
                        QString icon_name   = osname + ".png";
                        icon_name.replace(' ','_');
                        downloadMetaFile(entry.value("icon").toString(), folder+"/"+icon_name);
                    }
                    //@@Create JSON files for the os/flavours
                }
            }

            if (_numMetaFilesToDownload == 0)
            {
                /* All OSes selected are local */
                startImageDownload();
            }
            else if (!_silent)
            {
                _qpd = new QProgressDialog(tr("The download process will begin shortly."), QString(), 0, 0, this);
                _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                _qpd->show();
            }
        }
    }
}


void MainWindow::on_actionCancel_triggered()
{
    close();
    QApplication::quit();
}

void MainWindow::onCompleted(int arg)
{
    TRACE
    int ret;
    Q_UNUSED(ret);
    ret = QMessageBox::Ok;

    _qpssd->hide();
    _piDrivePollTimer.start(POLLTIME);
    QSettings settings("/settings/noobs.conf", QSettings::IniFormat, this);
    if (_eDownloadMode == MODE_INSTALL)
    {
        settings.setValue("default_partition_to_boot", "800");
        settings.remove("sticky_boot");
        settings.sync();
    }

    if (!_silent)
    {
        if (_eDownloadMode==MODE_DOWNLOAD)
        {
            /* make the USB stick read only again */
            QProcess::execute("sync");
            QProcess::execute("umount /dev/"+partdev(_osdrive,1));
            QProcess::execute("mount -o ro /dev/"+partdev(_osdrive,1)+" "+_local);

            QString info;
            if (arg)
                info = tr("OS(es) Downloaded with errors.\nSee debug log for details.");
            else
                info = tr("OS(es) Downloaded Successfully.");

            ret = QMessageBox::information(this,
                                     tr("OS(es) downloaded"),
                                     info, QMessageBox::Ok);
        }
        else if (_eDownloadMode==MODE_BACKUP)
        {
            /* make the USB stick read only again */
            QProcess::execute("sync");
            QProcess::execute("umount /dev/"+partdev(_osdrive,1));
            QString cmd = "mount -o ro /dev/"+partdev(_osdrive,1)+" "+_local;
            DBG(cmd);
            QProcess::execute(cmd);

            QString info;
            if (arg)
                info = tr("OS(es) Backed up with errors.\nSee debug log for details.");
            else
                info = tr("OS(es) Backed up Successfully.");
            ret = QMessageBox::information(this,
                                     tr("Backup OSes"),
                                     info, QMessageBox::Ok);
        }
        else // install,Reinstall or replace
        {
            QString info;
            if (arg)
                info = tr("OS(es) Installed with errors.\nSee debug log for details.");
            else
                info = tr("OS(es) Installed Successfully");
            ret = QMessageBox::information(this,
                                     tr("Installation complete"),
                                     info, QMessageBox::Ok);
        }
    }
    _qpssd->deleteLater();
    _qpssd = NULL;

    // Return back to main menu
    closeDialogs();
    setEnabled(true);
    show();
    _silent=false;

    if ((_eDownloadMode == MODE_INSTALL) || (_eDownloadMode == MODE_REPLACE) || (_eDownloadMode == MODE_REINSTALLNEWER))
    {
        // Update list of installed OSes.
        ug->listInstalled->clear();
        addInstalledImages();
        updateInstalledStatus();

        //Only close if there are bootable OSes
        if ((_eDownloadMode == MODE_INSTALL)  || (_eDownloadMode == MODE_REINSTALLNEWER))
        {   //Only reboot for install
            if (_numBootableOS)
            {
                close();
                QApplication::quit();
            }
        }
    }
}

void MainWindow::onErrorContinue(const QString &msg)
{

    qDebug() << "Error:" << msg;
    if (!_silent)
        QMessageBox::critical(this, tr("Error"), msg, QMessageBox::Close);
}

int MainWindow::closeDialogs()
{
    int numDialogs=0;
    QWidgetList topWidgets = QApplication::topLevelWidgets();
    foreach (QWidget *w, topWidgets)
    {
        if (qobject_cast<QMessageBox *>(w) || qobject_cast<WifiSettingsDialog*>(w))
        {
            if (w->isVisible())
            {
                numDialogs++;
                w->close();
                QApplication::processEvents();
            }
        }
    }
    return (numDialogs);
}

void MainWindow::onError(const QString &msg)
{

    qDebug() << "Error:" << msg;
    if (_qpssd)
        _qpssd->hide();

    setEnabled(true);

    this->lower();

    if (!_silent)
        QMessageBox::critical(this, tr("Error"), msg, QMessageBox::Close);

    closeDialogs();

    _piDrivePollTimer.start(POLLTIME);
    show();
}

void MainWindow::onQpdError(const QString &msg)
{

    qDebug() << "Error:" << msg;
    if (_qpd)
    {
        _qpd->hide();
    }
    QMessageBox::critical(this, tr("Error"), msg, QMessageBox::Close);
    setEnabled(true);
    _piDrivePollTimer.start(POLLTIME);
    show();
}

void MainWindow::onErrorNoMsg()
{
    qDebug() << "Rejected";
}

void MainWindow::onQuery(const QString &msg, const QString &title, QMessageBox::StandardButton* answer)
{
    *answer = QMessageBox::question(this, title, msg, QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
}

void MainWindow::onChecksumError(const QString &msg, const QString &title, QMessageBox::ButtonRole* answer)
{


    if (!_silent)
    {
        QMessageBox msgBox;
        //this->blockSignals(true);
        msgBox.setParent(NULL);
        msgBox.setWindowTitle(title);
        msgBox.setText(msg);

        msgBox.addButton(tr("Abort"), QMessageBox::DestructiveRole);
        msgBox.addButton(tr("Discard"), QMessageBox::RejectRole);
        msgBox.addButton(tr("Retry"), QMessageBox::NoRole);
        msgBox.addButton(tr("Keep"), QMessageBox::AcceptRole);
        msgBox.exec();

        *answer = msgBox.buttonRole(msgBox.clickedButton());
    }
    else
    {   //If silent, we'll accept any checksum errors, but we'll make it non-bootable.
        *answer = QMessageBox::AcceptRole;
    }
}

void MainWindow::on_list_currentRowChanged()
{

    updateActions();
}

void MainWindow::update_window_title()
{

    QString count;
    int currentCount = counter.getCountdown();
    if (currentCount)
    {
        count = QString(tr("Reboot in %1 secs")).arg(QString::number(currentCount));
    }
    setWindowTitle(QString(tr("PINN v%1 - Built: %2 (%3) %4")).arg(VERSION_NUMBER).arg(QString::fromLocal8Bit(__DATE__)).arg(_ipaddress.toString()).arg(count));
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        ug->retranslateUI();
        update_window_title();
        updateNeeded();
        setModelInfo();
        if (_menuLabel)
            _menuLabel->setText(menutext(toolbar_index));
        //repopulate(); #@@ Needs all lists to be cleared & network downloads re-done. Better when osSource implemented.
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::displayMode(int modenr, bool silent)
{
#ifdef Q_WS_QWS
    QString cmd, mode;

    if (!silent && _displayModeBox)
    {
        /* User pressed another mode selection key while the confirmation box is being displayed */
        silent = true;
        _displayModeBox->close();
    }

    switch (modenr)
    {
    case 0:
        cmd  = "-p";
        mode = tr("HDMI preferred mode");
        break;
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
        // unknown mode
        return;
    }
    _currentMode = modenr;

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
    qDebug() << "Current overscan" << "top" << oTop << "bottom" << oBottom << "left" << oLeft << "right" << oRight;
    QScreen::instance()->setMode(xres-oLeft-oRight, yres-oTop-oBottom, 16);

    // Resize this window depending on screen resolution
    QRect s = QApplication::desktop()->screenGeometry();
    int w = s.width()-100;
    int h = s.height() - 100;

    w =qMin(w,700);
    h =qMin(h,500);
    resize(w,h);

    // Update UI item locations
//    QPixmap pixmap;
//    if (QFile::exists("/mnt/wallpaper.png"))
//    {
//        pixmap.load("/mnt/wallpaper.png");
//    }
//    else
//    {
//        pixmap.load(":/wallpaper.png");
//    }
    _splash->resize();
    _splash->setPixmap(_splash->pixmap()); //reposition, keeping same image.

    LanguageDialog *ld = LanguageDialog::instance("en", "gb");
    ld->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignHCenter | Qt::AlignBottom, ld->size(), qApp->desktop()->availableGeometry()));
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), qApp->desktop()->availableGeometry()));

    // Refresh screen
    qApp->processEvents();
    QWSServer::instance()->refresh();

    // In case they can't see the message box, inform that mode change
    // is occurring by turning on the LED during the change
    QProcess *led_blink = new QProcess(this);
    connect(led_blink, SIGNAL(finished(int)), led_blink, SLOT(deleteLater()));
    led_blink->start("sh -c \"echo 1 > /sys/class/leds/led0/brightness; sleep 3; echo 0 > /sys/class/leds/led0/brightness\"");

    // Inform user of resolution change with message box.
    if (!silent && _settings)
    {
        _displayModeBox = new QMessageBox(QMessageBox::Question,
                      tr("Display Mode Changed"),
                      tr("Display mode changed to %1\nWould you like to make this setting permanent?").arg(mode),
                      QMessageBox::Yes | QMessageBox::No);
        _displayModeBox->installEventFilter(this);
        _displayModeBox->exec();

        if (_displayModeBox->standardButton(_displayModeBox->clickedButton()) == QMessageBox::Yes)
        {
            _settings->setValue("display_mode", modenr);
            _settings->sync();
            ::sync();
        }
        _displayModeBox = NULL;
    }

    /*
    QMessageBox *mbox = new QMessageBox;
    mbox->setWindowTitle(tr("Display Mode Changed"));
    mbox->setText(QString(tr("Display mode changed to %1")).arg(mode));
    mbox->setStandardButtons(0);
    mbox->show();
    QTimer::singleShot(2000, mbox, SLOT(hide()));
    */

#else
    Q_UNUSED(modenr)
    Q_UNUSED(silent)
#endif
}

bool MainWindow::eventFilter(QObject *, QEvent *event)
{
    extern QApplication * gApp;

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // Let user find the best display mode for their display
        // experimentally by using keys 1-4. PINN will default to using HDMI preferred mode.

        // HDMI preferred mode
        if (keyEvent->key() == Qt::Key_1 && _currentMode != 0)
        {
            displayMode(0);
        }
        // HDMI safe mode
        if (keyEvent->key() == Qt::Key_2 && _currentMode != 1)
        {
            displayMode(1);
        }
        // Composite PAL
        if (keyEvent->key() == Qt::Key_3 && _currentMode != 2)
        {
            displayMode(2);
        }
         // Composite NTSC
        if (keyEvent->key() == Qt::Key_4 && _currentMode != 3)
        {
            displayMode(3);
        }

        if (keyEvent->key() == Qt::Key_Plus && fontsize < 20)
        {
            fontsize++;
            updateFont(fontsize);
        }
        if (keyEvent->key() == Qt::Key_Minus && fontsize >9)
        {
            fontsize--;
            updateFont(fontsize);
        }

        // Catch Return key to trigger OS boot
        if (keyEvent->key() == Qt::Key_Return)
        {
            on_list_doubleClicked(ug->list->currentIndex());
        }

        // cursor Right changes tab headings
        if (keyEvent->key() == Qt::Key_Right)
        {
            if (ug->tabs && toolbar_index !=TOOLBAR_MAINTENANCE) //Don't do if no tabs visisble
            {
                if (ug->tabs->count() > 0)
                {
                    int index = ug->tabs->currentIndex()+1;
                    if (index >= ug->tabs->count())
                        index =0;
                    ug->tabs->setCurrentIndex(index);
                }
            }
        }
        // cursor Left changes tab headings
        if (keyEvent->key() == Qt::Key_Left)
        {
            if (ug->tabs && toolbar_index !=TOOLBAR_MAINTENANCE) //Don't do if no tabs visisble
            {
                if (ug->tabs->count() > 0)
                {
                    int index = ug->tabs->currentIndex()-1;
                    if (index < 0)
                        index = ug->tabs->count()-1;
                    ug->tabs->setCurrentIndex(index);
                }
            }
        }

        if (_kc.at(_kcpos) == keyEvent->key())
        {
            _kcpos++;
            if (_kcpos == _kc.size())
            {
                inputSequence();
                _kcpos = 0;
            }
        }
        else
            _kcpos=0;
    }

    return false;
}

void MainWindow::inputSequence()
{
    _info = new QLabel(this);
    _info->setPixmap(QPixmap("/usr/data"));
    _info->setGeometry(0,0,640,480);
    _info->show();
    _infoDelay=10;
}

void MainWindow::on_actionAdvanced_triggered()
{

    toolbars.value(toolbar_index)->setVisible(false);
    toolbar_index = (toolbar_index+1)%NUM_TOOLBARS;
    toolbars.value(toolbar_index)->setVisible(true);

    ui->groupBox->setVisible(toolbar_index == TOOLBAR_MAIN);
    ui->groupBoxUsb->setVisible( (toolbar_index == TOOLBAR_ARCHIVAL) || (toolbar_index == TOOLBAR_MAINTENANCE));

    if (_menuLabel)
        _menuLabel->setText(menutext(toolbar_index));
    cec->setMenu( menutext(toolbar_index) );

    if (ug->tabs)
    {
        ug->tabs->currentWidget()->setVisible(toolbar_index != TOOLBAR_MAINTENANCE);
    }
    else
    {
        ui->list->setVisible(toolbar_index != TOOLBAR_MAINTENANCE);
    }

    ug->toggleInstalled(toolbar_index== TOOLBAR_MAINTENANCE );
}

void MainWindow::on_actionEdit_config_triggered()
{

    QListWidgetItem *item = ug->listInstalled->currentItem();

    if (item && item->data(Qt::UserRole).toMap().contains("partitions"))
    {
        QVariantMap m = item->data(Qt::UserRole).toMap();
        QVariantList l = m.value("partitions").toList();
        if (!l.isEmpty())
        {
            QString partition = l.first().toString();
            ConfEditDialog d(m, partition, this);
            d.exec();
        }
    }
}

void MainWindow::on_actionBrowser_triggered()
{

    startBrowser();
}

void MainWindow::fullFAT()
{

    setEnabled(false);
    _qpd = new QProgressDialog( tr("Wiping SD card"), QString(), 0, 0, this);
    _qpd->setWindowModality(Qt::WindowModal);
    _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    FullFatThread *fft = new FullFatThread(_bootdrive, this);
    connect(fft, SIGNAL(statusUpdate(QString)), _qpd, SLOT(setLabelText(QString)));
    connect(fft, SIGNAL(completed()), _qpd, SLOT(deleteLater()));
    connect(fft, SIGNAL(error(QString)), this, SLOT(onQpdError(QString)));
    connect(fft, SIGNAL(query(QString, QString, QMessageBox::StandardButton*)),
            this, SLOT(onQuery(QString, QString, QMessageBox::StandardButton*)),
            Qt::BlockingQueuedConnection);


    QProcess::execute("umount /settings");

    fft->start();
    _qpd->exec();
    _partInited = false;
    setEnabled(true);
}

void MainWindow::on_actionWipe_triggered()
{

    if (QMessageBox::warning(this,
                             tr("Confirm"),
                             tr("Warning: this will restore your PINN drive to its initial state. All existing data on the drive except PINN will be overwritten, including any OSes that are already installed."),
                             QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        fullFAT();
        QMessageBox::warning(this,
                        tr("Drive Wiped!"),
                        tr("Warning: Any installed OSes have been wiped and the drive has been restored to its original size.\nInstall and download will no longer work until this drive is re-booted.\nBooting this drive again will re-instate PINN's partition structure."),
                        QMessageBox::Ok);
        updateNeeded();
    }
}


bool MainWindow::requireNetwork()
{

    if (!isOnline())
    {
        QMessageBox::critical(this,
                              tr("No network access"),
                              tr("Wired network access is required for this feature. Please insert a network cable into the network port."),
                              QMessageBox::Close);
        return false;
    }

    return true;
}

void MainWindow::startBrowser()
{

    if (!requireNetwork())
        return;
    if (_proc)
    {
        _proc->kill();
        _proc=NULL;
    }

    _proc = new QProcess(this);
    QString lang = LanguageDialog::instance("en", "gb")->currentLanguage();
    if (lang == "gb" || lang == "us" || lang == "ko" || lang == "")
        lang = "en";
    _proc->start("arora -lang "+lang+" "+HOMEPAGE);
}

void MainWindow::on_list_doubleClicked(const QModelIndex &index)
{

    if (index.isValid())
    {
        QListWidgetItem *item = ug->list->currentItem();
        if (!item)
            return;
        if (item->checkState() == Qt::Unchecked)
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::backupConf(const QString &fconf)
{
    // Precondition - /settings exists and is already mounted r/w

    /* If user supplied a conf file on the FAT partition copy that one to settings regardless */
    if (QFile::exists("/settings/"+fconf))
    {
        qDebug() << "Backing up "+fconf;

        QProcess::execute("mount -o remount,rw /mnt");

        QFile::copy("/settings/"+fconf, "/mnt/"+fconf);
        QProcess::execute("sync");

        QProcess::execute("mount -o remount,ro /mnt");
    }
}

void MainWindow::backupWpa()
{
    backupConf("wpa_supplicant.conf");
}

void MainWindow::backupDhcp()
{
    backupConf("dhcpcd.conf");
}

void MainWindow::copyConf(const QString &fconf)
{

    //This file is the one used by dhcpcd
    QFile f("/settings/"+fconf);
    if ( f.exists() && f.size() == 0 )
    {
        /* Remove corrupt file */
        f.remove();
    }

    /* If user supplied a conf file on the FAT partition copy that one to settings regardless */
    if (QFile::exists("/mnt/"+fconf))
    {
        qDebug() << "Copying  user "+fconf+" to /settings";

        QProcess::execute("mount -o remount,rw /settings");
        QProcess::execute("mount -o remount,rw /mnt");

        QFile::remove("/settings/"+fconf+".bak");
        QFile::rename("/settings/"+fconf,"/settings/"+fconf+".bak");
        QFile::copy("/mnt/"+fconf, "/settings/"+fconf);
        f.setPermissions( QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther | QFile::ReadUser );

        /* rename the user file to avoid overwriting any manually set SSIDs */
        QFile::remove("/mnt/"+fconf+".bak");
        QFile::rename("/mnt/"+fconf,"/mnt/"+fconf+".bak");

        QProcess::execute("sync");
        //QProcess::execute("mount -o remount,ro /settings");
        QProcess::execute("mount -o remount,ro /mnt");
    }
    else if ( !f.exists() )
    {
        /* There is no existing file, must be first installation */
        qDebug() << "Copying /etc/"+fconf+" to /settings";
        QFile::copy("/etc/"+fconf, "/settings/"+fconf);
    }
}

void MainWindow::copyWpa()
{
    copyConf("wpa_supplicant.conf");
}

void MainWindow::copyDhcp()
{
    copyConf("dhcpcd.conf");
}

void MainWindow::startNetworking()
{

    /* Enable dbus so that we can use it to talk to wpa_supplicant later */
    qDebug() << "Starting dbus";
    QProcess::execute("/etc/init.d/S30dbus start");

    /* Run dhcpcd in background */
    QProcess *proc = new QProcess(this);
    qDebug() << "Starting dhcpcd";
    proc->start("/sbin/dhcpcd --noarp -f /settings/dhcpcd.conf -e wpa_supplicant_conf=/settings/wpa_supplicant.conf --denyinterfaces \"*_ap\"");

    if ( isOnline() )
    {
        onOnlineStateChanged(true);
    }
    else
    {
        /* We could ask Qt's Bearer management to notify us once we are online,
           but it tends to poll every 10 seconds.
           Users are not that patient, so lets poll ourselves every 0.1 second */
        //QNetworkConfigurationManager *_netconfig = new QNetworkConfigurationManager(this);
        //connect(_netconfig, SIGNAL(onlineStateChanged(bool)), this, SLOT(onOnlineStateChanged(bool)));
        connect(&_networkStatusPollTimer, SIGNAL(timeout()), SLOT(pollNetworkStatus()));
        _networkStatusPollTimer.start(100);
    }
}

bool MainWindow::isOnline()
{

    /* Check if we have an IP-address other than localhost */
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();

    foreach (QHostAddress a, addresses)
    {
        if (a != QHostAddress::LocalHost && a != QHostAddress::LocalHostIPv6)
        {
            _ipaddress=a;
            update_window_title();
            return true;
        }
    }
    return false;
}

void MainWindow::pollNetworkStatus()
{

    if (!_hasWifi && QFile::exists("/sys/class/net/wlan0"))
    {
        _hasWifi = true;
        ui->actionWifi->setEnabled(true);
    }
    if (isOnline())
    {
        _networkStatusPollTimer.stop();
        onOnlineStateChanged(true);
    }
}

void MainWindow::onOnlineStateChanged(bool online)
{
    TRACE
    if (online)
    {
        qDebug() << "Network up in" << _time.elapsed()/1000.0 << "seconds";
        if (!_netaccess)
        {
            QDir dir;
            dir.mkdir("/settings/cache");
            _netaccess = new QNetworkAccessManager(this);
            QNetworkDiskCache *_cache = new QNetworkDiskCache(this);
            _cache->setCacheDirectory("/settings/cache");
            _cache->setMaximumCacheSize(8 * 1024 * 1024);
            _cache->clear();
            _netaccess->setCache(_cache);
            _listno = 0;
            QNetworkConfigurationManager manager;
            _netaccess->setConfiguration(manager.defaultConfiguration());

            UpdateTime();
            QString cmdline = getFileContents("/proc/cmdline");
            if (!cmdline.contains("no_update"))
                checkForUpdates( EUPDATEAUTO );
            else
                qDebug()<<"Skipping self update check";

            qDebug() <<"Online! repo_list= "<<repoList;

            downloadRepoList(repoList);
        }
        else
            UpdateTime();   //Re-check the time everytime we come online

        ui->actionBrowser->setEnabled(true);
        emit networkUp();
    }
}

void MainWindow::downloadRepoList(const QString &urlstring)
{

    qDebug() << "downloadRepoList: " << urlstring;
    if (urlstring.isEmpty())
        downloadLists();
    else
    {
        if (urlstring.startsWith("http"))
        {
            QUrl url(urlstring);
            QNetworkRequest request(url);
            request.setRawHeader("User-Agent", AGENT);
            QNetworkReply *reply = _netaccess->get(request);
            connect(reply, SIGNAL(finished()), this, SLOT(downloadRepoListRedirectCheck()));
        }
        else
        {
            processRepoListJson( Json::parse(getFileContents(urlstring)) );
        }
    }
}

void MainWindow::downloadRepoListRedirectCheck()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString redirectionurl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    /* Set our clock to server time if we currently have an old date */
    setTime(reply);

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        qDebug() << "Redirection - Re-trying download from" << redirectionurl;
        downloadRepoList(redirectionurl);
    }
    else
        downloadRepoListComplete();
}

void MainWindow::downloadRepoListComplete()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != reply->NoError || httpstatuscode < 200 || httpstatuscode > 399)
    {
        if (_qpd)
        {
            _qpd->hide();
        }
        QString errstr = tr("Error downloading distribution list from Internet:\n") + reply->url().toString();
        qDebug() << "Error Downloading "<< reply->url()<<" reply: "<< reply->error() << " httpstatus: "<< httpstatuscode;
        QMessageBox::critical(this, tr("Download error"), errstr, QMessageBox::Close);
        downloadLists(); //Can't process repo_list, but maybe can get others.

    }
    else
    {
        processRepoListJson(Json::parse( reply->readAll() ));
    }

    reply->deleteLater();
}

void MainWindow::processRepoListJson(QVariant json)
{

    if (json.isNull())
    {
        QMessageBox::critical(this, tr("Error"), tr("Error parsing repolist.json downloaded from server"), QMessageBox::Close);
        return;
    }

    QVariantList list = json.toMap().value("repo_list").toList();

    foreach (QVariant osv, list)
    {
        QVariantMap  os = osv.toMap();

        if (os.contains("name"))
            QString basename = os.value("name").toString();
        if (os.contains("url"))
            downloadRepoUrls << os.value("url").toString();
    }

    downloadLists();
}

void MainWindow::downloadLists()
{

    _numIconsToDownload = 0;
    _numFilesToCheck = 0;
    QStringList urls = _repo.split(' ', QString::SkipEmptyParts);

    //Add-in PINN's list of repos
    urls << downloadRepoUrls;
    urls.removeDuplicates();

    if (urls.isEmpty())
    {
        //No network lists to download, so remove the dialog.
        if (_qpd)
        {
            _qpd->hide();
            _qpd->deleteLater();
            _qpd = NULL;
        }
    }
    else
    {
        foreach (QString url, urls)
        {
            if (url.startsWith("/"))
            {
                QByteArray org_file (getFileContents(url));
                QByteArray base_file(expandBaseUrl(org_file));

                processJson( Json::parse(base_file) );
            }
            else
                downloadList(url);
        }
    }
}

void MainWindow::downloadList(const QString &urlstring)
{

    _numListsToDownload++;
    QUrl url(urlstring);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", AGENT);
    QNetworkReply *reply = _netaccess->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadListRedirectCheck()));
}


void MainWindow::rebuildInstalledList()
{

    /* Recovery procedure for damaged settings partitions
     * Scan partitions for operating systems installed and regenerate a minimal
     * installed_os.json so that boot menu can function.
     */
    QDir dir;
    dir.mkdir("/mnt2");
    QVariantList installedlist;

    for (int i=5; i<=MAXIMUM_PARTITIONS; i++)
    {
        QString part = partdev(_drive, i);

        if (QFile::exists(part) && QProcess::execute("mount -t vfat "+part+" /mnt2") == 0)
        {
            qDebug() << "Scanning" << part;
            if (QFile::exists("/mnt2/os_config.json"))
            {
                QVariantMap m = Json::loadFromFile("/mnt2/os_config.json").toMap();
                QString f = m.value("flavour").toString();
                if (!f.isEmpty())
                {
                    qDebug() << "OS found:" << f;
                    QVariantMap osinfo;
                    osinfo.insert("name", f);
                    osinfo.insert("release_date", m.value("release_date"));
                    osinfo.insert("partitions", m.value("partitions"));
                    osinfo.insert("folder", m.value("imagefolder"));
                    osinfo.insert("description", m.value("description"));
                    installedlist.append(osinfo);
                }
            }
            QProcess::execute("umount /mnt2");
        }
    }

    if (!installedlist.isEmpty())
    {
        Json::saveToFile("/settings/installed_os.json", installedlist);
    }
}

void MainWindow::downloadListComplete()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    _numListsToDownload--;
    if (reply->error() != reply->NoError || httpstatuscode < 200 || httpstatuscode > 399)
    {
        if (_qpd)
        {
            _qpd->hide();
        }
        QString errstr = tr("Error downloading distribution list from Internet:\n") + reply->url().toString();
        qDebug() << "Error Downloading "<< reply->url()<<" reply: "<< reply->error() << " httpstatus: "<< httpstatuscode;
        QMessageBox::critical(this, tr("Download error"), errstr, QMessageBox::Close);
    }
    else
    {
        QByteArray org_file (reply->readAll());
        QByteArray base_file(expandBaseUrl(org_file));

        processJson(Json::parse( base_file ));
    }

    ug->setFocus();

    if (_numListsToDownload==0)
    {
        _availableImages |= ALLNETWORK;
        _processedImages |= ALLNETWORK;
    }
    reply->deleteLater();
}

QByteArray MainWindow::expandBaseUrl(QByteArray org_file)
{
    QByteArray pattern("baseurl");
    int pos = org_file.indexOf(pattern);
    if (-1 == pos)
        return (org_file);

    pos += 2+pattern.size();
    int i=0;
    while ((org_file[pos+i]!='\"') && (i<10))
    {
        i++;
    }
    i++;
    int j=0;
    QByteArray baseurl;
    while (org_file[pos+i+j]!='\"')
    {
        baseurl += org_file[pos+i+j];
        j++;
    }

    //replace baseurl in string
    const char * key="${baseurl}";
    org_file.replace(key,strlen(key),baseurl,baseurl.size());

    return (org_file);
}


void MainWindow::processJson(QVariant json)
{
    TRACE
    if (json.isNull())
    {
        QMessageBox::critical(this, tr("Error"), tr("Error parsing list.json downloaded from server"), QMessageBox::Close);
        return;
    }

    QSet<QString> iconurls;

    QString baseurl = json.toMap().value("baseurl").toString();

    QVariantList list = json.toMap().value("os_list").toList();

    foreach (QVariant osv, list)
    {
        QVariantMap  os = osv.toMap();

        OverrideJson(os);

        QString basename = os.value("os_name").toString();
        //qDebug() << "#" <<basename << " " << os.value("os_info").toString() << " " << os.value("release_date").toString();
        if (canInstallOs(basename, os))
        {
            if (os.contains("flavours"))
            {
                QVariantList flavours = os.value("flavours").toList();

                foreach (QVariant flv, flavours)
                {
                    QVariantMap flavour = flv.toMap();
                    QString name        = flavour.value("name").toString();

                    QVariantMap item = os;
                    item.remove("flavours");
                    item["name"]= name;
                    item["description"]=flavour.value("description").toString();
                    if (flavour.contains("icon"))
                        item["icon"]=flavour.value("icon").toString();
                    item["source"]=SOURCE_NETWORK;

                    processJsonOs(name, item, iconurls);
                }
            }
            else if (os.contains("description"))
            {
                os["name"] = basename;
                os["source"] = SOURCE_NETWORK;
                processJsonOs(basename, os, iconurls);
            }
        }
    }

    /* Download icons */
    if (!iconurls.isEmpty())
    {
         _numIconsToDownload += iconurls.count();
        foreach (QString iconurl, iconurls)
        {
            //QString origurl(iconurl);
            //if (origurl.endsWith("/download"))
            //    origurl.chop(9);
            downloadIcon(iconurl, iconurl);
        }
    }
    else
    {
        if (_qpd)
        {
            _qpd->deleteLater();
            _qpd = NULL;
        }
    }
    updateInstalledStatus();

    filterList();
}

void MainWindow::processJsonOs(const QString &name, QVariantMap &new_details, QSet<QString> &iconurls)
{

    QIcon internetIcon(":/icons/download.png");
    bool bInstalled = false;
    QListWidgetItem *witem = findItemByName(name);
    if (!witem)
    {
        QString iconurl = new_details.value("icon").toString();
        if (!iconurl.isEmpty())
            iconurls.insert(iconurl);
    }
    if (CORE(name)==RECOMMENDED_IMAGE)
        new_details["recommended"]=true;
    addImage(new_details,internetIcon,bInstalled);
    if (! new_details.contains("download_size"))
    {
        getDownloadSize(new_details);
    }
}

void MainWindow::getDownloadSize(QVariantMap &new_details)
{

    qint64 downloadSize=0;    //Start off with a minimum of 2MB for all metadata files.

    QString name;
    if (new_details.contains("name"))
        name = new_details.value("name").toString();
    else if (new_details.contains("os_name"))
        name = new_details.value("os_name").toString();
    else
        return;

    //Set downloadsize to minimum
    if (! new_details.contains("download_size"))
    {
        new_details.insert("download_size",downloadSize);
    }
    else
    {
        new_details["download_size"] = downloadSize;
    }
    //Kick off a new filesize calc.

    QStringList tarballs = new_details.value("tarballs").toStringList();
    foreach (QString tarball, tarballs)
    {
        if ( !tarball.isEmpty() )
            checkFileSize(tarball, name);
    }
}


void MainWindow::downloadIcon(const QString &urlstring, const QString &originalurl)
{

    iconcache cache;
    QPixmap pix;
    if (cache.readPixmapFromCache(urlstring, pix))
    {
        assignPixmap(urlstring, pix);

        if (--_numIconsToDownload == 0 && _qpd)
        {
            _qpd->hide();
            _qpd->deleteLater();
            _qpd = NULL;
        }
    }
    else
    {
        QUrl url(urlstring);
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::User, originalurl);
        request.setRawHeader("User-Agent", AGENT);
        QNetworkReply *reply = _netaccess->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(downloadIconRedirectCheck()));
    }
}

QListWidgetItem *MainWindow::findItemByName(const QString &name)
{
    TRACE
    QList<QListWidgetItem *> all;
    all = ug->allItems();

    foreach (QListWidgetItem *item, all)
    {
        QVariantMap m = item->data(Qt::UserRole).toMap();
        QString name1 = name;
        if (m.value("name").toString().replace(" ","_") == name1.replace(" ","_"))
        {
            DBG("Found");
            return item;
        }
    }
    DBG("not found");
    return NULL;
}

void MainWindow::downloadIconComplete()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QString url = reply->url().toString();
    QString originalurl = reply->request().attribute(QNetworkRequest::User).toString();
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != reply->NoError || httpstatuscode < 200 || httpstatuscode > 399)
    {
        //QMessageBox::critical(this, tr("Download error"), tr("Error downloading icon '%1'").arg(reply->url().toString()), QMessageBox::Close);
        qDebug() << "Error "<< httpstatuscode << " downloading icon " << url;
    }
    else
    {
        QPixmap pix;
        pix.loadFromData(reply->readAll());

        assignPixmap(originalurl, pix);
    }
    if (--_numIconsToDownload == 0 && _qpd)
    {
        _qpd->hide();
        _qpd->deleteLater();
        _qpd = NULL;
    }

    reply->deleteLater();
    _listno++;
}

void MainWindow::assignPixmap(QString originalurl, QPixmap &pix)
{
    iconcache cache;
    cache.storePixmapInCache(originalurl, pix);
    QIcon icon(pix);

    //Set the icon in the OS list dialog box.
    QList<QListWidgetItem *> all;
    all = ug->allItems();

    for (int i=0; i<ug->count(); i++)
    {
        QVariantMap m = all.value(i)->data(Qt::UserRole).toMap();
        if (m.value("icon") == originalurl)
        {
            all.value(i)->setIcon(icon);
        }
    }
}

QList<QListWidgetItem *> MainWindow::selectedItems()
{

    return(ug->selectedItems());
}

void MainWindow::updateNeeded()
{

    bool enableWrite = false;
    bool enableDownload = false;
    qint64 neededDownload=0;
    QColor colorNeededLabel = Qt::black;
    bool bold = false;

    QString num2chk="";
    if (_numFilesToCheck)
    {
        num2chk = "Files to check: " + QString::number(_numFilesToCheck);
    }
    _checkLabel->setText(num2chk);

    _neededMB = 0;
    QList<QListWidgetItem *> selected = selectedItems();

    foreach (QListWidgetItem *item, selected)
    {
        QVariantMap entry = item->data(Qt::UserRole).toMap();
        _neededMB += entry.value("nominal_size").toUInt();

        if (entry.value("source").toString() == SOURCE_NETWORK)
        {
            neededDownload += entry.value("download_size").toUInt();
        }

        if (nameMatchesRiscOS(entry.value("name").toString()))
        {
            /* RiscOS needs to start at a predetermined sector, calculate the extra space needed for that */
            uint startSector = getFileContents(sysclassblock(_drive, 5)+"/start").trimmed().toUInt()+getFileContents(sysclassblock(_drive, 5)+"/size").trimmed().toUInt();
            if (RISCOS_SECTOR_OFFSET > startSector)
            {
                _neededMB += (RISCOS_SECTOR_OFFSET - startSector)/2048;
            }
        }
    }
    _neededDownloadMB = neededDownload / (1024*1024);

    ui->neededLabel->setText(QString("%1: %2 MB").arg(tr("Needed"), QString::number(_neededMB)));
    ui->availableLabel->setText(QString("%1: %2 MB").arg(tr("Available"), QString::number(_availableMB)));

    ui->neededLabelUsb->setText(QString("%1: %2 MB").arg(tr("Needed"), QString::number(_neededDownloadMB)));
    ui->availableLabelUsb->setText(QString("%1: %2 MB").arg(tr("Available"), QString::number(_availableDownloadMB)));

    /************ Write Image Control **********/

    if (_neededMB > _availableMB)
    {
        /* Selection exceeds available space, make label red to alert user */
        colorNeededLabel = Qt::red;
        bold = true;
    }
    else
    {
        if (_neededMB)
        {
            /* Enable OK button if a selection has been made that fits on the card */
            enableWrite = true;
        }
    }

    ui->actionWrite_image_to_disk->setEnabled(enableWrite);

    QPalette p = ui->neededLabel->palette();
    if (p.color(QPalette::WindowText) != colorNeededLabel)
    {
        p.setColor(QPalette::WindowText, colorNeededLabel);
        ui->neededLabel->setPalette(p);
    }
    QFont font = ui->neededLabel->font();
    font.setBold(bold);
    ui->neededLabel->setFont(font);

    /************ Download Image Control **********/
    if (_neededDownloadMB > _availableDownloadMB)
    {
        /* Selection exceeds available space, make label red to alert user */
        colorNeededLabel = Qt::red;
        bold = true;
    }
    else
    {
        colorNeededLabel = Qt::black;
        if (selected.count())
        {
            /* Enable Download button if a selection has been made that fits on the card */
            /* AND all download sizes are calculated */
            enableDownload = (_numFilesToCheck==0);
        }
    }

    ui->actionDownload->setEnabled(enableDownload);

    p = ui->neededLabelUsb->palette();
    if (p.color(QPalette::WindowText) != colorNeededLabel)
    {
        p.setColor(QPalette::WindowText, colorNeededLabel);
        ui->neededLabelUsb->setPalette(p);
    }
    font = ui->neededLabelUsb->font();
    font.setBold(bold);
    ui->neededLabelUsb->setFont(font);

}

void MainWindow::updateActions()
{


    //For the INSTALLED list...
    QListWidgetItem *item = ug->listInstalled->currentItem();
    if (ug->listInstalled->count() && !item)
    {
        item = ug->listInstalled->item(0);
        ug->listInstalled->setCurrentItem(item);
    }
    //item may still be NULL
    //Only need to make sure item is not null, since any item here is already installed    ?
    ui->actionEdit_config->setEnabled(item);
    ui->actionPassword->setEnabled(item && (item != ug->listInstalled->item(0)));
    ui->actionInfoInstalled->setEnabled(item && item->data(Qt::UserRole).toMap().contains("url"));
    ui->actionRename->setEnabled( item && (item != ug->listInstalled->item(0)) );

    QList<QListWidgetItem *> select = ug->selectedInstalledItems();
    int count = select.count();
    ui->actionRepair->setEnabled( count );
    ui->actionReinstall->setEnabled( count );

    item = ug->listInstalled->item(0); //First item in list is always PINN
    if (item->checkState()) //Cannot replace PINN with something else!
        count--;
    ui->actionReplace->setEnabled( count );
    ui->actionBackup->setEnabled(count && !_osdrive.isEmpty() );

    //For the normal list...
    item = ug->list->currentItem();
    if (ug->list->count() && !item)
    {
        item = ug->list->item(0);
        ug->list->setCurrentItem(item);
    }
    ui->actionInfo->setEnabled(item && item->data(Qt::UserRole).toMap().contains("url"));
    //others...

    if (_menuLabel)
        _menuLabel->setText(menutext(toolbar_index));
}

void MainWindow::on_list_itemChanged(QListWidgetItem *item)
{

    Q_UNUSED(item);
    updateNeeded();
    updateActions();
}

void MainWindow::downloadMetaFile(const QString &urlstring, const QString &saveAs)
{

    //qDebug() << "Downloading" << urlstring << "to" << saveAs;
    _numMetaFilesToDownload++;
    QUrl url(urlstring);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, saveAs);
    request.setRawHeader("User-Agent", AGENT);
    QNetworkReply *reply = _netaccess->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadMetaRedirectCheck()));
}

void MainWindow::downloadListRedirectCheck()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString redirectionurl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    /* Set our clock to server time if we currently have an old date */
    setTime(reply);

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        _numListsToDownload--;
        downloadList(redirectionurl);
    }
    else
        downloadListComplete();
}

void MainWindow::downloadIconRedirectCheck()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString redirectionurl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QString originalurl = reply->request().attribute(QNetworkRequest::User).toString();;

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        downloadIcon(redirectionurl, originalurl);
    }
    else
        downloadIconComplete();
}

void MainWindow::downloadMetaRedirectCheck()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString redirectionurl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QString saveAs = reply->request().attribute(QNetworkRequest::User).toString();

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        _numMetaFilesToDownload--;
        downloadMetaFile(redirectionurl, saveAs);
    }
    else
        downloadMetaComplete();
}

void MainWindow::downloadMetaComplete()
{


    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    //A filename string starting with '-' is optional, so not an error if not present
    QString saveAs = reply->request().attribute(QNetworkRequest::User).toString();
    bool ignoreError=false;
    if (saveAs.startsWith('-'))
    {
        ignoreError=true;
        saveAs=saveAs.mid(1);
    }

    if (reply->error() != reply->NoError || httpstatuscode < 200 || httpstatuscode > 399)
    {
        if (ignoreError)
        {
            _numMetaFilesToDownload--;
        }
        else
        {
            if (_qpd)
            {
                _qpd->hide();
                _qpd->deleteLater();
                _qpd = NULL;
            }
            QString error;
            error = tr("Error downloading meta file: ")+reply->url().toString();
            qDebug() << error;

            if (_eDownloadMode == MODE_DOWNLOAD)
            {   //Create an error file to prevent this download, but keep going for others
                QFileInfo finfo(saveAs);
                QString path = finfo.path() + QString("/error.log");
                QFile f(path);
                f.open(f.Append);
                f.write(error.toAscii());
                f.close();
            }
            else
            {   //When installing, a missing meta file is critical so stop.
                QMessageBox::critical(this, tr("Download error"), tr("Error downloading meta file")+"\n"+reply->url().toString(), QMessageBox::Close);
                setEnabled(true);
            }
        }
    }
    else
    {
        QFile f(saveAs);
        f.open(f.WriteOnly);
        if (f.write(reply->readAll()) == -1)
        {
            QMessageBox::critical(this, tr("Download error"), tr("Error writing downloaded file to SD card. SD card or file system may be damaged."), QMessageBox::Close);
            setEnabled(true);
        }
        else
        {
            _numMetaFilesToDownload--;
        }
        f.close();

        //Check for flavours.tar.xz and untar it.
        QFileInfo fi (saveAs);
        if (fi.fileName() == "flavours.tar.xz")
        {
            qDebug() << "extracting "<<saveAs;
            QFileInfo fi(saveAs);
            QString path = fi.path();
            QString filename = fi.fileName();
            QString cmd = "sh -o pipefail -c \"";

            cmd += "cd "+path+"; ";

            cmd += "xz -dc";
            cmd += " "+filename;

            cmd += " | bsdtar -xf - ";
            cmd += " --no-same-owner ";
            cmd += "\"";

            qDebug() << "Executing:" << cmd;

            QProcess p;
            p.setProcessChannelMode(p.MergedChannels);
            p.start(cmd);
            p.closeWriteChannel();
            p.waitForFinished(-1);

            if (p.exitCode() != 0)
            {
                QByteArray msg = p.readAll();
                qDebug() << msg;
            }
            else
                QFile::remove(saveAs);
        }
        if (fi.fileName() == "partition_setup.sh")
        {
            int errorcode;
            QString csum_download = readexec(1,"sha512sum "+fi.absoluteFilePath(), errorcode).split(" ").first();
        }
    }
    if (_numMetaFilesToDownload == 0)
    {
        if (_qpd)
        {
            _qpd->hide();
            _qpd->deleteLater();
            _qpd = NULL;
        }
        if (_eDownloadMode == MODE_DOWNLOAD)
            startImageDownload();
        else if ( (_eDownloadMode == MODE_REINSTALL) || (_eDownloadMode == MODE_REINSTALLNEWER) || (_eDownloadMode == MODE_REPLACE))
            startImageReinstall();
        else
            startImageWrite();
    }
}

void MainWindow::checkFileSize(const QString &urlstring, const QString &osname)
{

    qDebug() << "checking size of file: " << urlstring;
    QUrl url(urlstring);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, osname);
    request.setRawHeader("User-Agent", AGENT);
    QNetworkReply *reply = _netaccess->head(request);
    _numFilesToCheck++;

    if (_numFilesToCheck==1)
        updateNeeded();     //Hide the download button

    connect(reply, SIGNAL(finished()), this, SLOT(checkFileSizeRedirectCheck()));
}

void MainWindow::checkFileSizeRedirectCheck()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString redirectionurl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    const QString &osname = reply->request().attribute(QNetworkRequest::User).toString();

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        _numFilesToCheck--;
        checkFileSize(redirectionurl, osname);
    }
    else
        checkFileSizeComplete();
}

void MainWindow::checkFileSizeComplete()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QString &osname = reply->request().attribute(QNetworkRequest::User).toString();

    quint64 length = reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();

    if (reply->error() != reply->NoError || httpstatuscode < 200 || httpstatuscode > 399)
    {
        qDebug() << tr("filesize error checking ")+reply->url().toString();
    }
    else
    {
        //find QVariantMap of osname
        QListWidgetItem *witem = findItemByName(osname);
        if (witem)
        {
            QVariantMap existing_details = witem->data(Qt::UserRole).toMap();
            if (existing_details["source"]==SOURCE_NETWORK)
            {    //Only update if it is still a network source!

                //get its current download_size
                quint64 old_size = existing_details.value("download_size").toULongLong();
                //Increment by length
                old_size += length;
                //write back
                existing_details["download_size"] = old_size;
                witem->setData(Qt::UserRole,existing_details);
            }
        }
        else
        {
            qDebug() << "Cannot find " << osname << " to set download_size";
        }
    }
    //decrement number of filesizes to check
    _numFilesToCheck--;
    qDebug() << "Length:" << length << "files left: " <<_numFilesToCheck << " " << osname;
    if (_numFilesToCheck<2)
        updateNeeded();
}

void MainWindow::startImageWrite()
{
    TRACE
    _piDrivePollTimer.stop();
    /* All meta files downloaded, extract slides tarball, and launch image writer thread */
    MultiImageWriteThread *imageWriteThread = new MultiImageWriteThread(_bootdrive, _drive, _noobsconfig, _provision);
    QString folder, slidesFolder;
    QStringList slidesFolders;

    QList<QListWidgetItem *> selected = selectedItems();
    foreach (QListWidgetItem *item, selected)
    {
        QVariantMap entry = item->data(Qt::UserRole).toMap();

        QString settingsfolder = "/settings/os/"+CORE(entry.value("name").toString());
        settingsfolder.replace(' ', '_');

        if (entry.contains("folder"))
        {
            /* Local image */
            folder = entry.value("folder").toString();
        }
        else
        {
            folder = settingsfolder;

            QString marketingTar = folder+"/marketing.tar";
            if (QFile::exists(marketingTar))
            {
                /* Extract tarball with slides */
                QProcess::execute("tar xf "+marketingTar+" -C "+folder);
                QFile::remove(marketingTar);
            }

            /* Insert tarball download URL information into partition_info.json */
            QVariantMap json = Json::loadFromFile(folder+"/partitions.json").toMap();
            QVariantList partitions = json["partitions"].toList();
            int i=0;
            QStringList tarballs = entry.value("tarballs").toStringList();
            foreach (QString tarball, tarballs)
            {
                QVariantMap partition = partitions[i].toMap();
                partition.insert("tarball", tarball); //change to download
                partitions[i] = partition;
                i++;
            }
            json["partitions"] = partitions;

            //Store configpath in /settings/os/<osname>/partitions.json
            //So that  noobsconfig knows the URL of the flavour customisations.
            //Assume they are where the os.json file is.
            QFileInfo fi(entry.value("os_info").toString());
            json["configpath"] = fi.path();

            Json::saveToFile(folder+"/partitions.json", json);
        }

        slidesFolder.clear();
        if (QFile::exists(folder+"/slides_vga"))
        {
            slidesFolder = folder+"/slides_vga";
        }
        else if (QFile::exists(settingsfolder+"/slides_vga"))
        {
            slidesFolder = settingsfolder+"/slides_vga";
        }

        QString sTerms(folder+"/terms");
        bool allowContinue=true;
        if (QFile(sTerms).exists())
        {
            TermsDialog dlg( CORE(entry.value("name").toString()), sTerms);
            if (dlg.exec() != dlg.Accepted)
                allowContinue = false;
        }
        if (allowContinue)
        {
            imageWriteThread->addImage(folder, entry.value("name").toString());
            if (!slidesFolder.isEmpty())
                slidesFolders.append(slidesFolder);
        }

    }


#if 1
    QString cmdline = getFileContents("/proc/cmdline");
    if (! cmdline.contains("silentinstall"))
    {
        adjustSizes dlg(_provision, _bootdrive, _drive, imageWriteThread->getImages(), 0);
        if (dlg.exec() != QDialog::Accepted)
        {
            setEnabled(true);
            _piDrivePollTimer.start(POLLTIME);
            return;
        }
    }
#endif
   if (slidesFolders.isEmpty())
        slidesFolder.append("/mnt/defaults/slides");

    _qpssd = new ProgressSlideshowDialog(slidesFolders, "", 20, this); //_drive
    _qpssd->setWindowTitle("Installing Images");
    connect(imageWriteThread, SIGNAL(parsedImagesize(qint64)), _qpssd, SLOT(setMaximum(qint64)));
    connect(imageWriteThread, SIGNAL(completed(int)), this, SLOT(onCompleted(int)));
    connect(imageWriteThread, SIGNAL(error(QString)), this, SLOT(onError(QString)), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(errorContinue(QString)), this, SLOT(onErrorContinue(QString)));
    connect(imageWriteThread, SIGNAL(statusUpdate(QString)), _qpssd, SLOT(setLabelText(QString)));
    connect(imageWriteThread, SIGNAL(checksumError(const QString&, const QString&, QMessageBox::ButtonRole*)), this, SLOT(onChecksumError(QString,QString,QMessageBox::ButtonRole*)),Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(newDrive(const QString&,eProgressMode)), _qpssd , SLOT(setDriveMode(const QString&,eProgressMode)), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(startAccounting()), _qpssd, SLOT(startAccounting()), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(stopAccounting()), _qpssd , SLOT(stopAccounting()), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(idle()), _qpssd , SLOT(idle()));
    connect(imageWriteThread, SIGNAL(cont()), _qpssd , SLOT(cont()));
    connect(imageWriteThread, SIGNAL(consolidate()), _qpssd , SLOT(consolidate()));
    connect(imageWriteThread, SIGNAL(finish()), _qpssd , SLOT(finish()));

    connect(_qpssd, SIGNAL(rejected()), this, SLOT(onErrorNoMsg()), Qt::BlockingQueuedConnection);

    imageWriteThread->start();
    hide();
    _qpssd->exec();
}

void MainWindow::startImageReinstall()
{
    TRACE
    _piDrivePollTimer.stop();
    /* All meta files downloaded, extract slides tarball, and launch image writer thread */

    MultiImageWriteThread *imageWriteThread = new MultiImageWriteThread(_bootdrive, _drive, _noobsconfig, _provision, false, _eDownloadMode);
    QString folder, slidesFolder;
    QStringList slidesFolders;

    foreach (QVariantMap entry, _newList)
    {
        int i=0;

        QString settingsfolder = "/settings/os/"+CORE(entry.value("name").toString());
        settingsfolder.replace(' ', '_');

        if (entry.contains("folder"))
        {
            /* Local image */
            folder = entry.value("folder").toString();

            QVariantMap json = Json::loadFromFile(folder+"/partitions.json").toMap();
            QVariantList partitions = json["partitions"].toList();
            i = partitions.count();
        }
        else
        {
            folder = settingsfolder;

            QString marketingTar = folder+"/marketing.tar";
            if (QFile::exists(marketingTar))
            {
                /* Extract tarball with slides */
                QProcess::execute("tar xf "+marketingTar+" -C "+folder);
                QFile::remove(marketingTar);
            }

            /* Insert tarball download URL information into partition_info.json */
            QVariantMap json = Json::loadFromFile(folder+"/partitions.json").toMap();
            QVariantList partitions = json["partitions"].toList();

            QStringList tarballs = entry.value("tarballs").toStringList();
            foreach (QString tarball, tarballs)
            {
                QVariantMap partition = partitions[i].toMap();
                partition.insert("tarball", tarball); //change to download
                partitions[i] = partition;
                i++;
            }
            i=partitions.count();
            json["partitions"] = partitions;
            Json::saveToFile(folder+"/partitions.json", json);
        }

        slidesFolder.clear();
        if (QFile::exists(folder+"/slides_vga"))
        {
            slidesFolder = folder+"/slides_vga";
        }
        else if (QFile::exists(settingsfolder+"/slides_vga"))
        {
            slidesFolder = settingsfolder+"/slides_vga";
        }

        QVariantMap installedEntry = entry.value("existingOS").toMap();

        QVariantList iPartitions = installedEntry.value("partitions").toList();
        int nInstalledParts = iPartitions.count();

        if (i == nInstalledParts)
        {

            QString sTerms(folder+"/terms");
            bool allowContinue=true;
            if (QFile(sTerms).exists())
            {
                TermsDialog dlg( CORE(entry.value("name").toString()), sTerms);
                if (dlg.exec() != dlg.Accepted)
                    allowContinue = false;
            }
            if (allowContinue)
            {
                imageWriteThread->addInstalledImage(folder, entry.value("name").toString(), installedEntry);

            }
        }
        else
        {
            QMessageBox::warning(this,tr("Replace OSes"),
                                 entry.value("name").toString() + tr(" has an incompatible number of partitions with ") + installedEntry.value("name").toString(),QMessageBox::Close);
        }

        if (!slidesFolder.isEmpty())
            slidesFolders.append(slidesFolder);
    }
    if (slidesFolders.isEmpty())
        slidesFolder.append("/mnt/defaults/slides");

    _qpssd = new ProgressSlideshowDialog(slidesFolders, "", 20,  this); //_drive
    _qpssd->setWindowTitle("Re-Installing Images");
    connect(imageWriteThread, SIGNAL(parsedImagesize(qint64)), _qpssd, SLOT(setMaximum(qint64)));
    connect(imageWriteThread, SIGNAL(completed(int)), this, SLOT(onCompleted(int)));
    connect(imageWriteThread, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(imageWriteThread, SIGNAL(errorContinue(QString)), this, SLOT(onErrorContinue(QString)), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(statusUpdate(QString)), _qpssd, SLOT(setLabelText(QString)));
    connect(imageWriteThread, SIGNAL(checksumError(const QString&, const QString&, QMessageBox::ButtonRole*)), this, SLOT(onChecksumError(QString,QString,QMessageBox::ButtonRole*)),Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(newDrive(const QString&,eProgressMode)), _qpssd , SLOT(setDriveMode(const QString&,eProgressMode)), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(startAccounting()), _qpssd, SLOT(startAccounting()), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(stopAccounting()), _qpssd , SLOT(stopAccounting()), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(idle()), _qpssd , SLOT(idle()));
    connect(imageWriteThread, SIGNAL(cont()), _qpssd , SLOT(cont()));
    connect(imageWriteThread, SIGNAL(consolidate()), _qpssd , SLOT(consolidate()));
    connect(imageWriteThread, SIGNAL(finish()), _qpssd , SLOT(finish()));

    connect(_qpssd, SIGNAL(rejected()), this, SLOT(NoMsg()), Qt::BlockingQueuedConnection);

    imageWriteThread->start();
    hide();
    _qpssd->exec();

}


void MainWindow::startImageDownload()
{

    _piDrivePollTimer.stop();
    // The drive is already mounted R/W from on_actionDownload_triggered

    /* All meta files downloaded, extract slides tarball, and launch image download thread */
    MultiImageDownloadThread *imageDownloadThread = new MultiImageDownloadThread(0, _local, _osdrive);
    QString folder, slidesFolder;
    QStringList slidesFolders;

    if (QMessageBox::question(this,
                              tr("Resume partial downloads?"),
                              tr("Normally select NO, unless your last download was not successful, in which case select YES"),
                              QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        imageDownloadThread->allowResume(true);
    }
    else
    {
        imageDownloadThread->allowResume(false);
    }


    QList<QListWidgetItem *> selected = selectedItems();
    foreach (QListWidgetItem *item, selected)
    {
        QVariantMap entry = item->data(Qt::UserRole).toMap();

        if (entry.contains("folder"))
        {
            /* Local image */
            folder = entry.value("folder").toString();
            /* No need to download these! */
        }
        else
        {
            folder = _local+"/os/"+entry.value("name").toString();
            folder.replace(' ', '_');

            QString errorlog = folder+"/error.log";
            if (QFile::exists(errorlog))
            {
                qDebug() << "Skipping due to error.log";
                continue;
            }

            QString marketingTar = folder+"/marketing.tar";
            if (QFile::exists(marketingTar))
            {
                /* Extract tarball with slides */
                QProcess::execute("tar xf "+marketingTar+" -C "+folder);
                QFile::remove(marketingTar);
            }

            /* Insert tarball download URL information into partition_info.json to allow download */
            if (QFile::exists(folder+"/partitions.json"))
            {
                QVariantMap json = Json::loadFromFile(folder+"/partitions.json").toMap();
                QVariantList partitions = json["partitions"].toList();
                int i=0;
                QStringList tarballs = entry.value("tarballs").toStringList();
                foreach (QString tarball, tarballs)
                {
                    QVariantMap partition = partitions[i].toMap();
                    partition.insert("download", tarball);
                    partitions[i] = partition;
                    i++;
                }
                json["partitions"] = partitions;
                Json::saveToFile(folder+"/partitions.json", json);
            }
            else
                continue;

            slidesFolder.clear();
            if (QFile::exists(folder+"/slides_vga"))
            {
                slidesFolder = folder+"/slides_vga";
            }

            /* Insert download_size into os.json to allow correct use of download size */
            if (QFile::exists(folder+"/os.json"))
            {
                QVariantMap json = Json::loadFromFile(folder+"/os.json").toMap();
                if (! json.contains("download_size"))
                {
                    quint64 downloadSize= entry.value("download_size").toULongLong();
                    json.insert("download_size",downloadSize);
                    Json::saveToFile(folder+"/os.json", json);
                }
            }
            else
                continue;

            QString sTerms(folder+"/terms");
            bool allowContinue=true;
            if (QFile(sTerms).exists())
            {
                TermsDialog dlg( CORE(entry.value("name").toString()), sTerms);
                if (dlg.exec() != dlg.Accepted)
                    allowContinue = false;
            }
            if (allowContinue)
            {
                imageDownloadThread->addImage(folder, entry.value("name").toString());
                if (!slidesFolder.isEmpty())
                    slidesFolders.append(slidesFolder);

            }
        }
    }

    if (slidesFolders.isEmpty())
        slidesFolder.append("/mnt/defaults/slides");

    _qpssd = new ProgressSlideshowDialog(slidesFolders, "", 20, this); //_osdrive
    _qpssd->setWindowTitle("Downloading Images");
    connect(imageDownloadThread, SIGNAL(parsedImagesize(qint64)), _qpssd, SLOT(setMaximum(qint64)));
    connect(imageDownloadThread, SIGNAL(completed()), this, SLOT(onCompleted()));
    connect(imageDownloadThread, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(imageDownloadThread, SIGNAL(errorContinue(QString)), this, SLOT(onErrorContinue(QString)), Qt::BlockingQueuedConnection);
    connect(imageDownloadThread, SIGNAL(statusUpdate(QString)), _qpssd, SLOT(setLabelText(QString)));
    connect(imageDownloadThread, SIGNAL(imageWritten(QString)), this, SLOT(newImage(QString)));

    connect(imageDownloadThread, SIGNAL(newDrive(const QString&,eProgressMode)), _qpssd , SLOT(setDriveMode(const QString&,eProgressMode)), Qt::BlockingQueuedConnection);
    connect(imageDownloadThread, SIGNAL(startAccounting()), _qpssd, SLOT(startAccounting()), Qt::BlockingQueuedConnection);
    connect(imageDownloadThread, SIGNAL(stopAccounting()), _qpssd , SLOT(stopAccounting()), Qt::BlockingQueuedConnection);
    connect(imageDownloadThread, SIGNAL(idle()), _qpssd , SLOT(idle()));
    connect(imageDownloadThread, SIGNAL(cont()), _qpssd , SLOT(cont()));
    connect(imageDownloadThread, SIGNAL(consolidate()), _qpssd , SLOT(consolidate()));
    connect(imageDownloadThread, SIGNAL(finish()), _qpssd , SLOT(finish()));

    connect(_qpssd, SIGNAL(rejected()), this, SLOT(onErrorNoMsg()), Qt::BlockingQueuedConnection);

    imageDownloadThread->start();
    hide();
    _qpssd->exec();
    show();

    //QProcess::execute("mount -o remount,ro /mnt");
}

void MainWindow::startImageBackup()
{
    _piDrivePollTimer.stop();
    // The drive is already mounted R/W from on_actionBackup_triggered

    /* All meta files downloaded, extract slides tarball, and launch image download thread */
    BackupThread *bt = new BackupThread(0, _local);
    QString folder, slidesFolder;
    QStringList slidesFolders;

    QString cmd;
    int i=1;

    QList<QListWidgetItem *> selected = ug->selectedInstalledItems();
    foreach (QListWidgetItem *item, selected)
    {
        //Mount every partition of each selected OS
        QVariantMap entry = item->data(Qt::UserRole).toMap();
        {
            QVariantList PartitionList = entry.value("partitions").toList();
            foreach (QVariant pv, PartitionList)
            {
                QString part = pv.toString();
                QString mntpoint = "/tmp/media/p"+QString::number(i);

                cmd = "mkdir -p "+mntpoint;
                QProcess::execute(cmd);

                cmd = "mount "+part +" "+mntpoint;
                QProcess::execute(cmd);
                i++;
            }
        }
    }
    // Capture the partition sizes of all selected mounted OSes
    cmd = "sh -c \"df >/tmp/df.txt\"";
    QProcess::execute(cmd);

    //Process the sizes of each partition
    i=1;
    qulonglong backupSpaceMB = 0;
    foreach (QListWidgetItem *item, selected)
    {
        QVariantMap entry = item->data(Qt::UserRole).toMap();
        {
            qulonglong overall = 0;
            QVariantList partSizes;
            QVariantList PartitionList = entry.value("partitions").toList();
            foreach (QVariant pv, PartitionList)
            {
                QString part = pv.toString();
                QString mntpoint = "/tmp/media/p"+QString::number(i);
                QString fname = "/tmp/size"+QString::number(i)+".txt";

                QString dev = getDevice(part);

                cmd = "sh -c \"grep "+ dev +" /tmp/df.txt >/tmp/sizes.txt\""; QProcess::execute(cmd);
                cmd = "sh -c \"sed -i 's/ \\+/ /g' /tmp/sizes.txt\""; QProcess::execute(cmd);
                //get USED space in 1K blocks
                cmd = "sh -c \"cat /tmp/sizes.txt | cut -d ' ' -f 3 >"+fname+"\""; QProcess::execute(cmd);

                QByteArray size = getFileContents(fname).trimmed(); //in KB
                qulonglong lsize = size.toULongLong();
                overall += lsize;
                lsize /= 1024;
                lsize++; //MBs
                QVariant qv = lsize;
                partSizes.append(qv);

                //Accumulate total backup space required
                backupSpaceMB += lsize;

                //Unmount the partitions
                cmd = "umount "+mntpoint;
                QProcess::execute(cmd);

                cmd = "rmdir "+mntpoint;
                QProcess::execute(cmd);
                i++;
            }
            entry["partsizes"] = partSizes;
            entry["backupsize"] = overall*1024; //Convert from kB to bytes -> Used to show read progress
            item->setData(Qt::UserRole,entry);


            bt->addImage(entry);
            if (!slidesFolder.isEmpty())
                slidesFolders.append(slidesFolder);
        }
    }

    backupSpaceMB /= 3; //conservative estimate gzip compression
    if (backupSpaceMB > _availableDownloadMB)
    {
        QString message = tr("This backup may require ")
                +QString::number(backupSpaceMB)
                +tr(" MB of backup space, but only ")
                +QString::number(_availableDownloadMB)
                +tr(" MB is available. This is only an estimate. If you continue, the backup may not complete successfully.\n\nDo you want to continue?");
        if (QMessageBox::warning(this, tr("WARNING: Backup Space"),message,QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            setEnabled(false);
            return;
        }
    }

    QMessageBox::information(this, tr("Backup Info"), tr("Always test your backups before relying on them"), QMessageBox::Ok);

    if (slidesFolders.isEmpty())
        slidesFolder.append("/mnt/defaults/slides");

    _qpssd = new ProgressSlideshowDialog(slidesFolders, "", 20, this); //_osdrive, ePM_READSTATS
    _qpssd->setWindowTitle("Backing Up Images");
    connect(bt, SIGNAL(parsedImagesize(qint64)), _qpssd, SLOT(setMaximum(qint64)));
    connect(bt, SIGNAL(completed(int)), this, SLOT(onCompleted(int)));
    connect(bt, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(bt, SIGNAL(statusUpdate(QString)), _qpssd, SLOT(setLabelText(QString)));
    connect(bt, SIGNAL(newDrive(const QString&,eProgressMode)), _qpssd , SLOT(setDriveMode(const QString&,eProgressMode)), Qt::BlockingQueuedConnection);
    connect(bt, SIGNAL(startAccounting()), _qpssd, SLOT(startAccounting()), Qt::BlockingQueuedConnection);
    connect(bt, SIGNAL(stopAccounting()), _qpssd , SLOT(stopAccounting()), Qt::BlockingQueuedConnection);
    connect(bt, SIGNAL(newImage( QString)), this, SLOT(newImage( QString)));
    connect(bt, SIGNAL(idle()), _qpssd , SLOT(idle()));
    connect(bt, SIGNAL(cont()), _qpssd , SLOT(cont()));
    connect(bt, SIGNAL(consolidate()), _qpssd , SLOT(consolidate()), Qt::BlockingQueuedConnection);
    connect(bt, SIGNAL(finish()), _qpssd , SLOT(finish()));

    bt->start();
    hide();
    _qpssd->exec();
    show();

    //QProcess::execute("mount -o remount,ro /mnt");
}

void MainWindow::hideDialogIfNoNetwork()
{

    //Maybe more OSes will be downloaded after wifi is connected - WJDK
    //So for now we assume it is done and allow silentinstall/update to continue.
    _processedImages |= ALLNETWORK;
    if (_qpd)
    {
        if (!isOnline())
        {
            /* No network cable inserted */
            _qpd->hide();
            _qpd->deleteLater();
            _qpd = NULL;

            if (ug->count() == 0)
            {
                /* No local images either */
                if (_hasWifi)
                {
                    QMessageBox::critical(this,
                                          tr("No network access"),
                                          tr("Network access is required to use PINN without local images. Please select your wifi network in the next screen."),
                                          QMessageBox::Close);
                    on_actionWifi_triggered();
                }
                else
                {
                    QMessageBox::critical(this,
                                          tr("No network access"),
                                          tr("Wired network access is required to use PINN without local images. Please insert a network cable into the network port."),
                                          QMessageBox::Close);
                }
            }
        }
    }
}

void MainWindow::on_actionWifi_triggered()
{

    bool wasAlreadyOnlineBefore = !_networkStatusPollTimer.isActive();

    WifiSettingsDialog wsd;
    if ( wsd.exec() == wsd.Accepted )
    {
        if (wasAlreadyOnlineBefore)
        {
            /* Try to redownload list. Could have failed through previous access point */
            downloadLists();
        }
    }
}

void MainWindow::pollForNewDisks()
{
    TRACE
    QString dirname = "/sys/class/block";
    QDir dir(dirname);
    QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (_infoDelay)
    {
        _infoDelay--;
        if (!_infoDelay && _info)
        {
            _info->hide();
            _info->deleteLater();
            _info=NULL;
        }
    }

    if (_selectImages || _selectOsList.count())
    {   //We asked for some autoselection when there is no OS installed, and at least one available
        qDebug() <<"Waiting for OSes..." << _waitforImages << " Got " <<_processedImages;
        if ( (_processedImages & _waitforImages) == _waitforImages)
        {   //All required sources have been processed (see _availableImages for those that are present)
            //if ( (_waitforImages & ALLNETWORK ==0) || _networkup)
            {
                qDebug() <<"Selecting OSes...";

                QList<QListWidgetItem *> all = ug->allItems();
                foreach (QListWidgetItem * witem, all)
                {
                    QVariantMap existing_details = witem->data(Qt::UserRole).toMap();

                    witem->setCheckState(Qt::Unchecked);

                    if ((existing_details["source"].toString()==SOURCE_SDCARD) && (_selectImages & ALLSD))
                    {
                        witem->setCheckState(Qt::Checked); //No option for SOURCE_USB, so we'll assume it's the same as SOURCE_SD
                        qDebug() <<"  " << existing_details["name"].toString() << "  " << "(allsd)";
                    }

                    if ((existing_details["source"].toString()==SOURCE_USB) && (_selectImages & ALLUSB))
                    {
                        witem->setCheckState(Qt::Checked);
                        qDebug() <<"  " << existing_details["name"].toString() << "  " << "(allusb)";
                    }

                    if ((existing_details["source"].toString()==SOURCE_NETWORK) && (_selectImages & ALLNETWORK))
                    {
                        witem->setCheckState(Qt::Checked);
                        qDebug() <<"  " << existing_details["name"].toString() << "  " << "(allnetwork)";
                    }

                    if ((existing_details["installed"].toBool()==true) && (_selectImages & ALLINSTALLED))
                    {
                        witem->setCheckState(Qt::Checked);
                        qDebug() <<"  " << existing_details["name"].toString() << "  " << "(allinstalled)";
                    }


                    foreach (QString osname, _selectOsList)
                    {
                        if (existing_details["name"].toString()== osname)
                        {
                            witem->setCheckState(Qt::Checked);
                            qDebug() <<"  " << existing_details["name"].toString();
                        }
                    }

                }

                //Also, Select any installed os names that match the `select` parameter
                qDebug() <<"Selecting installed OSes...";
                for (int i=0; i< ug->listInstalled->count(); i++)
                {
                    QListWidgetItem * witem = ug->listInstalled->item(i);
                    witem->setCheckState(Qt::Unchecked);
                    QVariantMap installed_os = witem->data(Qt::UserRole).toMap();
                    foreach (QString osname, _selectOsList)
                    {
                        qDebug()<<"Checking "+installed_os["name"].toString()+" vs "+osname;
                        if ( CORE(installed_os["name"].toString())== osname)
                        {
                            qDebug() << "found";
                            witem->setCheckState(Qt::Checked);
                        }
                    }
                }

                _selectImages=0; //Prevent re-entry
                _selectOsList.clear();

                //Check for silentinstall & install them
                if ((_allowSilent) && !_numInstalledOS &&  ug->count() >= 1)
                {   //silentInstall was selected, so let's auto-install them
                    _silent=true;
                    counter.stopCountdown();
                    on_actionWrite_image_to_disk_triggered();
                    //Following will be done in onCompleted()
                    //addInstalledImages();   //Update the installed lists
                    //updateInstalledStatus();
                }

                //Check for silentreinstallnewer option
                QString cmdline = getFileContents("/proc/cmdline");
                int nReinstalls=0;
                if (cmdline.contains("silentreinstallnewer"))
                {   //Restrict the items to those that have newer versions
                    QList<QListWidgetItem *> select = ug->selectedInstalledItems();
                    nReinstalls = select.count();

                    foreach (QListWidgetItem * witem, select)
                    {
                        QVariantMap selected_os = witem->data(Qt::UserRole).toMap();
                        QString installedName = CORE(selected_os["name"].toString());
                        qDebug() <<"Searching for "+installedName;
                        QListWidgetItem * matchItem = ug->findItemByDataName(installedName);
                        if (matchItem)
                        {
                            qDebug() << "found";
                            QVariantMap matchEntry = matchItem->data(Qt::UserRole).toMap();
                            if (selected_os["release_date"].toString() >= matchEntry["release_date"].toString() )
                            {
                                witem->setCheckState(Qt::Unchecked);
                                nReinstalls--;
                                qDebug() <<"Deselecting " << selected_os["name"].toString();
                            }
                            else
                                qDebug() << " X " << installedName;
                        }
                        else
                        {
                            witem->setCheckState(Qt::Unchecked);
                            nReinstalls--;
                            qDebug() <<"No replacement for " << selected_os["name"].toString();
                        }
                    }

                    if (nReinstalls)
                    {
                        _silent=true;
                        qDebug() <<"Silently re-installing updates";

                        //on_actionReinstall_triggered();
                        _eDownloadMode = MODE_REINSTALLNEWER;
                        counter.stopCountdown();
                        doReinstall();

                        //Following will be done in onCompleted()
                        //addInstalledImages();   //Update the installed lists
                        //updateInstalledStatus();
                    }
                    else
                        qDebug() <<"No new updates";
                }
            }
        }
    }

    if (list.count() != _devlistcount)
    {
        foreach (QString devname, list)
        {
            QString blocklink = QFile::symLinkTarget(dirname+"/"+devname);
            /* skip virtual things and partitions */
            if (blocklink.contains("/devices/virtual/") || QFile::exists(blocklink+"/partition") )
                continue;

            QByteArray inflight = getFileContents(sysclassblock(devname)+"/inflight").trimmed();
            if (inflight.count() && inflight.left(1) != "0")
            {
                /* There are currently outstanding IO request, which could mean it hasn't finished
                 * reading the partition table. Check again on next round */
                return;
            }

            /* does the drive perhaps have a FAT partition with extra images? */
            if ("/dev/"+devname != _bootdrive && QFile::exists(sysclassblock(devname, 1)))
            {
                QString p1 = partdev(devname, 1);

                if (!QFile::exists("/dev/"+p1))
                {
                    /* /dev node not created yet. Check again on next round */
                    return;
                }

                if (_usbimages && !QFile::exists("/tmp/media/"+p1))
                {
                    addImagesFromUSB(p1); //eg 'sda1'
                }
            }

            /* is the drive writable? */
            if (getFileContents(blocklink+"/ro").trimmed() == "1")
                continue;

            QString model = getFileContents(dirname+"/"+devname+"/device/model").trimmed();
            if (model.isEmpty())
                model = getFileContents(dirname+"/"+devname+"/device/name").trimmed();
            QIcon icon;

            if (devname.startsWith("mmc"))
            {
                icon = QIcon(":/icons/hdd.png");
            }
            else
            {
                icon = QIcon(":/icons/hdd_usb_unmount.png");
            }

            if (ui->targetCombo->findData(devname) == -1)
            {
                ui->targetCombo->addItem(icon, devname+": "+model, devname);

                /* does the partition structure look like a preloaded Pi drive, then select it by default? */
                if (devname == "sda"
                        && QFile::exists(sysclassblock(devname, 1))
                        && QFile::exists(sysclassblock(devname, 5))
                        && getFileContents(sysclassblock(devname, 5)+"/size").trimmed().toInt() == SETTINGS_PARTITION_SIZE)
                {
                    ui->targetCombo->setCurrentIndex(ui->targetCombo->count()-1);
                }
            }

            if ((ui->targetComboUsb->findData(devname) == -1) && (!devname.startsWith("mmc")))
            {
                 /* does the partition structure look like it contains OSes, then select it by default? */
                if (LooksLikeOSDrive(devname))
                {
                    ui->targetComboUsb->addItem(icon, devname+": "+model, devname);
                    ui->targetComboUsb->setCurrentIndex(ui->targetComboUsb->count()-1);
                }
            }
        }

        if (ui->targetCombo->count() > 1)
        {
            ui->targetLabel->setHidden(false);
            ui->targetCombo->setHidden(false);
        }

        _devlistcount = list.count();
    }

}

bool MainWindow::LooksLikePiDrive(QString devname)
{

    /* Return TRUE if the drive partition structure looks like it has been PINN formatted */
    return( QFile::exists(sysclassblock(devname, 1))
            && QFile::exists(sysclassblock(devname, 5))
            && getFileContents(sysclassblock(devname, 5)+"/size").trimmed().toInt() == SETTINGS_PARTITION_SIZE );
}

bool MainWindow::LooksLikeOSDrive(QString devname)
{

    //@@ maybe mount and check for /os folder present?
    if( devname != "mmcblk0" && !LooksLikePiDrive(devname) )
    {
        return (QFile::exists("/tmp/media/"+partdev(devname,1)+"/os") );
    }
    return (false);
}

void MainWindow::recalcAvailableMB()
{
    int errorcode=0;

    _availableMB = (getFileContents(sysclassblock(_drive)+"/size").trimmed().toULongLong()-getFileContents(sysclassblock(_drive, 5)+"/start").trimmed().toULongLong()-getFileContents(sysclassblock(_drive, 5)+"/size").trimmed().toULongLong())/2048 - _provision;

//    QString cmd = "sh -c \"fdisk -l " + _drive + " | grep Extended | sed -e 's/ \\+/ /g' | cut -d ' ' -f 4\"";
//    QString result = readexec(0,cmd,errorcode);
//    _availableMB = (result.toULongLong() - getFileContents(sysclassblock(_drive, 5)+"/size").trimmed().toULongLong())/ 2048;

    if (_osdrive!="")
    {
        QString classdev = sysclassblock(_osdrive, 1);
        if (QFile::exists(classdev))
        {
            QProcess proc;
            QString cmd = "sh -c \"df -m /dev/" + partdev(_osdrive,1) + " | grep  /dev/" + partdev(_osdrive,1) + " | sed 's| \\+| |g' | cut -d' ' -f 4 \"";

            proc.start(cmd);
            proc.waitForFinished();
            QString result = proc.readAll();

            _availableDownloadMB = result.toInt();
        }
        else
        {
            _availableDownloadMB = 0;
        }
    }
    else
    {
        _availableDownloadMB = 0;
    }
}


void MainWindow::on_targetCombo_currentIndexChanged(int index)
{
    if (index != -1)
    {
        QString devname = ui->targetCombo->itemData(index).toString();

        if (devname != "mmcblk0" && (
                   !QFile::exists(sysclassblock(devname, 1))
                || !QFile::exists(sysclassblock(devname, 5))
                || getFileContents(sysclassblock(devname, 5)+"/size").trimmed().toInt() != SETTINGS_PARTITION_SIZE))
        {
            if ( !_skipformat && QMessageBox::question(this,
                                      tr("Reformat drive?"),
                                      tr("Are you sure you want to reformat the drive '%1' for use with PINN? All existing data on the drive will be deleted!").arg(devname),
                                      QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            {
                InitDriveThread idt("/dev/"+devname);
                idt.formatUsbDrive();
            }
            else
            {
                int idx = ui->targetCombo->findData("mmcblk0");
                if (idx != -1 && idx != index)
                    ui->targetCombo->setCurrentIndex(idx);
                return;
            }
        }

        qDebug() << "New drive selected:" << devname;
        _drive = "/dev/"+devname;
        recalcAvailableMB();
        filterList();
        updateNeeded();
    }
}

void MainWindow::on_targetComboUsb_currentIndexChanged(int index)
{

    if (index != -1)
    {
        QString devname = ui->targetComboUsb->itemData(index).toString();
        _osdrive=devname;
         qDebug() << "New Download drive selected:" << devname;
        recalcAvailableMB();
        filterList();
        updateNeeded();
    }
}

/* Add an image as an QListWidgetItem to the list widgets that hold the new installable OSes */
void MainWindow::addImage(QVariantMap& m, QIcon &icon, bool &bInstalled)
{
    TRACE
    OverrideJson(m);
    QString name = m.value("name").toString();
    QString folder  = m.value("folder").toString();
    QString description = m.value("description").toString();
    QString version = m.value("version").toString();
    bool recommended = m.value("recommended").toBool();
    DBG(name);
    QListWidgetItem *witem = NULL;
    QString tooltip;

    tooltip="";

    //If it is already installed, we don't care that it WAS installed from a backup, so remove date.
    if (bInstalled)
        name =NICKNAME(name);
        //name = getNameParts(name, eCORE);

    witem = findItemByName(name);
    DBG("...");
    if ((witem) && (!bInstalled))
    {
        DBG("witem && Not installed");
        QVariantMap existing_details = witem->data(Qt::UserRole).toMap();

        bool bReplace=false;
        if (existing_details["source"].toString()==SOURCE_INSTALLED_OS)
        {
            bReplace=true;
        }
        if (existing_details["release_date"].toString()  < m["release_date"].toString())
        {
            bReplace=true;
        }
        if (existing_details["release_date"].toString() == m["release_date"].toString())
        {   //Same date
            if (m["source"].toString() == SOURCE_SDCARD)
            {   //Prefer to use local rather than remote image
                bReplace=true;
            }
            if (m["source"].toString() == SOURCE_USB)
            {   //Prefer to use local rather than remote image
                bReplace=true;
            }
            if (existing_details["source"].toString()==SOURCE_INSTALLED_OS)
            {
                bReplace=true;
            }
        }
        if (bReplace)
        {
            DBG("replace");
            /* Existing item in list is same version or older. Prefer image on USB storage. */
            /* Copy current installed state */

            m.insert("installed", existing_details.value("installed", false));

            if (existing_details.contains("partitions"))
            {
                m["partitions"] = existing_details["partitions"];
            }
            QString friendlyname = name;
            if (recommended)
                friendlyname += " ["+tr("RECOMMENDED")+"]";
            if (!description.isEmpty())
                friendlyname += "\n"+description;
            witem->setText(friendlyname);

            //Set Tooltip
            if (m.contains("release_date"))
            {
                tooltip += m.value("release_date","").toString();
            }
            if (m.contains("version"))
            {
                if (!tooltip.isEmpty())
                    tooltip += ", ";
                tooltip += m.value("version","").toString();
            }
            witem->setToolTip(tooltip);

            witem->setData(Qt::UserRole, m);
            witem->setData(SecondIconRole, icon);
            ug->list->update();
        }
        else
        {
            DBG("Ignore");
        }
    }
    else //not found or bInstalled
    {
        //QListWidgetItem *witemNew = NULL;
        //witemNew = witem;

        DBG("New OS");
        /* It's a new OS, so add it to the list */
        QString iconFilename = m.value("icon").toString();

        // Icon maybe a remote URL
        if (!iconFilename.isEmpty() && !iconFilename.contains('/'))
            iconFilename = folder+"/"+iconFilename;
        if (!QFile::exists(iconFilename))
        {
            iconFilename = folder+"/"+CORE(name)+".png";
            iconFilename.replace(' ', '_');
        }
        QString friendlyname = name;
        if (recommended)
            friendlyname += " ["+tr("RECOMMENDED")+"]";
        if (!description.isEmpty())
            friendlyname += "\n"+description;

        witem = new QListWidgetItem(friendlyname);
        witem->setCheckState(Qt::Unchecked);
        witem->setData(Qt::UserRole, m);

        //Set Tooltip
        if (m.contains("release_date"))
        {
            tooltip += m.value("release_date","").toString();
        }
        if (m.contains("version"))
        {
            if (!tooltip.isEmpty())
                tooltip += ", ";
            tooltip += m.value("version","").toString();
        }
        witem->setToolTip(tooltip);

        witem->setData(SecondIconRole, icon);

        if (QFile::exists(iconFilename))
        {
            QIcon iconos;
            iconos = QIcon(iconFilename);
            QList<QSize> avs = iconos.availableSizes();
            if (avs.isEmpty())
            {
                /* Icon file corrupt */
                iconos = QIcon();
            }
            witem->setIcon(iconos);
        }
        if (bInstalled)
        {
            //Add image to installed list
            if (recommended)
                ug->listInstalled->insertItem(1, witem); //After PINN entry
            else
                ug->listInstalled->addItem(witem);
            //ug->listInstalled->update();
#if 0
            //Clone image to new list if not already known
            if (!witemNew)

            {
                //Clone to normal list
                QListWidgetItem *witemNew = witem->clone();
                witemNew->setCheckState(Qt::Unchecked);
                if (recommended)
                    ug->insertItem(0, witemNew);
                else
                    ug->addItem(witemNew);
                //ug->list->update();
            }
#endif
        }
        else
        {
            //Add new OS to os list.
            DBG("Adding to list");
            if (recommended)
                ug->insertItem(0, witem);
            else
                ug->addItem(witem);
            //ug->list->update();
        }
    }
    DBG("done");
    ug->showTab(DEFGROUP);
    QApplication::processEvents();
}

void MainWindow::newImage(QString Imagefile)
{

    QVariantMap entry = Json::loadFromFile(Imagefile).toMap();

    entry["source"] = SOURCE_USB;
    QString folder = QFileInfo(Imagefile).path();
    entry["folder"] = folder;

    if (!entry.contains("nominal_size"))
    {
        /* Calculate nominal_size based on information inside partitions.json */
        int nominal_size = 0;
        QVariantMap pv = Json::loadFromFile(folder+"/partitions.json").toMap();
        QVariantList pvl = pv.value("partitions").toList();

        foreach (QVariant v, pvl)
        {
            QVariantMap pv = v.toMap();
            nominal_size += pv.value("partition_size_nominal").toInt();
            nominal_size += 1; /* Overhead per partition for EBR */
        }

        entry["nominal_size"]= nominal_size;
    }

    bool bInstalled=false;
    QIcon usbIcon(":/icons/hdd_usb_unmount.png");
    //qDebug() << entry;
    addImage(entry,usbIcon,bInstalled);
}

void MainWindow::addImagesFromUSB(const QString &device)
{

    QDir dir;
    QString mntpath = "/tmp/media/"+device;

    dir.mkpath(mntpath);
    QProcess::execute("umount /dev/"+device);
    if (QProcess::execute("mount -o ro /dev/"+device+" "+mntpath) != 0)
    {
        dir.rmdir(mntpath);
        return;
    }

    if (!QFile::exists(mntpath+"/os"))
    {
        QProcess::execute("umount "+mntpath);
        dir.rmdir(mntpath);
        return;
    }

    QIcon usbIcon(":/icons/hdd_usb_unmount.png");
    QMap<QString,QVariantMap> images = listImages(mntpath+"/os");

    foreach (QVariant v, images.values())
    {
        QVariantMap m = v.toMap();
        bool bInstalled=false;
        addImage(m,usbIcon,bInstalled);
    }
    updateInstalledStatus();
    filterList();
    ug->showTab(DEFGROUP);
    ug->setDefaultItems();
    ug->setFocus();
    _availableImages |= ALLUSB;
    _processedImages |= ALLUSB;
}


/* Dynamically hide items from list depending on target drive */
void MainWindow::filterList()
{

    QList<QListWidgetItem *> all;
    all = ug->allItems();
    for (int i=0; i < ug->count(); i++)
    {
        QListWidgetItem *witem = all.value(i);

        if (_drive == "/dev/mmcblk0")
        {
            witem->setHidden(false);
        }
        else
        {   //Installing to something other than SD card
            QVariantMap m = witem->data(Qt::UserRole).toMap();
            bool supportsUsb;
            QString param;

            if (_drive.contains("/dev/nvme"))
            {
                if (_bootdrive == "/dev/mmcblk0")
                    param = "supports_nvme_root";
                else
                    param = "supports_nvme_boot";
            }
            else {
                if (_bootdrive == "/dev/mmcblk0")
                    param = "supports_usb_root";
                else
                    param = "supports_usb_boot";
            }

            /* If the repo explicity states whether or not usbS is supported use that info */
            if (m.contains(param))
            {
                supportsUsb = m.value(param).toBool();
            }
            else
            {
                /* Otherwise just assume Linux does, and RiscOS and Windows do not */
                QString name = m.value("name").toString();
                supportsUsb = (!nameMatchesRiscOS(name) && !nameMatchesWindows(name));
            }

            if (supportsUsb)
            {
                witem->setHidden(false);
            }
            else
            {
                witem->setCheckState(Qt::Unchecked);
                witem->setHidden(true);
            }
        }
    }
}

void MainWindow::on_actionClone_triggered()
{

    char buffer[256];
    QString src;
    QString dst;
    QString src_dev;
    QString dst_dev;
    piclonedialog pDlg;
    bool resize;
    int result = pDlg.exec();

    if (result==QDialog::Rejected)
        return;

    src=pDlg.get_src();
    dst=pDlg.get_dst();
    src_dev=pDlg.get_src_dev();
    dst_dev=pDlg.get_dst_dev();
    resize = pDlg.get_resize();
    if (src_dev == dst_dev)
        return;

    sprintf (buffer, tr("This will erase all content on the device '%s'. Are you sure?").toUtf8().constData(), dst.toUtf8().constData());

    QMessageBox msgBox(QMessageBox::Warning, tr("Clone SD Card"),
                       buffer, 0, this);
    msgBox.addButton(tr("Yes"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("No"), QMessageBox::RejectRole);
    if (msgBox.exec() == QMessageBox::AcceptRole)
    {
        msgBox.close();
        piCloneThread *cloneThread = new piCloneThread(src_dev, dst_dev, resize);
        QStringList DirList;
        setEnabled(false);
        //Reuse the existing Progress Slide Dialog
        _qpd = new ProgressSlideshowDialog(DirList, "", 20);//Add dst_dev
        _qpd->setWindowTitle("Clone SD Card");
        // ((ProgressSlideshowDialog*)_qpd)->disableIOaccounting();
        connect(cloneThread, SIGNAL(setMaxProgress(qint64)), _qpd, SLOT(setMaximum(qint64)));
        connect(cloneThread, SIGNAL(completed()), this, SLOT(onCloneCompleted()));
        connect(cloneThread, SIGNAL(error(QString)), this, SLOT(onCloneError(QString)));
        connect(cloneThread, SIGNAL(statusUpdate(QString)), _qpd, SLOT(setLabelText(QString)));
        connect(cloneThread, SIGNAL(secondaryUpdate(QString)), _qpd, SLOT(setMBWrittenText(QString)));
        connect(cloneThread, SIGNAL(setProgress(qint64)), _qpd, SLOT(updateProgress(qint64)));
        cloneThread->start();
        _qpd->exec();
        setEnabled(true);
    }
}

void MainWindow::onCloneCompleted()
{
    _qpd->hide();

    QMessageBox::information(this,
                             tr("Clone Completed"),
                             tr("Clone Completed Successfully"), QMessageBox::Ok);
    _qpd->deleteLater();
    _qpd = NULL;
}

void MainWindow::onCloneError(const QString &msg)
{

    qDebug() << "Error:" << msg;
    if (_qpd)
    {
        _qpd->hide();
    }
    _qpd->deleteLater();
    _qpd = NULL;
    QMessageBox::critical(this, tr("Error"), msg, QMessageBox::Close);

    //Anything could have happened, so umount all then mount what we need
    QProcess::execute("sh -c \"umount /tmp/src");
    QProcess::execute("sh -c \"umount /tmp/dst");
    QProcess::execute("sh -c \"umount /dev/mmcblk0p1 /mnt\"");
    QProcess::execute("rmdir "+QString("/tmp/src"));
    QProcess::execute("rmdir "+QString("/tmp/dst"));
    QProcess::execute("sh -c \"mount -o ro /dev/mmcblk0p1 /mnt\"");

    setEnabled(true);
}

void MainWindow::on_actionPassword_triggered()
{

    /* If no installed OS is selected, default to first extended partition */
    QListWidgetItem *item = ug->listInstalled->currentItem();
    QVariantMap m;

    if (item)
    {
        m = item->data(Qt::UserRole).toMap();
        if (m.contains("partitions"))
        {
            //QVariantList l = item->data(Qt::UserRole).toMap().value("partitions").toList();
            Passwd pDlg(m);
            pDlg.exec();
        }
    }
}

void MainWindow::checkForUpdates(enum UpdateMode mode)
{

    _updateMode = mode;
    _numBuildsToDownload=0;
    downloadUpdate(BUILD_URL,  "BUILD|" BUILD_NEW);
    downloadUpdate(README_URL, "README|" README_NEW);
    downloadUpdate(GROUP_URL,  "GROUP|" GROUP_NEW);
}

void MainWindow::downloadUpdate(const QString &urlstring, const QString &saveAs)
{
    //NOTE: saveAs=type|filename
    _numBuildsToDownload++;
    qDebug() << "Downloading" << urlstring << "to" << saveAs;
    QUrl url(urlstring);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", AGENT);
    request.setAttribute(QNetworkRequest::User, saveAs);
    QNetworkReply *reply = _netaccess->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadUpdateRedirectCheck()));
}

void MainWindow::downloadUpdateRedirectCheck()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString redirectionurl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QString saveAs = reply->request().attribute(QNetworkRequest::User).toString();
    //NOTE: saveAs=type|filename

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        _numBuildsToDownload--;
        downloadUpdate(redirectionurl, saveAs);
    }
    else
    {
        downloadUpdateComplete();
    }
}

void MainWindow::downloadUpdateComplete()
{
    int error=0;

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString userInfo = reply->request().attribute(QNetworkRequest::User).toString();
    //NOTE: userInfo=type|filename
    QStringList userInfoList = userInfo.split("|");

    QString saveAs = userInfoList.at(0);
    QString type="";
    if (userInfoList.count()>1)
    {
        type = saveAs;
        saveAs = userInfoList.at(1);
    }

    qDebug() << type;

    if (reply->error() != reply->NoError || httpstatuscode < 200 || httpstatuscode > 399)
    {
        if (type == "UPDATE")
        {   // We only care if the user initiated upgrade fails. Others are non-fatal.
            QMessageBox::critical(this, tr("Download error"), tr("Error downloading update file")+"\n"+reply->url().toString(), QMessageBox::Close);
        }
        else
        {
            _numBuildsToDownload--;
            if ((_updateMode == EUPDATEMANUAL) && (type =="BUILD"))
            {
                QMessageBox::information(this, tr("PINN Update Check"), tr("Error contacting update server"), QMessageBox::Close);
            }
        }
        setEnabled(true);
        return;
    }

    //Successful download

    QFile f(saveAs);
    f.open(f.WriteOnly);
    if (f.write(reply->readAll()) == -1)
    {
        if (type == "UPDATE")
        {   // We only care if the user initiated upgrade fails. Others are non-fatal.
            QMessageBox::critical(this, tr("Download error"), tr("Error writing downloaded file to SD card. SD card or file system may be damaged."), QMessageBox::Close);
        }
        else
        {
            _numBuildsToDownload--;
        }
    }
    else
    {
        qDebug() << "Downloaded " << type << ":" << saveAs;
        _numBuildsToDownload--;
        //?
    }
    f.close();

    setEnabled(true);

    if (type=="UPDATE") //upgrade
    {
        qDebug() << "Time to update PINN!";

        if (_qpdup)
        {
            ((QProgressDialog*)_qpdup)->setLabel( new QLabel(tr("PINN will now update and reboot in a few secs...")));
            QApplication::processEvents();
            SleepSimulator s;
            s.sleep(1000);
        }

        error = updatePinn();

        QProcess::execute(QString("rm ")+BUILD_IGNORE);
        QProcess::execute("sync");

        if (_qpdup)
        {
            _qpdup->hide();
            _qpdup->deleteLater();
            _qpdup = NULL;
        }

        if (error)
        {
            if (_updateMode == EUPDATEMANUAL)
                QMessageBox::critical(this, tr("PINN update failed"), tr(""), QMessageBox::Close);
            return;
        }
        else
        {
            //Reboot back into PINN
            QByteArray partition("1");
            setRebootPartition(partition);

            // Reboot
            reboot();
        }
    }
    else if (type=="GROUP") //update categories
    {
        QByteArray r = getFileContents(GROUP_NEW);
        if (r.length()>1 && r.at(0) == (int)'{')
        {   //Valid file (minimal test!)
            QString cmd("/usr/bin/diff " GROUP_NEW " " GROUP_CURRENT);
            if (QProcess::execute(cmd))
            {
                qDebug() << "Updating overrides.json";
                QProcess::execute("mount -o remount,rw /mnt");
                QProcess::execute("cp " GROUP_NEW " " GROUP_CURRENT);
                QProcess::execute("mount -o remount,ro /mnt");
                QProcess::execute("sync");
            }
        }
    }

    if (_numBuildsToDownload==0)
    {
        BuildData currentver, newver, ignorever;

        if (_updateMode == EUPDATEMANUAL)
        {   //If user manually asks for an update check, then remove the ignore file.
            QProcess::execute(QString("rm ")+BUILD_IGNORE);
            QProcess::execute("sync");
        }

        qDebug()<<"BUILD_IGNORE...";
        ignorever.read(BUILD_IGNORE);
        qDebug()<<"BUILD_CURRENT...";
        currentver.read(BUILD_CURRENT);
        if (!ignorever.isEmpty())
        {
            if (currentver > ignorever)
                QProcess::execute(QString("rm ")+BUILD_IGNORE);
            if (currentver < ignorever)
                currentver = ignorever;
        }
        qDebug()<<"BUILD_NEW...";
        newver.read(BUILD_NEW);

        if (newver > currentver)
        {
            on_newVersion();
        }
        else if (_updateMode==EUPDATEMANUAL)
        {
            QMessageBox::information(this, tr("PINN Update Check"), tr("No updates available"), QMessageBox::Close);
        }
    }
}

int MainWindow::updatePinn()
{
    int error=0;
    int dummy;
    //When PINN is updated, We don't need these files to be extracted
    QString exclusions = " -x cmdline.txt -x preupdate -x postupdate -x exclude.txt";

    readexec(1,"mount -o remount,rw /mnt",dummy);

    //First we'll extract these 2 files to /tmp to automate the update process
    readexec(1,"unzip /tmp/pinn-lite.zip -o exclude.txt preupdate postupdate -d /tmp",dummy);


    //In case we need to do some additional pre-update processing
    if (QFile::exists("/tmp/preupdate"))
    {
        if (_qpdup)
            ((QProgressDialog*)_qpdup)->setLabel( new QLabel(tr("Executing preupdate")));
        QApplication::processEvents();

        QProcess::execute("chmod +x /tmp/preupdate");
        readexec(1,"/tmp/preupdate",error);
    }
    if (!error)
    {

        if (QFile::exists("/tmp/exclude.txt"))
        {
            //exclude.txt can avoid extracting addtional files to RECOVERY that we don't want updating
            QString contents = getFileContents("/tmp/exclude.txt");
            QStringList fileList = contents.split("\n");
            foreach (QString file, fileList)
            {
                //Add each filename to the list of file extraction exclusios
                file=file.trimmed();
                if (file.length()>0)
                    exclusions += " -x "+file;
            }
        }
        //Extract all the files to Recovery, except our excluded set
        if (_qpdup)
            ((QProgressDialog*)_qpdup)->setLabel( new QLabel(tr("Extracting update")));
        QApplication::processEvents();

        QString cmd = "unzip /tmp/pinn-lite.zip -o" + exclusions + " -d /mnt";
        readexec(1,cmd,dummy);
    }

    //In case we need to do some additional upgrade processing
    if (QFile::exists("/tmp/postupdate"))
    {
        if (_qpdup)
            ((QProgressDialog*)_qpdup)->setLabel( new QLabel(tr("Executing postupdate")));
        QApplication::processEvents();

        QProcess::execute("chmod +x /tmp/postupdate");

        QStringList args("/tmp/postupdate");
        args << QString::number(error);

        QString cmd = "/bin/sh";
        QProcess::execute(cmd,args);
    }

    if (error)
    {
        qDebug() << "update failed.";
    }

    QProcess::execute("mount -o remount,ro /mnt");
    return(error);
}

void MainWindow::on_newVersion()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("PINN UPDATE"));
    msgBox.setText(tr("A new version of PINN is available"));
    msgBox.setInformativeText(tr("Do you want to download this version?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Ignore);
    msgBox.setDefaultButton(QMessageBox::No);

    QFile f(README_NEW);
    QString history;

    if (f.exists())
    {
        if (f.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&f);
            history = in.readAll();
            msgBox.setDetailedText(history);
            f.close();
        }
    }

    msgBox.installEventFilter(&counter);

    QString cmdline = getFileContents("/proc/cmdline");
    int ret;
    if (cmdline.contains("forceupdatepinn"))
        ret=QMessageBox::Yes;
    else
        ret = msgBox.exec();

    switch (ret)
    {
        case QMessageBox::Yes:
            // Yes was clicked
            setEnabled(false);
            _qpdup = new QProgressDialog( QString(tr("Downloading Update")), QString(), 0, 0, this);
            _qpdup->setWindowModality(Qt::WindowModal);
            _qpdup->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            _qpdup->setWindowTitle("Updating PINN");
            _qpdup->show();
            downloadUpdate(UPDATE_URL,  "UPDATE|" UPDATE_NEW);
            break;
        case QMessageBox::No:
            // No was clicked
            break;
        case QMessageBox::Ignore:
            // Ignore was clicked
            QString cmd = "cp ";
            cmd.append(BUILD_NEW);
            cmd.append(" ");
            cmd.append(BUILD_IGNORE);
            QProcess::execute(cmd);
            break;
    }
}

/* Key on TV remote pressed */
void MainWindow::onKeyPress(int cec_code, int value)
{
    //qDebug() << "Processing CEC " << cec_code << ", " << value;
    cec->process_cec(cec_code,value);
}

#if 1
/* joystick pressed */
void MainWindow::onJoyPress(int joy_code, int value)
{
    //
    //qDebug() << "Processing Joy "<<joy_code <<", " << value;
    joystick* joy = (joystick*) sender(); //joy1 or joy2
    joy->process_joy(joy_code,value);
}
#endif

void MainWindow::onJoyEvent(int type, int number, int value)
{
    qDebug() << "Joy type: " <<type<< " No: "<<number<<" Value: " << value;
}

void MainWindow::onJoyDebug(QString dbgmsg)
{
    qDebug() << dbgmsg;
}


void MainWindow::on_actionInfo_triggered()
{

    if (!requireNetwork())
        return;

    QListWidgetItem * item = NULL;
    item = ug->list->currentItem();
    if (!item)
    {
        return;
    }
    QVariantMap m = item->data(Qt::UserRole).toMap();
    if (m.contains("url"))
    {
        if (_proc)
        {
            _proc->kill();
            _proc=NULL;
        }
        _proc = new QProcess(this);
        QString lang = LanguageDialog::instance("en", "gb")->currentLanguage();
        if (lang == "gb" || lang == "us" || lang == "ko" || lang == "")
            lang = "en";
        _proc->start("arora -lang "+lang+" "+m.value("url").toString());
    }
}

void MainWindow::on_actionInfoInstalled_triggered()
{


    if (!requireNetwork())
        return;

    QListWidgetItem * item = NULL;
    item = ug->listInstalled->currentItem();
    if (!item)
    {
        return;
    }
    QVariantMap m = item->data(Qt::UserRole).toMap();
    if (m.contains("url"))
    {
        if (_proc)
        {
            _proc->kill();
            _proc=NULL;
        }

        _proc = new QProcess(this);
        QString lang = LanguageDialog::instance("en", "gb")->currentLanguage();
        if (lang == "gb" || lang == "us" || lang == "ko" || lang == "")
            lang = "en";
        _proc->start("arora -lang "+lang+" "+m.value("url").toString());
    }
}

void MainWindow::on_actionReplace_triggered()
{

    _eDownloadMode = MODE_REPLACE;

    QList<QListWidgetItem *> replacementList;
    QList<QListWidgetItem *> installedList;

    bool bPartuuids=true;
    QString nonpartuuids;

    replacementList = ug->selectedItems();
    installedList   = ug->selectedInstalledItems();

    QList<QListWidgetItem*>::iterator it = installedList.begin();
    while (it != installedList.end())
    {
        QVariantMap installedMap = (*it)->data(Qt::UserRole).toMap();
        QString name = CORE(installedMap.value("name").toString());
        //Ignore PINN if it is selected
        if (name =="PINN")
        {
            it = installedList.erase(it);
        }
        //Ignore RISC OS or Windows if it is selected
        else if (nameMatchesRiscOS(name) || nameMatchesWindows(name))
        {
            it = installedList.erase(it);
        }
        else
        {
            ++it;
        }
    }

    it = replacementList.begin();
    while (it != replacementList.end())
    {
        QVariantMap replacementMap = (*it)->data(Qt::UserRole).toMap();
        QString name = CORE(replacementMap.value("name").toString());
        //Ignore RISC OS if it is selected
        if (nameMatchesRiscOS(name) || nameMatchesWindows(name))
            it = replacementList.erase(it);
        else
            ++it;

        if ((replacementMap.value("use_partuuid")==false) && ((_bootdrive!="/dev/mmcblk0") || (_drive!="/dev/mmcblk0")))
        {
            nonpartuuids += "\n" + name;
            bPartuuids = false;
        }
    }

    if (!replacementList.count() || !installedList.count())
    {
        QMessageBox::warning(this,
                             tr("Replace OSes"),
                             tr("Error: No OSes selected\n(Do not choose RISC OS or PINN)"),
                             QMessageBox::Close);
        return;
    }

    if (bPartuuids == false)
    {
        if ( !_silent && QMessageBox::warning(this,
                                    tr("Confirm"),
                                    tr("Warning: Partial USB support. The following OSes can only be executed from USB when it is /dev/sda and may fail to boot or function correctly if that is not the case:\n") + nonpartuuids + tr("\n\nDo you want to continue?"),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
            return;
    }


    replace dlg(replacementList,installedList,this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    _newList.clear();
    _newList = dlg.getMappedList();

    if (_newList.count())
    {
        foreach (QVariantMap os, _newList)
        {
            if (os.value("source").toString() == SOURCE_INSTALLED_OS)
            {
                onError(os.value("name").toString() + tr(" is not available.\nPlease provide it locally or connect to the internet."));
                return;
            }
        }
        prepareMetaFiles();
    }
    else
    {
        QMessageBox::warning(this,
                             tr("Replace OSes"),
                             tr("Warning: No OSes selected\n"),
                             QMessageBox::Close);

    }
}


void MainWindow::on_actionFschk_triggered()
{

    QListWidgetItem *item = ug->listInstalled->currentItem();
    if (ug->listInstalled->count() && item)
    {
        fscheck dlg(ug->listInstalled);
        dlg.exec();
    }
}

void MainWindow::on_actionRepair_triggered()
{

    QListWidgetItem *item = ug->listInstalled->currentItem();
    if (ug->listInstalled->count() && item)
    {
        repair dlg(ug->listInstalled, this, _drive);
        dlg.exec();
    }
}

void MainWindow::on_actionBackup_triggered()
{

    _eDownloadMode = MODE_BACKUP;

    _local = "/tmp/media/"+partdev(_osdrive,1);

    //if (QProcess::execute("mount -o remount,rw /dev/"+partdev(_osdrive,1)+" "+_local) != 0)
    //{
    //    return;
    //}
    // The NTFS driver can't remount, so we'll just umount & mount again
    QProcess::execute("umount /dev/"+partdev(_osdrive,1));
    QProcess::execute("mount  /dev/"+partdev(_osdrive,1)+" "+_local);

    if (_silent || QMessageBox::warning(this,
                                        tr("Confirm"),
                                        tr("Warning: this will backup the selected Operating System(s)."),
                                        QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)
    {
        /* See if any of the OSes are unsupported */
        bool allSupported = true;
        QString unsupportedOses;
        QList<QListWidgetItem *> selected = ug->selectedInstalledItems();

        // Check for OSes we can't back up
        foreach (QListWidgetItem *item, selected)
        {
            QVariantMap entry = item->data(Qt::UserRole).toMap();
            QString name = CORE(entry.value("name").toString());
            if (nameMatchesRiscOS(name) || nameMatchesWindows(name) || name.contains("XBian", Qt::CaseInsensitive))
            {
                allSupported = false;
                unsupportedOses += "\n" + name;
                item->setCheckState(Qt::Unchecked); //Deselect the unsupported OSes
            }
            if (entry.value("supports_backup",false).toString()=="update")
            {
                QString name = CORE(entry.value("name").toString());
                QListWidgetItem *witem = findItemByName(name);

                if (!updatePartitionScript(entry,witem))
                {
                    allSupported = false;
                    unsupportedOses += "\n" + name + " (UPDATE Reqd)";
                    item->setCheckState(Qt::Unchecked); //Deselect the unsupported OSes
                }
            }

            if (entry.value("supports_backup",false).toBool()==false)
            {
                allSupported = false;
                unsupportedOses += "\n" + name;
                item->setCheckState(Qt::Unchecked); //Deselect the unsupported OSes
            }
        }

        selected = ug->selectedInstalledItems();    //Select again without the unsupported OSes.

        if (_silent || allSupported || QMessageBox::warning(this,
                                        tr("Confirm"),
                                        tr("Warning: Unsupported Operating System(s) detected. PINN currently cannot backup the following OSes correctly:") + unsupportedOses,
                                        QMessageBox::Ok) == QMessageBox::Yes) //Can never be yes
        {
            setEnabled(false);
            _numMetaFilesToDownload = 0;
            foreach (QListWidgetItem *item, selected)
            {
                QVariantMap entry = item->data(Qt::UserRole).toMap();
                if (entry.value("source").toString() == SOURCE_INSTALLED_OS) // only installed OSes Can be backed up.
                {
                    //Get date/time
                    QDateTime tnow = QDateTime::currentDateTime();
                    QString now = "#" + tnow.toString("yyyyMMdd")+"-"+tnow.toString("hhmmss");;
                    while (now.left(5)=="#1970")
                    {
                        qDebug() << "Current time is not known";
                        //Open dialog to request current date.
                        DateTimeDialog dlg;
                        if (dlg.exec() == QDialog::Accepted)
                        {   //currentDateTime has been updated
                            tnow = QDateTime::currentDateTime();
                            now = "#" + tnow.toString("yyyyMMdd")+"-"+tnow.toString("hhmmss");
                        }
                        else
                        {
                            //QProcess::execute("mount -o remount,ro /dev/"+partdev(_osdrive,1)+" "+_local);
                            // The NTFS driver can't remount, so we'll just umount & mount again
                            QProcess::execute("umount /dev/"+partdev(_osdrive,1));
                            QProcess::execute("mount -o ro /dev/"+partdev(_osdrive,1)+" "+_local);

                            setEnabled(true);
                            return;
                        }
                    }

                    //Get partition number
                    QString partnr;
                    {
                        //@@ partnr = entry.value["partitions"].blahblah;
                    }

                    QString currentname = entry.value("name").toString();

                    //Create the new osname and the folder the backup will be stored in
                    QString backupName = getNameParts(currentname, eCORE|eNICKNAME) + now + partnr;
                    entry["backupName"]   = backupName;

                    //Dialog to update backupname, name & description
                    backupdialog dlg (entry, NULL);
                    if (QDialog::Rejected == dlg.exec())
                    {
                        setEnabled(true);
                        return;
                    }

                    backupName = entry.value("backupName").toString();
                    QString backupFolder = _local+"/os/" + getNameParts(backupName, eBASE|eDATE) + partnr;
                    backupFolder.replace(' ', '_'); //Reqd??

                    entry["backupFolder"] = backupFolder;
                    item->setData(Qt::UserRole, entry);

                    //Don't need flavours because they would already have been applied
                    QString settingsFolder = "/settings/os/"+ CORE(entry.value("name").toString()).replace(' ', '_');
                    //Copy:
                    QString cmd;

                    int errors =0;
                    cmd = "mkdir "+ backupFolder;
                    errors += QProcess::execute(cmd);

                    //- /slides_vga
                    if (QFile::exists(settingsFolder+"/slides_vga/"))
                    {
                        cmd = "cp -r "+ settingsFolder+"/slides_vga/ "+backupFolder;
                        errors += QProcess::execute(cmd);
                    }

                    //- os.json (with new description
                    cmd = "cp "+ settingsFolder+"/os.json "+backupFolder;
                    errors += QProcess::execute(cmd);
                    //- partitions.json
                    cmd = "cp "+ settingsFolder+"/partitions.json "+backupFolder;
                    errors += QProcess::execute(cmd);
                    //- partition_setup.sh
                    cmd = "cp "+ settingsFolder+"/partition_setup.sh "+backupFolder;
                    errors += QProcess::execute(cmd);

                    //- icon.png
                    QString iconfilename;
                    if (entry.contains("icon"))
                        iconfilename = entry.value("icon").toString();
                    else
                        iconfilename = settingsFolder+"/icon.png";
                    cmd = "cp "+ iconfilename+" "+backupFolder+"/"+CORE(backupName).replace(' ', '_')+".png";
                    errors += QProcess::execute(cmd);

                    //- [Copy release_notes.txt?]
                    cmd = "cp "+ settingsFolder+"/release_notes.txt "+backupFolder;
                    QProcess::execute(cmd); //Not critical

                    if (errors)
                    {
                        QMessageBox::critical(this,tr("Backup OSes"),"An error occurred backing up. Perhaps there is no disk space?", QMessageBox::Cancel);
                        setEnabled(true);
                        return;
                    }
                }
            }

            /* All OSes selected are local */
            startImageBackup();
        }
    }
}

void MainWindow::createPinnEntry()
{

    QVariantMap pinnMap;
    pinnMap["name"]=QString("PINN");
    pinnMap["description"]="An enhanced OS installer";
    pinnMap["url"]=INFO_URL;
    QString friendlyName = pinnMap.value("name").toString() + " [FIXED]\n" + pinnMap.value("description").toString();

    QVariantList list;
    list.append(partdev(_bootdrive, 1));
    pinnMap["partitions"]=list;

    QIcon icon(":/icons/hdd.png");
    QListWidgetItem *item = new QListWidgetItem(icon, friendlyName);
    item->setData(Qt::UserRole, pinnMap);
    item->setCheckState(Qt::Unchecked);
    ug->listInstalled->insertItem(0, item);
}

void MainWindow::on_actionClear_c_triggered()
{

    QList<QListWidgetItem *> selected = selectedItems();

    /* Get list of all selected OSes and see if any are unsupported */
    foreach (QListWidgetItem *item, selected)
    {
            item->setCheckState(Qt::Unchecked);
    }

    selected = ug->selectedInstalledItems();
    foreach (QListWidgetItem *item, selected)
    {
        item->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::on_actionTime_triggered()
{
    DateTimeDialog dlg;
    dlg.exec();
}

void MainWindow::on_actionRename_triggered()
{
    QListWidgetItem *item = ug->listInstalled->currentItem();
    QVariantMap m;

    if (item)
    {
        m = item->data(Qt::UserRole).toMap();
        renamedialog pDlg(m);
        pDlg.exec();

        ug->listInstalled->clear();
        addInstalledImages();
        updateInstalledStatus();

    }
}


void MainWindow::UpdateTime()
{

    if (!timeset)
    {
        qDebug() << "current date is "<< QDate::currentDate();
        qDebug() << "Requesting current time";
        QUrl url(BUILD_URL);
        QNetworkRequest request(url);
        request.setRawHeader("User-Agent", AGENT);
        QNetworkReply *reply = _netaccess->head(request);
        connect(reply, SIGNAL(finished()), this, SLOT(checkUpdateTime()));
    }
}

void MainWindow::checkUpdateTime()
{

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    setTime(reply);
}

void MainWindow::setTime(QNetworkReply *reply)
{
    /* Set our clock to server time if it is more than 1 day newer */
    QByteArray dateStr = reply->rawHeader("Date");

    if (!dateStr.isEmpty())
    {
        // Qt 4 does not have a standard function for parsing the Date header, but it does
        // have one for parsing a Last-Modified header that uses the same date/time format, so just use that
        QNetworkRequest dummyReq;
        dummyReq.setRawHeader("Last-Modified", dateStr);
        QDateTime parsedDate = dummyReq.header(dummyReq.LastModifiedHeader).toDateTime();

        QDateTime now = QDateTime::currentDateTime().addDays(1);
        if (parsedDate > now)
        {
            struct timeval tv;
            tv.tv_sec = parsedDate.toTime_t();
            tv.tv_usec = 0;
            settimeofday(&tv, NULL);
            timeset=true;
            qDebug() << "Time set to: " << parsedDate;
        }
    }
}

void MainWindow::on_actionReload_Repos_triggered()
{
#if 0
    QList<QListWidgetItem *> all;
    all = ug->allItems();
    QByteArray buffer;

    foreach (QListWidgetItem *item, all)
    {
        QVariantMap m = item->data(Qt::UserRole).toMap();
        buffer += Json::serialize( (QVariant)m);
    }
    qDebug()<<buffer;
#endif

    downloadRepoList(repoList);
}

