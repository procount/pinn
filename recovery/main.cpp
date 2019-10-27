/*
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 * Modified by @procount (c) 2019
 *
 * See LICENSE.txt for license details
 *
 */

#include "mainwindow.h"
#include "languagedialog.h"
#include "config.h"
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
#include "splash.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/reboot.h>
#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QFile>
#include <QIcon>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QTime>
#include <QLabel>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

#ifdef Q_WS_QWS
#include <QWSServer>
#endif

CecListener *cec = NULL;
CecListener *enableCEC(QObject *parent=0);
joystick *joy = NULL;
simulate *sim = NULL;

void showBootMenu(const QString &drive, const QString &defaultPartition, bool setDisplayMode)
{
    QByteArray reboot_part;
#ifdef Q_WS_QWS
    QWSServer::setBackground(BACKGROUND_COLOR);
    QWSServer::setCursorVisible(true);
#endif
    BootSelectionDialog bsd(drive, defaultPartition);
    if (setDisplayMode)
        bsd.setDisplayMode();
    bsd.exec();

    // Shut down networking
    QProcess::execute("ifdown -a");
    // Unmount file systems
    QProcess::execute("umount -ar");
    ::sync();
    // Reboot
    reboot_part = getFileContents("/run/reboot_part").trimmed();
    ::syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, reboot_part.constData());
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

    int rev = readBoardRevision();

    qDebug() << "Board revision is " << rev;

    int gpioChannel;

    if (rev == 2 || rev == 3)
        gpioChannel = 0;
    else
        gpioChannel = 2;

    QApplication a(argc, argv);
    RightButtonFilter rbf;
    LongPressHandler lph;
    GpioInput gpio(gpioChannel);

    cec = enableCEC();
    Kinput::setWindow("mainwindow");
    Kinput::setMenu("Main Menu");

    bool runinstaller = false;
    bool gpio_trigger = false;
    bool keyboard_trigger = true;
    bool force_trigger = false;
    bool wallpaper_resize = false;

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
        // Enables use of GPIO 3 to force recovery to launch by pulling low
        else if (strcmp(argv[i], "-gpiotriggerenable") == 0)
            gpio_trigger = true;
        // Disables use of keyboard to trigger recovery GUI
        else if (strcmp(argv[i], "-keyboardtriggerdisable") == 0)
            keyboard_trigger = false;
        // Forces display of recovery GUI every time
        else if (strcmp(argv[i], "-forcetrigger") == 0)
            force_trigger = true;
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
    }

    // Intercept right mouse clicks sent to the title bar
    a.installEventFilter(&rbf);

    // Treat long holds as double-clicks
    if (hasTouchScreen)
        a.installEventFilter(&lph);

#ifdef Q_WS_QWS
    QWSServer::setCursorVisible(false);
#endif

    QDir settingsdir;
    settingsdir.mkdir("/settings");

    // Set wallpaper and icon, if we have resource files for that
    if (QFile::exists(":/icons/raspberry_icon.png"))
        a.setWindowIcon(QIcon(":/icons/raspberry_icon.png"));

#ifdef Q_WS_QWS
    QWSServer::setBackground(BACKGROUND_COLOR);
#endif

    int y1=1080;
    int y2=700;
    int x2=1600;

    QPixmap pixmap;
    QString wallpaperName = ":/icons/wallpaper.jpg";

    QPixmap temp;
    temp.load(wallpaperName);
    QRect screen= a.desktop()->availableGeometry();
    QRect pixsize = temp.rect();
    QRect crop = pixsize;

    if (!wallpaper_resize)
    {   //Crop the centre of the image out to fit the screen else showMessage is off screen
        if (crop.width()>screen.width())
        {
            crop.setLeft( (crop.width() - screen.width())/2 );
            crop.setWidth( screen.width() );
        }
        if (crop.height()>screen.height())
        {
            crop.setTop( (crop.height() - screen.height())/2 );
            crop.setHeight( screen.height() );
        }
    }
    pixmap=temp.copy(crop);

    if (wallpaper_resize)
    {
        y1 = (y1 * screen.height()) / pixsize.height();
        y2 = (y2 * screen.height()) / pixsize.height();
        x2 = (x2 * screen.width()) / pixsize.width();
    }

    //QSplashScreen *splash = new QSplashScreen(QPixmap(":/wallpaper.jpg"));
    KSplash *splash = new KSplash(pixmap,0,wallpaper_resize);

    QFont splashFont;
    splashFont.setFamily("Arial");
    splashFont.setBold(true);
    splashFont.setPixelSize(24);
    splashFont.setStretch(125);
    splash->setFont(splashFont);

    QRect rect(0,0,0,0);
    rect.setSize( QApplication::desktop()->screenGeometry(-1).size() );
    splash->setMessageRect(rect, Qt::AlignCenter); // Setting the message position.

    splash->show();
    splash->resize();
    QApplication::processEvents();

    temp.load(":/icons/atari_logo1_trans.png");
    QLabel logo1(splash);
    logo1.setPixmap(temp.scaled( (temp.width() * screen.width()) / pixsize.width(), (temp.height() * screen.height()) / pixsize.height() ));
    logo1.move(0,y1);
    logo1.show();

    temp.load(":/icons/atari_mc2.png");
    QLabel logo2(splash);
    logo2.setPixmap(temp.scaled( (temp.width() * screen.width()) / pixsize.width(), (temp.height() * screen.height()) / pixsize.height() ));
    logo2.move( -3* logo2.width() ,y2);
    logo2.show();


    // Wait for drive device to show up
    QString drive;
    bool driveReady = false;
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
        QMessageBox::critical(NULL, "Files not found", QString("Cannot find the drive with installer files %1").arg(drive), QMessageBox::Close);
        return 1;
    }
    qDebug() << "INSTALL drive:" << drive;

    cec->loadMap("/mnt/cec_keys.json");
    joy->loadMap("/mnt/joy_keys.json");


    QPropertyAnimation *animation1 = new QPropertyAnimation(&logo1,"geometry");
    animation1->setDuration(2000);
    animation1->setStartValue(QRect(0, y1, logo1.width(), logo1.height()));
    animation1->setEndValue(QRect(0,0, logo1.width(), logo1.height()));
    animation1->setEasingCurve(QEasingCurve::OutQuad);

    QPropertyAnimation *animation2 = new QPropertyAnimation(&logo2,"geometry");
    animation2->setDuration(2000);
    animation2->setStartValue( QRect(0, y2, logo2.width(), logo2.height()));
    animation2->setEndValue(QRect(x2,y2, logo2.width(), logo2.height()));
    animation2->setEasingCurve(QEasingCurve::OutBounce);

    QSequentialAnimationGroup *group = new QSequentialAnimationGroup;
    group->addAnimation(animation1);
    group->addAnimation(animation2);
    group->start();

    joy->clearKeyPressed();
    cec->clearKeyPressed();

    // If -runinstaller is not specified, only continue if SHIFT is pressed, GPIO is triggered,
    // or no OS is installed (/settings/installed_os.json does not exist)
    bool bailout = !runinstaller
        && !force_trigger
        && !(gpio_trigger && (gpio.value() == 0 ))
        && hasInstalledOS(drive);

    if (bailout && keyboard_trigger)
    {
        t.start();

        while (t.elapsed() < 4000)
        {
            splash->showStatusMessage("For recovery mode, hold SHIFT...", (t.elapsed()%1000 < 500)?Qt::black : Qt::white);

            QApplication::processEvents(QEventLoop::WaitForMoreEvents, 10);
            if (QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
            {
                bailout = false;
                qDebug() << "Shift detected";
                break;
            }
            if (cec->hasKeyPressed())
            {
                bailout = false;
                qDebug() << "cec key detected";
                break;
            }
            if (joy->hasKeyPressed())
            {
                bailout = false;
                qDebug() << "joy key detected";
                break;
            }
            if (hasTouchScreen && QApplication::mouseButtons().testFlag(Qt::LeftButton))
            {
                bailout = false;
                qDebug() << "Tap detected";
                break;
            }
        }
    }

    splash->showStatusMessage("Please wait",Qt::white);
//    while (group->currentTime()<group->totalDuration())
//        QApplication::processEvents();
    splash->showStatusMessage("", Qt::black);

    cec->clearKeyPressed();
    joy->clearKeyPressed();

    if (bailout)
    {
        splash->hide();
        showBootMenu(drive, defaultPartition, true);
    }

#ifdef Q_WS_QWS
    QWSServer::setCursorVisible(true);
#endif

    // Main window in the middle of screen
    MainWindow mw(drive, defaultDisplay, splash);
    mw.setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, mw.size(), a.desktop()->availableGeometry()));
    mw.show();

#ifdef ENABLE_LANGUAGE_CHOOSER
     // Language chooser at the bottom center
    LanguageDialog* ld = new LanguageDialog(defaultLang, defaultKeyboard);
    ld->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignHCenter | Qt::AlignBottom, ld->size(), a.desktop()->availableGeometry()));
    ld->show();
#endif

    a.exec();
    showBootMenu(drive, defaultPartition, false);

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
