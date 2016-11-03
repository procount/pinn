#include "ossource.h"
#include "config.h"
#include "json.h"
#include "osinfo.h"

#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QMap>
#include <QMapIterator>

OsSource::OsSource(QObject *parent) :
    QObject(parent)
{
}

void OsSource::setSourceType(const char * type)
{
    sourceType = type;
}

QString OsSource::getSourceType()
{
    return(sourceType);
}

void OsSource::setDevice(const char * id)
{
    device = id;
}

QString OsSource::getDevice()
{
    return(device);
}

void OsSource::setLocation(const char * locn)
{
    location = locn;
}

QString OsSource::getLocation()
{
    return(location);
}

void OsSource::monitorDevice()
{
    qDebug() << "OsSource::monitorDevice";
}

void OsSource::monitorNetwork(QNetworkAccessManager *netaccess)
{
    qDebug() << "OsSource::monitorNetwork" << location;
}

void OsSource::clearOSes()
{
    oses.clear();
}

void OsSource::addOS(OsInfo *os, const QString source)
{
    QMap<QString,OsInfo *>::Iterator i = oses.find(os->name());
    if (i != oses.end())
    {   //Already exists
        //Check if new OS is newer
        if ( (os->releaseDate() > i.value()->releaseDate()) || source == SOURCE_INSTALLED_OS)
            //replace with Newer OS.
            i.value() = os;
    }
    else
    {   //New OS
        oses[os->name()] = os;
    }

}
