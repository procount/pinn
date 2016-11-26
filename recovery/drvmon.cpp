#include "drvmon.h"

#include <QTimer>
#include <QDebug>

#include <QDir>
#include <QDialog>
#include <QList>
#include <QStringList>

drvmon::drvmon(QObject *parent) :
    QObject(parent)
{
    //Add a min period of 5 secs
    _timer = new QTimer(this);
    connect(_timer,SIGNAL(timeout()),this,SLOT(checkDrives()));
}

drvmon::~drvmon()
{
    _timer->stop();
    _periodList.clear();
    _driveList.clear();
}

void drvmon::calcMinPeriod(void)
{
    int m=5000;
    foreach (int v, _periodList)
    {
        if (v < m)
            m = v;
    }
    _shortestPeriodms = m;
}


void drvmon::startMonitoringDrives(int period)
{
    //period indicates the max period in ms for a timer.
    //This class remembers all requested periods and starts a timer with the shortest period
    _periodList.append(period);
   calcMinPeriod();
    _timer->start(_shortestPeriodms);
}

void drvmon::stopMonitoringDrives(int period)
{
    //Stops the timer and removes the requested period from the list of requested periods
    //If there are still some requested periods left, the timer is restarted with the shortest period
    //The period used to start the timer should be used to stop the timer. otherwise periods could be left
    _timer->stop();
    int index = _periodList.indexOf(period);
    if (index != -1)
        _periodList.removeAt(index);
    if (_periodList.count())
    {
       calcMinPeriod();
       _timer->start(_shortestPeriodms);
    }
}

/* Monitor drives for changes (drvmon sticks) */
void drvmon::checkDrives(void)
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
    newlist.sort();	//sometimes they are presented in a different order
    if (newlist != _driveList)
    {
	//qDebug() << "New Drive list" <<newlist;
        _driveList = newlist;
        emit drivesChanged();
    }
    pclose(fp);
}

