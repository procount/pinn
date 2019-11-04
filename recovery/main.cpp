#include "config.h"
#include "languagedialog.h"
#include "mainwindow.h"
#include "keydetection.h"
#include "gpioinput.h"
#include "rightbuttonfilter.h"
#include "longpresshandler.h"
#include "json.h"
#include "util.h"
#include "bootselectiondialog.h"
#include "ceclistener.h"
#include "joystick.h"
#include "simulate.h"

#define DBG_LOCAL 1
#define LOCAL_DO_DBG 0
//#define LOCAL_DBG_FUNC 0
//#define LOCAL_DBG_OUT 0
//#define LOCAL_DBG_MSG 0
#include "mydebug.h"

#include "splash.h"

#include <stdio.h>
#include <unistd.h>
#include <QApplication>
#include <QBitmap>
#include <QStyle>
#include <QDesktopWidget>
//#include <QSplashScreen>
#include <QFile>
#include <QIcon>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QTime>
#ifdef Q_WS_QWS
#include <QWSServer>
#endif

/*
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

bool g_nofirmware=false;
bool dsi=false;
CecListener *cec = NULL;
CecListener *enableCEC(QObject *parent=0);
joystick *joy = NULL;
simulate *sim = NULL;

QStringList downloadRepoUrls;
QString repoList;

QString stylesheet = "";
QColor backgroundColour = BACKGROUND_COLOR;

bool timedReboot=false;

MainWindow * gMW=NULL;

void runCustomScript(const QString &driveDev, int partNr, const QString &cmd, bool inBackground=false )
{
    bool mntStillMounted = true ; // suppose yes.

    // check if /mnt is still mounted to the recovery partition.
    if (QFile::exists("/mnt/recovery.rfs"))
    {
        mntStillMounted = true ;
    }
    else
    {   // mount it.
        if (QProcess::execute("mount -t vfat -o ro "+partdev(driveDev, partNr)+" /mnt") == 0)
        {
            mntStillMounted = false ;
        }
    }

    // execute the cmd script.
    if (inBackground)
    {
        if (QProcess::startDetached("/mnt/"+cmd) != 0)
        {
            // not fatal if does not work
        }
    }
    else
    {
        if (QProcess::execute("/mnt/"+cmd) != 0)
        {
            // not fatal if does not work
        }
    }

    // clean exit.
    if (! mntStillMounted)
    {
        QProcess::execute("umount /mnt");
    }
}

void showBootMenu(const QString &drive, const QString &defaultPartition, bool setDisplayMode)
{
#ifdef Q_WS_QWS
    QWSServer::setBackground(backgroundColour);
    QWSServer::setCursorVisible(true);
#endif
    //Just reuse setDisplayMode as indicator for sticky boot direct mode
    BootSelectionDialog bsd(drive, defaultPartition, setDisplayMode, dsi);
    bsd.setStyleSheet(stylesheet);
    if (setDisplayMode)
        bsd.setDisplayMode();
    bsd.exec();


    // Shut down networking
    QProcess::execute("ifdown -a");
    // Unmount file systems
    QProcess::execute("umount -ar");
    // Reboot
    reboot();
}

bool hasInstalledOS(const QString &drive)
{
    bool installedOsFileExists = false;

    if (QProcess::execute("mount -o ro "+partdev(drive, SETTINGS_PARTNR)+" /settings") == 0)
    {
        installedOsFileExists = QFile::exists("/settings/installed_os.json");
        QProcess::execute("umount /settings");
    }

    return installedOsFileExists;
}

QString findRecoveryDrive()
{
    /* Search for drive with recovery.rfs */
    QString drive;
    QString dirname  = "/sys/class/block";
    QDir    dir(dirname);
    QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (QString devname, list)
    {
        /* Only search first partition and partitionless devices. Skip virtual devices (such as ramdisk) */
        if ((devname.right(1).at(0).isDigit() && !devname.endsWith("1"))
                || QFile::symLinkTarget("/sys/class/block/"+devname).contains("/devices/virtual/"))
            continue;

        if (QProcess::execute("mount -t vfat -o ro /dev/"+devname+" /mnt") == 0)
        {

            if (QFile::exists("/mnt/recovery.rfs"))
            {
                qDebug() << "Found recovery.rfs at" << devname;

                // We are interested in the drive, not the exact partition
                drive = "/dev/"+devname;
                if (drive.endsWith("p1"))
                    drive.chop(2);
                else if (drive.endsWith("1"))
                    drive.chop(1);
            }

            QProcess::execute("umount /mnt");
        }

        if (!drive.isEmpty())
            break;
    }

    return drive;
}



int main(int argc, char *argv[])
{
    bool hasTouchScreen = QFile::exists("/sys/devices/platform/rpi_ft5406");

    // Unless we have a touch screen, wait for keyboard to appear before displaying anything
    if (!hasTouchScreen)
        KeyDetection::waitForKeyboard();

    qDebug() << VERSION_NUMBER;

    int rev = readBoardRevision();
    qDebug() << "Board revision is " << rev;

    int gpioChannel;
    int gpioChannelValue = 0;

    if (rev == 2 || rev == 3)
        gpioChannel = 0;
    else
        gpioChannel = 2;

    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    RightButtonFilter rbf;
    LongPressHandler lph;
    GpioInput *gpio=NULL;
    cec = enableCEC();

    QString drive;
    bool driveReady = false;

    bool runinstaller = false;
    bool keyboard_trigger = true;
    bool force_trigger = false;
    bool noobsconfig = true;
    bool use_default_source = true;
    bool wallpaper_resize = false;
    bool gpio_trigger = false;

    QString defaultLang = "en";
    QString defaultKeyboard = "gb";
    QString defaultDisplay = "0";
    QString defaultPartition = "800";

    // Process command-line arguments
    for (int i=1; i<argc; i++)
    {
        // Flag to indicate first boot
        if (strcmp(argv[i], "-runinstaller") == 0)
            runinstaller = true;
        if (strcmp(argv[i], "-wallpaper_resize") == 0)
            wallpaper_resize = true;
        // Enables use of GPIO 3 to force NOOBS to launch by pulling low
        else if (strcmp(argv[i], "-gpiotriggerenable") == 0)
            gpio_trigger=true;
        // Disables use of keyboard to trigger recovery GUI
        else if (strcmp(argv[i], "-keyboardtriggerdisable") == 0)
            keyboard_trigger = false;
        // Forces display of recovery GUI every time
        else if (strcmp(argv[i], "-forcetrigger") == 0)
            force_trigger = true;
        // Force recovery to do noobsconfig
        else if (strcmp(argv[i], "-noconfig") == 0)
            noobsconfig = false;
        // Force dsi switching
        else if (strcmp(argv[i], "-dsi")==0)
            dsi = true;
        // Allow default language to be specified in commandline
        else if (strcmp(argv[i], "-lang") == 0)
        {
            if (argc > i+1)
                defaultLang = argv[i+1];
        }
        // Allow default keyboard layout to be specified in commandline
        else if (strcmp(argv[i], "-kbdlayout") == 0)
        {
            if (argc > i+1)
                defaultKeyboard = argv[i+1];
        }
        // Allow default display mode to be specified in commandline
        else if (strcmp(argv[i], "-dispmode") == 0)
        {
            if (argc > i+1)
                defaultDisplay = --argv[i+1];
        }
        // Allow default boot partition to be specified in commandline
        else if (strcmp(argv[i], "-partition") == 0)
        {
            if (argc > i+1)
                defaultPartition = argv[i+1];
        }
        else if (strcmp(argv[i], "-pinndrive") == 0)
        {
            if (argc > i+1)
                drive = argv[i+1];
        }
        // Allow default repos to be specified in commandline
        else if (strcmp(argv[i], "-no_default_source") == 0)
        {
             use_default_source = false;
        }
        // Allow Extra repos to be specified in commandline
        else if (strcmp(argv[i], "-alt_image_source") == 0)
        {
             //This could append multiple URLs now
             if (argc > i+1)
             {
                 QString url(argv[i+1]);
                 if (url.startsWith("http://"))
                    downloadRepoUrls << url;
             }
        }
        else if (strcmp(argv[i], "-repo_list")==0)
        {
            if (argc > i+1)
            {
                  repoList = argv[i+1];
            }
        }
        // Allow gpio channel to be specified in commandline
        else if (strcmp(argv[i], "-gpiochannel") == 0)
        {
          if (argc > i+1)
              gpioChannel = atoi(argv[i+1]);
        }
        // Allow gpio channel value i.e pull up or pull down to be specified in commandline
        else if (strcmp(argv[i], "-gpiochannelvalue") == 0)
        {
          if (argc > i+1)
              gpioChannelValue = atoi(argv[i+1]);
        }
    }

    if (gpio_trigger)
        gpio = new GpioInput(gpioChannel);


    //==========================
    // Wait for drive device to show up
    QTime t;
    t.start();

    while (t.elapsed() < 10000)
    {
        if (drive.isEmpty())
        {
            /* We do not know the exact drive name to wait for */
            drive = findRecoveryDrive();
            if (!drive.isEmpty())
            {
                driveReady = true;
                break;
            }
        }
        else if (drive.startsWith("/dev"))
        {
            if (QFile::exists(drive))
            {
                driveReady = true;
                break;
            }
        }

        QApplication::processEvents(QEventLoop::WaitForMoreEvents, 100);
    }
    if (!driveReady)
    {
        QMessageBox::critical(NULL, "Files not found", QString("Cannot find the drive with PINN files %1").arg(drive), QMessageBox::Close);
        return 1;
    }
    qDebug() << "PINN drive:" << drive;

    QProcess::execute("mount -o ro -t vfat "+partdev(drive, 1)+" /mnt");
    cec->loadMap("/mnt/cec_keys.json");
    joy->loadMap("/mnt/joy_keys.json");

#if 0
    qDebug() << "Starting dbus";
    QProcess::execute("/etc/init.d/S30dbus start");
    QProcess *proc = new QProcess();
    qDebug() << "Starting dhcpcd from main";
    proc->start("/sbin/dhcpcd --noarp -f /settings/dhcpcd.conf -e wpa_supplicant_conf=/settings/wpa_supplicant.conf --denyinterfaces \"*_ap\"");
#endif

    // do some stuff at start in background.
    runCustomScript(drive, 1,"background.sh", true);

    if (use_default_source)
    {
        QStringList urls = QString(DEFAULT_REPO_SERVER).split(' ', QString::SkipEmptyParts);
        foreach (QString url, urls)
        {
            downloadRepoUrls << url;
        }
    }

    // Intercept right mouse clicks sent to the title bar
    a.installEventFilter(&rbf);

    // Treat long holds as double-clicks
    if (hasTouchScreen)
        a.installEventFilter(&lph);

    QDir settingsdir;
    settingsdir.mkdir("/settings");

    // Set wallpaper and icon, if we have resource files for that
    if (QFile::exists(":/icons/raspberry_icon.png"))
        a.setWindowIcon(QIcon(":/icons/raspberry_icon.png"));

    int r,g,b;
    int newBGnd;

    QPixmap pixmap;
    QString wallpaperName = "/mnt/wallpaper.jpg";
    if (!QFile::exists(wallpaperName))
        wallpaperName = "/mnt/wallpaper.png";

    if (QFile::exists(wallpaperName))
    {
        QPixmap temp;
        temp.load(wallpaperName);
        QRect screen= a.desktop()->availableGeometry();
        QRect area = temp.rect();
        if (!wallpaper_resize)
        {   //Crop the centre of the image out to fit the screen else showMessage is off screen
            if (area.width()>screen.width())
            {
                area.setLeft( (area.width() - screen.width())/2 );
                area.setWidth( screen.width() );
            }
            if (area.height()>screen.height())
            {
                area.setTop( (area.height() - screen.height())/2 );
                area.setHeight( screen.height() );
            }
        }
        pixmap=temp.copy(area);
    }
    else
    {
        pixmap.load(":/wallpaper.png");
        wallpaper_resize = false; //We don't want the standard logo resized - it looks really bad
    }
    QString cmdline = getFileContents("/proc/cmdline");
    QStringList args = cmdline.split(QChar(' '),QString::SkipEmptyParts);
    foreach (QString s, args)
    {
        if (s.contains("background"))
        {
            QStringList params = s.split(QChar('='));
            QString arg = params.at(1);
            QStringList colours = arg.split(QChar(','));
            r=colours.at(0).toInt();
            g=colours.at(1).toInt();
            b=colours.at(2).toInt();
            backgroundColour = QColor(r,g,b);
            newBGnd = qRgba(r,g,b,0xff);
            r=qMin(r+20,255);
            g=qMin(g+20,255);
            b=qMin(b+20,255);

            stylesheet = "* {background: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+"); }";
            a.setStyleSheet(stylesheet);

            {   //Change background colour of splash wallpaper
                QImage tmp = pixmap.toImage();

                // Loop all the pixels
                for(int y = 0; y < tmp.height(); y++)
                {
                  for(int x= 0; x < tmp.width(); x++)
                  {
                    if (tmp.pixel(x,y) == qRgba(0xde,0xdf,0xde,0xff))
                    {
                        // Apply the pixel color
                        tmp.setPixel(x,y,newBGnd);
                    }
                  }
                }
                // Get the coloured pixmap
                pixmap = QPixmap::fromImage(tmp);
            }
        }
        if (s.contains("nofirmware"))
        {
            g_nofirmware = true;
        }
    }
#ifdef Q_WS_QWS
    QWSServer::setBackground(backgroundColour);
#endif

    KSplash *splash = new KSplash(pixmap,0,wallpaper_resize);

    splash->show();
    splash->resize();
    splash->showMessage("For recovery mode, hold SHIFT...",Qt::black);
    QApplication::processEvents();

#ifdef Q_WS_QWS
    QWSServer::setCursorVisible(false);
#endif

    //------------------------------------------------

    // If -runinstaller is not specified, only continue if SHIFT is pressed, GPIO is triggered,
    // or no OS is installed (/settings/installed_os.json does not exist)
    bool bailout = !runinstaller
        && !force_trigger
        && !(gpio && (gpio->value() == gpioChannelValue ))
        && hasInstalledOS(drive);

    if (bailout && keyboard_trigger)
    {
        t.start();
        while (t.elapsed() < 3000)
        {
            splash->showMessage("For recovery mode, hold SHIFT...", Qt::AlignLeft, (t.elapsed()%1000 < 500)?Qt::black : Qt::white);
            QApplication::processEvents(QEventLoop::WaitForMoreEvents, 10);
            if (QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
            {
                bailout = false;
                qDebug() << "Shift detected";
                break;
            }
            if (QApplication::mouseButtons().testFlag(Qt::LeftButton))
            {
                bailout = false;
                qDebug() << "Tap detected";
                break;
            }
            if (cec->hasKeyPressed())
            {
                bailout = false;
                qDebug() << "cec key detected";
                break;
            }
        }
    }

    splash->clearMessage();

    QProcess::execute("umount /mnt");   //restore mounted behaviour

    cec->clearKeyPressed();

    if (bailout)
    {
        showBootMenu(drive, defaultPartition, true);
    }

#ifdef Q_WS_QWS
    QWSServer::setCursorVisible(true);
#endif

    // Main window in the middle of screen
    MainWindow mw(drive, defaultDisplay, splash, noobsconfig);
    gMW = &mw;  //Make it available globally.

    mw.setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, mw.size(), a.desktop()->availableGeometry()));
    mw.show();

#ifdef ENABLE_LANGUAGE_CHOOSER
     // Language chooser at the bottom center
    LanguageDialog* ld = new LanguageDialog(defaultLang, defaultKeyboard);
    ld->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignHCenter | Qt::AlignBottom, ld->size(), a.desktop()->availableGeometry()));
    ld->show();
#endif

    int result = a.exec();
    qDebug() << "Application Exit " << result;
    showBootMenu(drive, defaultPartition, timedReboot);

    return 0;
}

CecListener *enableCEC(QObject *parent)
{
    QFile f("/proc/cpuinfo");
    f.open(f.ReadOnly);
    QByteArray cpuinfo = f.readAll();
    f.close();

    if (cpuinfo.contains("BCM2708") || cpuinfo.contains("BCM2709") || cpuinfo.contains("BCM2835")) /* Only supported on the Raspberry for now */
    {
        cec = new CecListener(parent);
        cec->start();
    }

    joy = new joystick(parent);
    joy->start();

    sim = new simulate();
    return(cec);
}
