#include "usb.h"

#include <QTimer>
#include <QDebug>

#include <QDir>
#include <QDialog>
#include <QStringList>

usb::usb(QObject *parent) :
    QObject(parent)
{
    _timer = new QTimer(this);
    connect(_timer,SIGNAL(timeout()),this,SLOT(checkDrives()));
}

void usb::startMonitoringDrives(void)
{
    _timer->start(2000);
}

void usb::stopMonitoringDrives(void)
{
    _timer->stop();
}

/* Monitor drives for changes (USB sticks) */
void usb::checkDrives(void)
{
    QStringList newlist;
    char device[32];

    //Get list of all block devices starting SD or MMCBLK
    //FILE *fp = popen ("blkid | cut -d: -f 1", "r");
    FILE *fp = popen ("parted -l | grep \"^Disk /dev/\" | cut -d ' ' -f 2 | cut -d ':' -f 1", "r");
    if (fp != NULL)
    {
        while (1)
        {
            if (fgets (device, sizeof (device) - 1, fp) == NULL)
                break;
            //qDebug() << device;
            if (!strncmp (device + 5, "sd", 2) || !strncmp (device + 5, "mmcblk", 6) )
            {   //remove trailing LF
                device [strlen(device)-1] ='\0';
                newlist << device;
            }
        }
    }
    newlist.sort();
    if (newlist != _driveList)
    {
        qDebug() << "New Drive list" <<newlist;
        _driveList = newlist;
        emit drivesChanged();
    }
    pclose(fp);
}

