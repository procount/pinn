#include "usb.h"

#include <QTimer>
#include <QDebug>

#include <QDir>
#include <QDialog>


usb::usb(QObject *parent) :
    QObject(parent)
{
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(checkDrives()));
}

void usb::startMonitoringDrives(void)
{
    timer->start(2000);
}

void usb::stopMonitoringDrives(void)
{
    timer->stop();
}

/* Monitor drives for changes (USB sticks) */
void usb::checkDrives(void)
{
    static int lastNumDrives=0;

    char device[32];
    qDebug() << "Check:";
    FILE *fp = popen ("parted -l | grep \"^Disk /dev/\" | cut -d ' ' -f 2 | cut -d ':' -f 1", "r");
    int n=0;
    if (fp != NULL)
    {
        while (1)
        {
            if (fgets (device, sizeof (device) - 1, fp) == NULL)
                break;
            qDebug() << device;
            if (!strncmp (device + 5, "sd", 2) || !strncmp (device + 5, "mmcblk1", 7) )
            {
                n=n+1;
            }
        }
    }
    if (n != lastNumDrives)
    {
        qDebug() << "New Drive list";
        lastNumDrives = n;
        emit drivesChanged();
    }
    pclose(fp);
}
