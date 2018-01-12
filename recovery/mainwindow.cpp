#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "multiimagewritethread.h"
#include "multiimagedownloadthread.h"
#include "initdrivethread.h"
#include "fullfatthread.h"
#include "confeditdialog.h"
#include "progressslideshowdialog.h"
#include "config.h"
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
#include "osgroup.h"
#include "fscheck.h"
#include "mydebug.h"

#include <QByteArray>
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
#include <QSplashScreen>
#include <QDesktopWidget>
#include <QSettings>
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
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/time.h>

#ifdef RASPBERRY_CEC_SUPPORT
extern "C" {
#include <interface/vmcs_host/vc_cecservice.h>
}
#endif

#ifdef Q_WS_QWS
#include <QWSServer>
#endif

void reboot_to_extended(const QString &defaultPartition, bool setDisplayMode);

extern CecListener * cec;

extern QStringList downloadRepoUrls;
extern QString repoList;

/* Main window
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

/* To keep track of where the different OSes get 'installed' from */
#define SOURCE_SDCARD "sdcard"
#define SOURCE_NETWORK "network"
#define SOURCE_INSTALLED_OS "installed_os"

//TODO Change to enums
#define TOOLBAR_MAIN 0
#define TOOLBAR_ARCHIVAL 1
#define TOOLBAR_MAINTENANCE 2
#define NUM_TOOLBARS 3

/* time in ms to poll for new disks */
#define POLLTIME 1000

/* Flag to keep track wheter or not we already repartitioned. */
bool MainWindow::_partInited = false;

/* Flag to keep track of current display mode. */
int MainWindow::_currentMode = 0;

MainWindow::MainWindow(const QString &drive, const QString &defaultDisplay, QSplashScreen *splash, bool noobsconfig, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _qpd(NULL), _kcpos(0), _defaultDisplay(defaultDisplay),
    _silent(false), _allowSilent(false), _showAll(false), _fixate(false), _splash(splash), _settings(NULL),
    _hasWifi(false), _numInstalledOS(0), _devlistcount(0), _netaccess(NULL), _displayModeBox(NULL), _drive(drive),
    _bootdrive(drive), _noobsconfig(noobsconfig), _numFilesToCheck(0), _eDownloadMode(MODE_INSTALL)

{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setContextMenuPolicy(Qt::NoContextMenu);
    update_window_title();
    _kc << 0x01000013 << 0x01000013 << 0x01000015 << 0x01000015 << 0x01000012
        << 0x01000014 << 0x01000012 << 0x01000014 << 0x42 << 0x41;

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

    QString cmdline = getFileContents("/proc/cmdline");
    ug = new OsGroup(this, ui, !cmdline.contains("no_group"));

    ug->list->setIconSize(QSize(40,40)); //ALL?? set each list?
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

    connect(cec, SIGNAL(keyPress(int)), this, SLOT(onKeyPress(int)));

    if (qApp->arguments().contains("-runinstaller") && !_partInited)
    {
        /* Repartition SD card first */
        _partInited = true;
        setEnabled(false);
        _qpd = new QProgressDialog( tr("Setting up SD card"), QString(), 0, 0, this);
        _qpd->setWindowModality(Qt::WindowModal);
        _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

        InitDriveThread *idt = new InitDriveThread(_bootdrive, this);
        connect(idt, SIGNAL(statusUpdate(QString)), _qpd, SLOT(setLabelText(QString)));
        connect(idt, SIGNAL(completed()), _qpd, SLOT(deleteLater()));
        connect(idt, SIGNAL(error(QString)), this, SLOT(onError(QString)));
        connect(idt, SIGNAL(query(QString, QString, QMessageBox::StandardButton*)),
                this, SLOT(onQuery(QString, QString, QMessageBox::StandardButton*)),
                Qt::BlockingQueuedConnection);

        idt->start();
        _qpd->exec();
        setEnabled(true);
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
                                  QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
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
    QProcess::execute("mount -o ro -t vfat "+partdev(_bootdrive, 1)+" /mnt");

    _model = getFileContents("/proc/device-tree/model");

    loadOverrides("/mnt/overrides.json");

    if (QFile::exists("/mnt/os_list_v3.json"))
    {
        /* We have a local os_list_v3.json for testing purposes */
        _repo = "/mnt/os_list_v3.json";

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

    if (cmdline.contains("showall"))
    {
        _showAll = true;
    }
    if (cmdline.contains("fixate"))
    {
        _fixate = true;
    }

    copyWpa();

    if (cmdline.contains("silentinstall"))
    {
        /* If silentinstall is specified, auto-install single image in /os */
        _allowSilent = true;
    }
    else
    {
        startNetworking();
    }

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

MainWindow::~MainWindow()
{
    disconnect(cec, SIGNAL(keyPress(int)), this, SLOT(onKeyPress(int)));

    QProcess::execute("umount /mnt");
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    disconnect(cec, SIGNAL(keyPress(int)), this, SLOT(onKeyPress(int)));
    event->accept();
}

QString MainWindow::menutext(int index)
{
    static const char* menutext_strings[] = {
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
        _qpd = new QProgressDialog(tr("Please wait while PINN initialises"), QString(), 0, 0, this);
        _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        _qpd->show();

        int timeout = 5000;
        if (getFileContents("/settings/wpa_supplicant.conf").contains("ssid="))
        {
            /* Longer timeout if we have a wifi network configured */
            timeout = 8000;
        }
        QTimer::singleShot(timeout, this, SLOT(hideDialogIfNoNetwork()));
        _time.start();
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
    repopulate();
    recalcAvailableMB();
    updateNeeded();

    ug->setDefaultItems();

    if (_allowSilent && !_numInstalledOS && ug->count() == 1)
    {
        // No OS installed, perform silent installation
        qDebug() << "Performing silent installation";
        _silent = true;
        ug->list->item(0)->setCheckState(Qt::Checked);
        on_actionWrite_image_to_disk_triggered();
        _numInstalledOS = 1;
    }

    ui->actionCancel->setEnabled(_numInstalledOS > 0);
}

void MainWindow::repopulate()
{
    QMap<QString,QVariantMap> images = listImages();
    bool haveicons = false;
    QSize currentsize = ug->list->iconSize();
    QIcon localIcon(":/icons/hdd.png");
    QIcon internetIcon(":/icons/download.png");
    _numInstalledOS = 0;

    createPinnEntry();
    foreach (QVariant v, images.values())
    {
        QVariantMap m = v.toMap();
        OverrideJson(m);
        QString flavour = m.value("name").toString();
        QString description = m.value("description").toString();
        QString folder  = m.value("folder").toString();
        QString iconFilename = m.value("icon").toString();
        bool installed = m.value("installed").toBool();
        bool recommended = m.value("recommended").toBool();

        if (!iconFilename.isEmpty() && !iconFilename.contains('/'))
            iconFilename = folder+"/"+iconFilename;
        if (!QFile::exists(iconFilename))
        {
            iconFilename = folder+"/"+flavour+".png";
            iconFilename.replace(' ', '_');
        }

        QString friendlyname = flavour;
        if (recommended)
            friendlyname += " ["+tr("RECOMMENDED")+"]";
        if (installed)
        {
            friendlyname += " ["+tr("INSTALLED")+"]";
            _numInstalledOS++;
        }
        if (!description.isEmpty())
            friendlyname += "\n"+description;

        QIcon icon;
        if (QFile::exists(iconFilename))
        {
            icon = QIcon(iconFilename);
            QList<QSize> avs = icon.availableSizes();
            if (avs.isEmpty())
            {
                /* Icon file corrupt */
                icon = QIcon();
            }
            else
            {
                QSize iconsize = avs.first();
                haveicons = true;

                if (iconsize.width() > currentsize.width() || iconsize.height() > currentsize.height())
                {
                    /* Make all icons as large as the largest icon we have */
                    currentsize = QSize(qMax(iconsize.width(), currentsize.width()),qMax(iconsize.height(), currentsize.height()));
                    ug->list->setIconSize(currentsize);
                }
            }
        }
        QListWidgetItem *item = new QListWidgetItem(icon, friendlyname);
        item->setData(Qt::UserRole, m);
#ifdef KHDBG
        qDebug() << "Repopulate: " << m << "\n";
#endif
        if (installed)
        {
            item->setData(Qt::BackgroundColorRole, INSTALLED_OS_BACKGROUND_COLOR);
            item->setCheckState(Qt::Checked);
        }
        else
            item->setCheckState(Qt::Unchecked);

        if (m["source"] == SOURCE_INSTALLED_OS)
        {
            item->setData(SecondIconRole, QIcon());
        }
        else
        {
            if (folder.startsWith("/mnt"))
                item->setData(SecondIconRole, localIcon);
            else
                item->setData(SecondIconRole, internetIcon);
        }

        if (installed)
        {
            QListWidgetItem *clone = item->clone();
            clone->setCheckState(Qt::Unchecked);
            ug->listInstalled->addItem(clone);
        }
        if (recommended)
            ug->insertItem(0, item);
        else
            ug->addItem(item);
    }

    if (haveicons)
    {
        /* Giving items without icon a dummy icon to make them have equal height and text alignment */
        QPixmap dummyicon = QPixmap(currentsize.width(), currentsize.height());
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
    }

    if (_numInstalledOS)
    {
        ui->actionCancel->setEnabled(true);
        if (_fixate)
        {
            ug->list->setEnabled(false);
        }
    }

    filterList();
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

    /* Display OS in list if it is supported or "showall" is specified in recovery.cmdline */
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

QMap<QString, QVariantMap> MainWindow::listImages(const QString &folder, bool includeInstalled)
{
    QMap<QString,QVariantMap> images;
    /* Local image folders */
    QDir dir(folder, "", QDir::Name, QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList list = dir.entryList();

    foreach (QString image,list)
    {
        QString imagefolder = folder+"/"+image;
        if (!QFile::exists(imagefolder+"/os.json"))
            continue;
        QVariantMap osv = Json::loadFromFile(imagefolder+"/os.json").toMap();
        osv["source"] = SOURCE_SDCARD;

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
                        QString name = fm.value("name").toString();
                        if (name == RECOMMENDED_IMAGE)
                            fm["recommended"] = true;
                        fm["folder"] = imagefolder;
                        fm["release_date"] = osv.value("release_date");
                        fm["source"] = osv.value("source");
                        fm["url"] = osv.value("url");
                        fm["group"] = osv.value("group");
                        images[name] = fm;
                    }
                }
            }
            else
            {
                QString name = basename;
                if (name == RECOMMENDED_IMAGE)
                    osv["recommended"] = true;
                osv["folder"] = imagefolder;
                images[name] = osv;
            }
        }
    }

    /* Also add information about files already installed */
    if (_settings && includeInstalled)
    {
        QVariantList i = Json::loadFromFile("/settings/installed_os.json").toList();
        foreach (QVariant v, i)
        {
            QVariantMap m = v.toMap();
            QString name = m.value("name").toString();
            if (images.contains(name))
            {
                images[name]["partitions"] = m["partitions"];
                images[name]["username"] = m["username"];
                images[name]["password"] = m["password"];
            }
            else
            {
                images[name] = m;
                if (name == RECOMMENDED_IMAGE)
                    images[name]["recommended"] = true;
                images[name]["source"] = SOURCE_INSTALLED_OS;
            }
            images[name]["installed"] = true;
        }
    }

    for (QMap<QString,QVariantMap>::iterator i = images.begin(); i != images.end(); i++)
    {
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

void MainWindow::on_actionWrite_image_to_disk_triggered()
{
    _eDownloadMode = MODE_INSTALL;

    bool allSupported = true;
    QString unsupportedOses;
    QString selectedOSes;

    QList<QListWidgetItem *> selected = selectedItems();

    /* Get list of all selected OSes and see if any are unsupported */
    foreach (QListWidgetItem *item, selected)
    {
        QVariantMap entry = item->data(Qt::UserRole).toMap();
        QString name = entry.value("name").toString();
        selectedOSes += "\n" + name;
        if (!isSupportedOs(name, entry))
        {
            allSupported = false;
            unsupportedOses += "\n" + name;
        }
    }

    QString warning = tr("Warning: this will install the selected Operating System(s) to ")+_drive+tr(":")+
            selectedOSes+
            tr("\nAll existing data on the SD card will be overwritten, including any OSes that are already installed. Continue?");
    if (_drive != "mmcblk0")
        warning.replace(tr("SD card"), tr("drive"));

    if (_silent || QMessageBox::warning(this,
                                        tr("Confirm"),
                                        warning,
                                        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        if (_silent || allSupported || QMessageBox::warning(this,
                                        tr("Confirm"),
                                        tr("Warning: incompatible Operating System(s) detected. The following OSes aren't supported on this revision of Raspberry Pi and may fail to boot or function correctly:") + unsupportedOses,
                                        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            setEnabled(false);
            _numMetaFilesToDownload = 0;
            if (_networkStatusPollTimer.isActive())
                _networkStatusPollTimer.stop();

            QList<QListWidgetItem *> selected = selectedItems();
            foreach (QListWidgetItem *item, selected)
            {
                QVariantMap entry = item->data(Qt::UserRole).toMap();

                if (!entry.contains("folder"))
                {
                    QDir d;
                    QString folder = "/settings/os/"+entry.value("name").toString();
                    folder.replace(' ', '_');
                    if (!d.exists(folder))
                        d.mkpath(folder);

                    downloadMetaFile(entry.value("os_info").toString(), folder+"/os.json");
                    downloadMetaFile(entry.value("partitions_info").toString(), folder+"/partitions.json");
                    if (entry.contains("marketing_info"))
                        downloadMetaFile(entry.value("marketing_info").toString(), folder+"/marketing.tar");

                    if (entry.contains("partition_setup"))
                        downloadMetaFile(entry.value("partition_setup").toString(), folder+"/partition_setup.sh");

                    if (entry.contains("icon"))
                        downloadMetaFile(entry.value("icon").toString(), folder+"/icon.png");
                }
            }

            if (_numMetaFilesToDownload == 0)
            {
                /* All OSes selected are local */
                startImageWrite();
            }
            else if (!_silent)
            {
                _qpd = new QProgressDialog(tr("The install process will begin shortly."), QString(), 0, 0, this);
                _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                _qpd->show();
            }
        }
    }
}

void MainWindow::on_actionReinstall_triggered()
{
    _eDownloadMode = MODE_REINSTALL;
    _numMetaFilesToDownload=0;

    QString warning = tr("Warning: this will Reinstall the selected Operating System(s). The existing data will be deleted.");

    if (QMessageBox::warning(this,
                                        tr("Confirm"),
                                        warning,
                                        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        {
            setEnabled(false);
            if (_networkStatusPollTimer.isActive())
                _networkStatusPollTimer.stop();

            QList<QListWidgetItem *> selected = ug->selectedInstalledItems();
            foreach (QListWidgetItem *item, selected)
            {

                QVariantMap installedEntry = item->data(Qt::UserRole).toMap();
                QString name = installedEntry.value("name").toString();

                if (name=="PINN")
                {
                    if (selected.count()==1)
                    {
                        //Only check upgrades to PINN if it is the ONLY Os to be reinstalled
                        //Because it causes a reboot
                        checkForUpdates( true );
                        return;
                    }
                    continue;   //Do not reinstall PINN - it's just a dummy os.
                }

                QListWidgetItem *witem = findItem(name);
                if (witem)
                {
                    QVariantMap new_details = witem->data(Qt::UserRole).toMap();
                    if (new_details.value("source").toString() == SOURCE_INSTALLED_OS)
                    {
                        onError(name + tr(" is not available.\nPlease provide it locally or connect to the internet."));
                        return;
                    }
                }

                //
                if (!installedEntry.contains("folder"))
                {
                    QDir d;
                    QString folder = "/settings/os/"+installedEntry.value("name").toString();
                    folder.replace(' ', '_');
                    if (!d.exists(folder))
                        d.mkpath(folder);

                    downloadMetaFile(installedEntry.value("os_info").toString(), folder+"/os.json");
                    downloadMetaFile(installedEntry.value("partitions_info").toString(), folder+"/partitions.json");
                    if (installedEntry.contains("marketing_info"))
                        downloadMetaFile(installedEntry.value("marketing_info").toString(), folder+"/marketing.tar");

                    if (installedEntry.contains("partition_setup"))
                        downloadMetaFile(installedEntry.value("partition_setup").toString(), folder+"/partition_setup.sh");

                    if (installedEntry.contains("icon"))
                        downloadMetaFile(installedEntry.value("icon").toString(), folder+"/icon.png");
                }
            }

            if (_numMetaFilesToDownload == 0)
            {
                /* All OSes selected are local */
                startImageReinstall();
            }
        }
    }
}

void MainWindow::on_actionDownload_triggered()
{
    _eDownloadMode = MODE_DOWNLOAD;

    //@@ maybe here decide if to download to /mnt or /settings and only mount that one rw

    _local = "/tmp/media/"+partdev(_osdrive,1);
    if (QProcess::execute("mount -o remount,rw /dev/"+partdev(_osdrive,1)+" "+_local) != 0)
    {
        return;
    }
    // OR ....
    // QProcess::execute("mount -o remount,rw /mnt");

    if (_silent || QMessageBox::warning(this,
                                        tr("Confirm"),
                                        tr("Warning: this will download the selected Operating System(s)."),
                                        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        /* See if any of the OSes are unsupported */
        bool allSupported = true;
        QString unsupportedOses;
        QList<QListWidgetItem *> selected = selectedItems();
        //@@ Check for unsupported (undownloadable) OSes
        if (_silent || allSupported || QMessageBox::warning(this,
                                        tr("Confirm"),
                                        tr("Warning: incompatible Operating System(s) detected. The following OSes aren't supported on this revision of Raspberry Pi and may fail to boot or function correctly:") + unsupportedOses,
                                        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
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

                    downloadMetaFile(entry.value("os_info").toString(), folder+"/os.json");
                    downloadMetaFile(entry.value("partitions_info").toString(), folder+"/partitions.json");
                    QString urlpath = entry.value("os_info").toString().left(entry.value("os_info").toString().lastIndexOf('/'));
                    downloadMetaFile(urlpath+"/release_notes.txt", "-" + folder+"/release_notes.txt"); //'-' indicates optional
                    if (entry.contains("marketing_info"))
                        downloadMetaFile(entry.value("marketing_info").toString(), folder+"/marketing.tar");

                    if (entry.contains("partition_setup"))
                        downloadMetaFile(entry.value("partition_setup").toString(), folder+"/partition_setup.sh");

                    if (entry.contains("icon"))
                    {
                        //Extract icon filename from URL
                        QStringList splitted = entry.value("icon").toString().split("/");
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
}

void MainWindow::onCompleted()
{
    int ret = QMessageBox::Ok;

    _qpd->hide();
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
            ret = QMessageBox::information(this,
                                     tr("OS(es) downloaded"),
                                     tr("OS(es) Downloaded Successfully.\nReboot PINN to take account of these OSes?"), QMessageBox::Ok|QMessageBox::Cancel);
        }
        else
        {
            ret = QMessageBox::information(this,
                                     tr("OS(es) installed"),
                                     tr("OS(es) Installed Successfully"), QMessageBox::Ok);
        }
    }
    _qpd->deleteLater();
    _qpd = NULL;
    if (_eDownloadMode==MODE_DOWNLOAD)
    {
        setEnabled(true);
        show();

        if (ret == QMessageBox::Ok)
        {
            //@@Temporary solution....
            // Shut down networking
            QProcess::execute("ifdown -a");
            // Unmount file systems
            QProcess::execute("umount -ar");
            ::sync();
            // Reboot
            ::reboot(RB_AUTOBOOT);

            //@@What we really want to do is just refresh the dialog, but not possible yet.
            //repopulate();
        }
    }

    if (_eDownloadMode == MODE_REINSTALL)
    {
        setEnabled(true);
        show();
    }
    if (_eDownloadMode == MODE_INSTALL)
        close();
}

void MainWindow::onError(const QString &msg)
{
    qDebug() << "Error:" << msg;
    if (_qpd)
        _qpd->hide();
    QMessageBox::critical(this, tr("Error"), msg, QMessageBox::Close);
    setEnabled(true);
    _piDrivePollTimer.start(POLLTIME);
    show();
}

void MainWindow::onQuery(const QString &msg, const QString &title, QMessageBox::StandardButton* answer)
{
    *answer = QMessageBox::question(this, title, msg, QMessageBox::Yes|QMessageBox::No);
}



void MainWindow::on_list_currentRowChanged()
{
    updateActions();
}

void MainWindow::update_window_title()
{
    setWindowTitle(QString(tr("PINN v%1 - Built: %2 (%3)")).arg(VERSION_NUMBER).arg(QString::fromLocal8Bit(__DATE__)).arg(_ipaddress.toString()));
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        ug->retranslateUI();
        update_window_title();
        updateNeeded();
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
    //@@ _splash->setPixmap(QPixmap(":/wallpaper.png")); //Comment out for now whilst changing pixmap background
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
        // Catch Return key to trigger OS boot
        if (keyEvent->key() == Qt::Key_Return)
        {
            on_list_doubleClicked(ug->list->currentIndex());
        }

        // catch toolbar changes (PageDown is same as M)
        if (keyEvent->key() == Qt::Key_PageDown)
        {
            on_actionAdvanced_triggered();
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
    QLabel* info = new QLabel(this);
    info->setPixmap(QPixmap("/usr/data"));
    info->setGeometry(0,0,640,480);
    info->show();
}

void MainWindow::on_actionAdvanced_triggered()
{
    toolbars.value(toolbar_index)->setVisible(false);
    toolbar_index = (toolbar_index+1)%NUM_TOOLBARS;
    toolbars.value(toolbar_index)->setVisible(true);

    ui->groupBox->setVisible(toolbar_index == TOOLBAR_MAIN);
    ui->groupBoxUsb->setVisible(toolbar_index == TOOLBAR_ARCHIVAL);

    if (_menuLabel)
        _menuLabel->setText(menutext(toolbar_index));

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
            ConfEditDialog d(m, partition);
            d.exec();
        }
    }
}

void MainWindow::on_actionBrowser_triggered()
{
#if KHDBG
    for (int i=0; i<ug->list->count(); i++)
    {
        QListWidgetItem *item = ug->list->item(i);
        QVariantMap m = item->data(Qt::UserRole).toMap();
        qDebug() << m;
    }
#endif
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
    connect(fft, SIGNAL(error(QString)), this, SLOT(onError(QString)));
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
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
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
    QProcess *proc = new QProcess(this);
    QString lang = LanguageDialog::instance("en", "gb")->currentLanguage();
    if (lang == "gb" || lang == "us" || lang == "ko" || lang == "")
        lang = "en";
    proc->start("arora -lang "+lang+" "+HOMEPAGE);
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

void MainWindow::copyWpa()
{
    //This file is the one used by dhcpcd
    QFile f("/settings/wpa_supplicant.conf");
    if ( f.exists() && f.size() == 0 )
    {
        /* Remove corrupt file */
        f.remove();
    }

    /* If user supplied a wpa_supplicant.conf on the FAT partition copy that one to settings regardless */
    if (QFile::exists("/mnt/wpa_supplicant.conf"))
    {
        qDebug() << "Copying  user wpa_supplicant.conf to /settings/wpa_supplicant.conf";

        QProcess::execute("mount -o remount,rw /settings");
        QProcess::execute("mount -o remount,rw /mnt");

        QFile::remove("/settings/wpa_supplicant.conf.bak");
        QFile::rename("/settings/wpa_supplicant.conf","/settings/wpa_supplicant.conf.bak");
        QFile::copy("/mnt/wpa_supplicant.conf", "/settings/wpa_supplicant.conf");
        f.setPermissions( QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther | QFile::ReadUser );

        /* rename the user file to avoid overwriting any manually set SSIDs */
        QFile::remove("/mnt/wpa_supplicant.conf.bak");
        QFile::rename("/mnt/wpa_supplicant.conf","/mnt/wpa_supplicant.conf.bak");

        QProcess::execute("sync");
        QProcess::execute("mount -o remount,ro /settings");
        QProcess::execute("mount -o remount,ro /mnt");
    }
    else if ( !f.exists() )
    {
        /* There is no existing file, must be first installation */
        qDebug() << "Copying /etc/wpa_supplicant.conf to /settings/wpa_supplicant.conf";
        QFile::copy("/etc/wpa_supplicant.conf", "/settings/wpa_supplicant.conf");
    }
    QFile::remove("/etc/wpa_supplicant.conf");
}


void MainWindow::startNetworking()
{
    /* Enable dbus so that we can use it to talk to wpa_supplicant later */
    qDebug() << "Starting dbus";
    QProcess::execute("/etc/init.d/S30dbus start");

    /* Run dhcpcd in background */
    QProcess *proc = new QProcess(this);
    qDebug() << "Starting dhcpcd";
    proc->start("/sbin/dhcpcd --noarp -e wpa_supplicant_conf=/settings/wpa_supplicant.conf --denyinterfaces \"*_ap\"");

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

            QString cmdline = getFileContents("/proc/cmdline");
            if (!cmdline.contains("no_update"))
                checkForUpdates();
            else
                qDebug()<<"Skipping self update check";

            downloadRepoList(repoList);
        }
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

    /* Set our clock to server time if we currently have a date before 2015 */
    QByteArray dateStr = reply->rawHeader("Date");
    if (!dateStr.isEmpty() && QDate::currentDate().year() < 2016)
    {
        // Qt 4 does not have a standard function for parsing the Date header, but it does
        // have one for parsing a Last-Modified header that uses the same date/time format, so just use that
        QNetworkRequest dummyReq;
        dummyReq.setRawHeader("Last-Modified", dateStr);
        QDateTime parsedDate = dummyReq.header(dummyReq.LastModifiedHeader).toDateTime();

        struct timeval tv;
        tv.tv_sec = parsedDate.toTime_t();
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);

        qDebug() << "Time set to: " << parsedDate;
    }

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
            _qpd->hide();
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

    //qDebug() << "processRepoListJson: " << list;

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
        _qpd->hide();
        _qpd->deleteLater();
        _qpd = NULL;
    }
    else
    {
        foreach (QString url, urls)
        {
            qDebug() << "Downloading list from " << url;
            if (url.startsWith("/"))
                processJson( Json::parse(getFileContents(url)) );
            else
                downloadList(url);
        }
    }
}

void MainWindow::downloadList(const QString &urlstring)
{
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

    if (reply->error() != reply->NoError || httpstatuscode < 200 || httpstatuscode > 399)
    {
        if (_qpd)
            _qpd->hide();
        QString errstr = tr("Error downloading distribution list from Internet:\n") + reply->url().toString();
        qDebug() << "Error Downloading "<< reply->url()<<" reply: "<< reply->error() << " httpstatus: "<< httpstatuscode;
        QMessageBox::critical(this, tr("Download error"), errstr, QMessageBox::Close);
    }
    else
    {
        processJson(Json::parse( reply->readAll() ));
    }

    ug->setFocus();

    reply->deleteLater();
}

void MainWindow::processJson(QVariant json)
{
    if (json.isNull())
    {
        QMessageBox::critical(this, tr("Error"), tr("Error parsing list.json downloaded from server"), QMessageBox::Close);
        return;
    }

    QSet<QString> iconurls;
    QVariantList list = json.toMap().value("os_list").toList();

    foreach (QVariant osv, list)
    {
        QVariantMap  os = osv.toMap();

        QString basename = os.value("os_name").toString();
        if (canInstallOs(basename, os))
        {
            if (os.contains("flavours"))
            {
                QVariantList flavours = os.value("flavours").toList();

                foreach (QVariant flv, flavours)
                {
                    QVariantMap flavour = flv.toMap();
                    QVariantMap item = os;
                    QString name        = flavour.value("name").toString();
                    QString description = flavour.value("description").toString();
                    QString iconurl     = flavour.value("icon").toString();

                    item.insert("name", name);
                    item.insert("description", description);
                    item.insert("icon", iconurl);
                    item.insert("feature_level", flavour.value("feature_level"));
                    item.insert("source", SOURCE_NETWORK);

                    processJsonOs(name, item, iconurls);
                }
            }
            if (os.contains("description"))
            {
                QString name = basename;
                os["name"] = name;
                os["source"] = SOURCE_NETWORK;
                processJsonOs(name, os, iconurls);
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

    filterList();
}

void MainWindow::processJsonOs(const QString &name, QVariantMap &new_details, QSet<QString> &iconurls)
{
    QIcon internetIcon(":/icons/download.png");
    OverrideJson(new_details);

    QListWidgetItem *witem = findItem(name);
    if (witem)
    {
        QVariantMap existing_details = witem->data(Qt::UserRole).toMap();

        if ((existing_details["release_date"].toString() < new_details["release_date"].toString()) || (existing_details["source"].toString() == SOURCE_INSTALLED_OS))
        {
            /* Local version is older (or unavailable). Replace info with newer Internet version */
            new_details.insert("installed", existing_details.value("installed", false));
            if (existing_details.contains("partitions"))
            {
                new_details["partitions"] = existing_details["partitions"];
            }
            witem->setData(Qt::UserRole, new_details);
            witem->setData(SecondIconRole, internetIcon);
            ug->list->update();
        }

    }
    else if (!_fixate || _numInstalledOS == 0)
    {
        /* It's a new OS, so add it to the list */
        QString iconurl = new_details.value("icon").toString();
        QString description = new_details.value("description").toString();

        if (!iconurl.isEmpty())
            iconurls.insert(iconurl);

        bool recommended = (name == RECOMMENDED_IMAGE);

        QString friendlyname = name;
        if (recommended)
            friendlyname += " ["+tr("RECOMMENDED")+"]";
        if (!description.isEmpty())
            friendlyname += "\n"+description;

        witem = new QListWidgetItem(friendlyname);
        witem->setCheckState(Qt::Unchecked);
        witem->setData(Qt::UserRole, new_details);
        witem->setData(SecondIconRole, internetIcon);

        if (recommended)
            ug->insertItem(0, witem);
        else
            ug->addItem(witem);
    }

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
    QUrl url(urlstring);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, originalurl);
    request.setRawHeader("User-Agent", AGENT);
    QNetworkReply *reply = _netaccess->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadIconRedirectCheck()));
}

QListWidgetItem *MainWindow::findItem(const QVariant &name)
{
    QList<QListWidgetItem *> all;
    all = ug->allItems();

    foreach (QListWidgetItem *item, all)
    {
        QVariantMap m = item->data(Qt::UserRole).toMap();
        if (m.value("name").toString() == name.toString())
        {
            return item;
        }
    }
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
        qDebug() << "Error downloading icon" << url;
    }
    else
    {
        QPixmap pix;
        pix.loadFromData(reply->readAll());
        QIcon icon(pix);

        //Set the icon in the OS list dialog box.
        QList<QListWidgetItem *> all;
        all = ug->allItems();

        for (int i=0; i<ug->count(); i++)
        {
            QVariantMap m = all.value(i)->data(Qt::UserRole).toMap();
            ug->list->setIconSize(QSize(40,40)); //ALL??
            if (m.value("icon") == originalurl)
            {
                all.value(i)->setIcon(icon);
            }
        }
    }
    if (--_numIconsToDownload == 0 && _qpd)
    {
        _qpd->hide();
        _qpd->deleteLater();
        _qpd = NULL;
    }

    reply->deleteLater();
    _listno++;
    //@@? if (_listno ==1)
    //@@?    downloadList(DEFAULT_REPO_SERVER);

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
        if (_neededDownloadMB)
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
    ui->actionPassword->setEnabled(item);
    ui->actionInfoInstalled->setEnabled(item && item->data(Qt::UserRole).toMap().contains("url"));

    QList<QListWidgetItem *> select = ug->selectedInstalledItems();
    ui->actionFschk->setEnabled( select.count() );
    ui->actionReinstall->setEnabled( select.count() );

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

void MainWindow::on_list_itemChanged(QListWidgetItem *)
{
    updateNeeded();
    updateActions();
}

void MainWindow::downloadMetaFile(const QString &urlstring, const QString &saveAs)
{
    qDebug() << "Downloading" << urlstring << "to" << saveAs;
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

    /* Set our clock to server time if we currently have a date before 2015 */
    QByteArray dateStr = reply->rawHeader("Date");
    if (!dateStr.isEmpty() && QDate::currentDate().year() < 2016)
    {
        // Qt 4 does not have a standard function for parsing the Date header, but it does
        // have one for parsing a Last-Modified header that uses the same date/time format, so just use that
        QNetworkRequest dummyReq;
        dummyReq.setRawHeader("Last-Modified", dateStr);
        QDateTime parsedDate = dummyReq.header(dummyReq.LastModifiedHeader).toDateTime();

        struct timeval tv;
        tv.tv_sec = parsedDate.toTime_t();
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);

	qDebug() << "Time set to: " << parsedDate;
    }

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        //qDebug() << "Redirection - Re-trying download from" << redirectionurl;
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
        //qDebug() << "Redirection - Re-trying download from" << redirectionurl;
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
        //qDebug() << "Redirection - Re-trying download from" << redirectionurl;
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
            //qDebug() << tr("Error downloading meta file: ")+reply->url().toString() + tr(". Continuing\n");
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
        else if (_eDownloadMode == MODE_REINSTALL)
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
        //qDebug() << "Redirection - Re-trying filesize from" << redirectionurl;
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
        QListWidgetItem *witem = findItem(osname);
        if (witem)
        {
            QVariantMap existing_details = witem->data(Qt::UserRole).toMap();
            //get its download_size
            quint64 old_size = existing_details.value("download_size").toULongLong();
            //Increment by length
            old_size += length;
            //write back
            existing_details["download_size"] = old_size;

            if (existing_details["source"]==SOURCE_NETWORK)
                witem->setData(Qt::UserRole,existing_details); //Only update if it is still a network source!
        }
        else
        {
            qDebug() << "Cannot find " << osname << " to set download_size";
        }
    }
    //decrement number of filesizes to download
    _numFilesToCheck--;
    qDebug() << "Length:" << length << "files left: " <<_numFilesToCheck << " " << osname;
    if (_numFilesToCheck<2)
        updateNeeded();
}

void MainWindow::startImageWrite()
{
    _piDrivePollTimer.stop();
    /* All meta files downloaded, extract slides tarball, and launch image writer thread */
    MultiImageWriteThread *imageWriteThread = new MultiImageWriteThread(_bootdrive, _drive, _noobsconfig);
    QString folder, slidesFolder;
    QStringList slidesFolders;

    QList<QListWidgetItem *> selected = selectedItems();
    foreach (QListWidgetItem *item, selected)
    {
        QVariantMap entry = item->data(Qt::UserRole).toMap();

        if (entry.contains("folder"))
        {
            /* Local image */
            folder = entry.value("folder").toString();
        }
        else
        {
            folder = "/settings/os/"+entry.value("name").toString();
            folder.replace(' ', '_');

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
        //QRect s = QApplication::desktop()->screenGeometry();
        //if (s.width() > 640 && QFile::exists(folder+"/slides"))
        //{
        //    slidesFolder = folder+"/slides";
        //}
        if (QFile::exists(folder+"/slides_vga"))
        {
            slidesFolder = folder+"/slides_vga";
        }
        imageWriteThread->addImage(folder, entry.value("name").toString());
        if (!slidesFolder.isEmpty())
            slidesFolders.append(slidesFolder);
    }

    if (slidesFolders.isEmpty())
        slidesFolder.append("/mnt/defaults/slides");

    _qpd = new ProgressSlideshowDialog(slidesFolders, "", 20, _drive, this);
    _qpd->setWindowTitle("Installing Images");
    connect(imageWriteThread, SIGNAL(parsedImagesize(qint64)), _qpd, SLOT(setMaximum(qint64)));
    connect(imageWriteThread, SIGNAL(completed()), this, SLOT(onCompleted()));
    connect(imageWriteThread, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(imageWriteThread, SIGNAL(statusUpdate(QString)), _qpd, SLOT(setLabelText(QString)));
    connect(imageWriteThread, SIGNAL(runningMKFS()), _qpd, SLOT(pauseIOaccounting()), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(finishedMKFS()), _qpd , SLOT(resumeIOaccounting()), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(newDrive(const QString&)), _qpd , SLOT(changeDrive(const QString&)), Qt::BlockingQueuedConnection);
    imageWriteThread->start();
    hide();
    _qpd->exec();
}

void MainWindow::startImageReinstall()
{
    _piDrivePollTimer.stop();
    /* All meta files downloaded, extract slides tarball, and launch image writer thread */
    MultiImageWriteThread *imageWriteThread = new MultiImageWriteThread(_bootdrive, _drive, _noobsconfig, false);
    QString folder, slidesFolder;
    QStringList slidesFolders;

    QList<QListWidgetItem *> selected = ug->selectedInstalledItems();
    foreach (QListWidgetItem * item, selected)
    {
        QVariantMap entry = item->data(Qt::UserRole).toMap();

        //qDebug() <<entry;

        if (entry.value("name")=="PINN")
        {
            continue;   //Do not reinstall PINN - it's just a dummy os.
        }

        if (entry.contains("folder"))
        {
            /* Local image */
            folder = entry.value("folder").toString();
        }
        else
        {
            folder = "/settings/os/"+entry.value("name").toString();
            folder.replace(' ', '_');

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
            Json::saveToFile(folder+"/partitions.json", json);
        }

        slidesFolder.clear();
        if (QFile::exists(folder+"/slides_vga"))
        {
            slidesFolder = folder+"/slides_vga";
        }

        //qDebug() << "@@@@ " << entry;
        imageWriteThread->addInstalledImage(folder, entry.value("name").toString(), entry); //@@

        if (!slidesFolder.isEmpty())
            slidesFolders.append(slidesFolder);
    }
    if (slidesFolders.isEmpty())
        slidesFolder.append("/mnt/defaults/slides");

    _qpd = new ProgressSlideshowDialog(slidesFolders, "", 20, _drive, this);
    _qpd->setWindowTitle("Installing Images");
    connect(imageWriteThread, SIGNAL(parsedImagesize(qint64)), _qpd, SLOT(setMaximum(qint64)));
    connect(imageWriteThread, SIGNAL(completed()), this, SLOT(onCompleted()));
    connect(imageWriteThread, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(imageWriteThread, SIGNAL(statusUpdate(QString)), _qpd, SLOT(setLabelText(QString)));
    connect(imageWriteThread, SIGNAL(runningMKFS()), _qpd, SLOT(pauseIOaccounting()), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(finishedMKFS()), _qpd , SLOT(resumeIOaccounting()), Qt::BlockingQueuedConnection);
    connect(imageWriteThread, SIGNAL(newDrive(const QString&)), _qpd , SLOT(changeDrive(const QString&)), Qt::BlockingQueuedConnection);
    imageWriteThread->start();
    hide();
    _qpd->exec();

}


void MainWindow::startImageDownload()
{
    _piDrivePollTimer.stop();
    // The drive is already mounted R/W from on_actionDownload_triggered

    /* All meta files downloaded, extract slides tarball, and launch image download thread */
    MultiImageDownloadThread *imageDownloadThread = new MultiImageDownloadThread(0, _local);
    QString folder, slidesFolder;
    QStringList slidesFolders;

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

            slidesFolder.clear();
            if (QFile::exists(folder+"/slides_vga"))
            {
                slidesFolder = folder+"/slides_vga";
            }

            /* Insert download_size into os.json to allow correct use of download size */
            json = Json::loadFromFile(folder+"/os.json").toMap();
            if (! json.contains("download_size"))
            {
                quint64 downloadSize= entry.value("download_size").toULongLong();
                json.insert("download_size",downloadSize);
                Json::saveToFile(folder+"/os.json", json);
            }

            imageDownloadThread->addImage(folder, entry.value("name").toString());
            if (!slidesFolder.isEmpty())
                slidesFolders.append(slidesFolder);
        }
    }

    if (slidesFolders.isEmpty())
        slidesFolder.append("/mnt/defaults/slides");

    _qpd = new ProgressSlideshowDialog(slidesFolders, "", 20, _osdrive, this);
    _qpd->setWindowTitle("Downloading Images");
    connect(imageDownloadThread, SIGNAL(parsedImagesize(qint64)), _qpd, SLOT(setMaximum(qint64)));
    connect(imageDownloadThread, SIGNAL(completed()), this, SLOT(onCompleted()));
    connect(imageDownloadThread, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(imageDownloadThread, SIGNAL(statusUpdate(QString)), _qpd, SLOT(setLabelText(QString)));
    imageDownloadThread->start();
    hide();
    _qpd->exec();
    show();

    //QProcess::execute("mount -o remount,ro /mnt");
}


void MainWindow::hideDialogIfNoNetwork()
{
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
    QString dirname = "/sys/class/block";
    QDir dir(dirname);
    QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

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
                    _osdrive=devname; //eg 'sda'
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
    _availableMB = (getFileContents(sysclassblock(_drive)+"/size").trimmed().toULongLong()-getFileContents(sysclassblock(_drive, 5)+"/start").trimmed().toULongLong()-getFileContents(sysclassblock(_drive, 5)+"/size").trimmed().toULongLong())/2048;


    QString classdev = sysclassblock(_osdrive, 1);
    if (QFile::exists(classdev))
    {
        QProcess proc;
        QString cmd = "sh -c \"df -m /dev/" + partdev(_osdrive,1) + " | grep  /dev/" + partdev(_osdrive,1) + " | sed 's| \\+| |g' | cut -d' ' -f 4 \"";
        //qDebug() << cmd;

        proc.start(cmd);
        proc.waitForFinished();
        QString result = proc.readAll();
        //qDebug()<< result;

        _availableDownloadMB = result.toInt();
        //qDebug() << classdev << " : " << _availableDownloadMB;
    }
    else
    {
        _availableDownloadMB = 0;
        //qDebug() << classdev << " : " << _availableDownloadMB;
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
            if (QMessageBox::question(this,
                                      tr("Reformat drive?"),
                                      tr("Are you sure you want to reformat the drive '%1' for use with PINN? All existing data on the drive will be deleted!").arg(devname),
                                      QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
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

void MainWindow::addImagesFromUSB(const QString &device)
{
    QDir dir;
    QString mntpath = "/tmp/media/"+device;

    dir.mkpath(mntpath);
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
    QMap<QString,QVariantMap> images = listImages(mntpath+"/os", false);

    foreach (QVariant v, images.values())
    {
        QVariantMap m = v.toMap();
        OverrideJson(m);
        QString name = m.value("name").toString();
        QString folder  = m.value("folder").toString();

        QListWidgetItem *witem = findItem(name);
        if (witem)
        {
            QVariantMap existing_details = witem->data(Qt::UserRole).toMap();

            if ((existing_details["release_date"].toString() <= m["release_date"].toString()))
            {
                /* Existing item in list is same version or older. Prefer image on USB storage. */
                m.insert("installed", existing_details.value("installed", false));
                if (existing_details.contains("partitions"))
                {
                    m["partitions"] = existing_details["partitions"];
                }
                witem->setData(Qt::UserRole, m);
                witem->setData(SecondIconRole, usbIcon);
                ug->list->update();
            }
        }
        else
        {
            /* It's a new OS, so add it to the list */
            QString description = m.value("description").toString();
            QString iconFilename = m.value("icon").toString();
            bool recommended = m.value("recommended").toBool();

            if (!iconFilename.isEmpty() && !iconFilename.contains('/'))
                iconFilename = folder+"/"+iconFilename;
            if (!QFile::exists(iconFilename))
            {
                iconFilename = folder+"/"+name+".png";
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
            witem->setData(SecondIconRole, usbIcon);
            if (QFile::exists(iconFilename))
                witem->setIcon(QIcon(iconFilename));

            if (recommended)
                ug->insertItem(0, witem);
            else
                ug->addItem(witem);
        }
    }

    filterList();
    ug->setFocus();
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
        {
            QVariantMap m = witem->data(Qt::UserRole).toMap();
            bool supportsUsb;
            QString param;

            if (_bootdrive == "/dev/mmcblk0")
                param = "supports_usb_root";
            else
                param = "supports_usb_boot";


            /* If the repo explicity states wheter or not usb is supported use that info */
            if (m.contains(param))
            {
                supportsUsb = m.value(param).toBool();
            }
            else
            {
                /* Otherwise just assume Linux does, and RiscOS and Windows do not */
                QString name = m.value("name").toString();
                supportsUsb = (!nameMatchesRiscOS(name) && !name.contains("Windows", Qt::CaseInsensitive));
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
    int result = pDlg.exec();

    if (result==QDialog::Rejected)
        return;

    src=pDlg.get_src();
    dst=pDlg.get_dst();
    src_dev=pDlg.get_src_dev();
    dst_dev=pDlg.get_dst_dev();

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
        piCloneThread *cloneThread = new piCloneThread(src_dev, dst_dev);
        QStringList DirList;
        setEnabled(false);
        //Reuse the existing Progress Slide Dialog
        _qpd = new ProgressSlideshowDialog(DirList, "", 20);//Add dst_dev
        _qpd->setWindowTitle("Clone SD Card");
        ((ProgressSlideshowDialog*)_qpd)->disableIOaccounting();
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
        _qpd->hide();
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
        //qDebug() << "Passwd triggered: " << m;
        if (m.contains("partitions"))
        {
            //QVariantList l = item->data(Qt::UserRole).toMap().value("partitions").toList();
            Passwd pDlg(m);
            pDlg.exec();
        }
    }
}

void MainWindow::checkForUpdates(bool display)
{
    _bdisplayUpdate = display;
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
    //qDebug() << "Redirect check" << redirectionurl << "to" << saveAs << " Reply="<<httpstatuscode;

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        //qDebug() << "Redirection - Re-trying download from" << redirectionurl;
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
            if (_bdisplayUpdate && type =="BUILD")
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
        QProcess::execute("mount -o remount,rw /mnt");
        QProcess::execute("unzip /tmp/pinn-lite.zip -o -x recovery.cmdline -d /mnt");
        QProcess::execute("mount -o remount,ro /mnt");
        QProcess::execute("sync");
        if (_qpd)
        {
            _qpd->hide();
            _qpd->deleteLater();
            _qpd = NULL;
        }

        QByteArray partition("1");
        setRebootPartition(partition);
        ::sync();
        // Reboot
        ::reboot(RB_AUTOBOOT);
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
        BuildData currentver, newver;

        qDebug()<<"BUILD_IGNORE...";
        currentver.read(BUILD_IGNORE);
        if (currentver.isEmpty())
        {
            qDebug()<<"BUILD_CURRENT...";
            currentver.read(BUILD_CURRENT);
        }
        qDebug()<<"BUILD_NEW...";
        newver.read(BUILD_NEW);

        if (newver > currentver)
        {
            on_newVersion();
        }
        else if (_bdisplayUpdate)
        {
            QMessageBox::information(this, tr("PINN Update Check"), tr("No updates available"), QMessageBox::Close);
        }
    }
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

    int ret = msgBox.exec();
    switch (ret)
    {
        case QMessageBox::Yes:
            // Yes was clicked
            setEnabled(false);
            _qpd = new QProgressDialog( QString(tr("Downloading Update")), QString(tr("Press ESC to cancel")), 0, 0, this);
            _qpd->setWindowModality(Qt::WindowModal);
            _qpd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            _qpd->show();
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
void MainWindow::onKeyPress(int cec_code)
{
#ifdef Q_WS_QWS

    const int key_map[NUM_TOOLBARS][6]={
        /* KEYS:             1          2          3          4          5               6 */
        /* Main Menu   */   {Qt::Key_I, Qt::Key_W, Qt::Key_H, Qt::Key_N, Qt::Key_Escape, 0},
        /* Archival    */   {Qt::Key_D, Qt::Key_C, Qt::Key_N, 0, 0, 0},
        /* Maintenance */   {Qt::Key_E, Qt::Key_P, Qt::Key_N, 0, 0, 0}
    };

    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    int key=0;
    QPoint p = QCursor::pos();
    int menu =toolbar_index;

    switch (cec_code)
    {
/* MOUSE SIMULATION */
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
    case CEC_User_Control_Select:
        { //CLick!
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
        break;

/* ARROW KEY SIMULATION */
    case CEC_User_Control_Play:
        key = Qt::Key_Space;
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

    case CEC_User_Control_F2Red:
        key = Qt::Key_M;
        modifiers = Qt::ControlModifier;
        break;

/* Key 9 is always menu selection toggle */
    case CEC_User_Control_Number9:
        key = Qt::Key_PageDown;
        break;
/* SPECIAL NUMBER KEYS FOR THIS DIALOG */
    default:
        if (cec_code > CEC_User_Control_Number0 && cec_code < CEC_User_Control_Number7)
        {   //Keys 1..6
            key = key_map[menu][cec_code - CEC_User_Control_Number1];
        }
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
    qDebug() << "onKeyPress" << cec_code;
#endif
}

void MainWindow::on_actionInfo_triggered()
{
    if (!requireNetwork())
        return;

    QListWidgetItem * item = NULL;
    item = ug->list->currentItem();
    if (!item)
    {
        //qDebug()<<"No List Item";
        return;
    }
    QVariantMap m = item->data(Qt::UserRole).toMap();
    if (m.contains("url"))
    {
        QProcess *proc = new QProcess(this);
        QString lang = LanguageDialog::instance("en", "gb")->currentLanguage();
        if (lang == "gb" || lang == "us" || lang == "ko" || lang == "")
            lang = "en";
        proc->start("arora -lang "+lang+" "+m.value("url").toString());
    }
    //else
        //qDebug() << m;

}

void MainWindow::on_actionInfoInstalled_triggered()
{
    if (!requireNetwork())
        return;

    QListWidgetItem * item = NULL;
    item = ug->listInstalled->currentItem();
    if (!item)
    {
        //qDebug()<<"No List Item";
        return;
    }
    QVariantMap m = item->data(Qt::UserRole).toMap();
    if (m.contains("url"))
    {
        QProcess *proc = new QProcess(this);
        QString lang = LanguageDialog::instance("en", "gb")->currentLanguage();
        if (lang == "gb" || lang == "us" || lang == "ko" || lang == "")
            lang = "en";
        proc->start("arora -lang "+lang+" "+m.value("url").toString());
    }
    //else
    //    qDebug() << m;
}

void MainWindow::loadOverrides(const QString &filename)
{
    if (QFile::exists(filename))
    {
        _overrides = Json::loadFromFile(filename).toMap();
    }
}


void MainWindow::OverrideJson(QVariantMap& m)
{
    QString name;
    if (m.contains("name"))
        name = m.value("name").toString();
    else if (m.contains("os_name"))
        name = m.value("os_name").toString();
    else
        return;
    if (!_overrides.contains(name))
    {
        return;
    }
    QVariantMap osMap = _overrides.value(name).toMap();
    for(QVariantMap::const_iterator iter = osMap.begin(); iter != osMap.end(); ++iter) {
        QString key = iter.key();
        QString action = key.left(1);
        if (action == "+" || action =="-")
            key = key.mid(1,-1); //Remove the action character
        else
            action = "";    //default action

        if (action=="")
        {   //Default action is to add or replace new override
            m[key] = iter.value();
        }
        else if (action=="+")
        {   //Only add if it does not already exist
            if (!m.contains(key))
            {
                m[key] = iter.value();
            }
        }
        else if (action=="-")
        {   //Remove the key if it exists
            if (!m.contains(key))
                m.remove(key);
        }
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
