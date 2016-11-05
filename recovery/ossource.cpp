#include "ossource.h"
#include "config.h"
#include "json.h"
#include "osinfo.h"
#include "mydebug.h"

#include <QThread>
#include <QTimer>
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
    //qDebug() << "OsSource::monitorDevice";
}

void OsSource::monitorNetwork(QNetworkAccessManager *netaccess)
{
    //qDebug() << "OsSource::monitorNetwork" << location;
}

void OsSource::clearOSes()
{
    oses.clear();
}

// Add/replace an OsInfo to OsSource::oses if newer
void OsSource::addOS(OsInfo *os, const QString source)
{
    MYDEBUG
    QMap<QString,OsInfo *>::Iterator i = oses.find(os->name());
    if (i != oses.end())
    {   //Already exists
        //Check if new OS is newer
        if ( (os->releaseDate() > i.value()->releaseDate()) || source == SOURCE_INSTALLED_OS)
        {
            //replace with Newer OS.
            i.value() = os;
            //##DBG("Adding newer os: " << os->name());
        }
    }
    else
    {   //New OS
        //##DBG("Adding NEW os: " << os->name());
        oses[os->name()] = os;
    }
}



void OsSource::filterAddSource(OsSource *src)
{
    MYDEBUG
    QMap<QString,OsInfo *>::Iterator i;
    for (i=src->oses.begin(); i!=src->oses.end(); i++)
    {
        OsInfo * pOs = i.value();
        if (pOs->canInstallOs())
        {
            //##DBG("Adding " << pOs->name() << " from " << src->getDevice());
            addOS(pOs, pOs->source());
        }
    }
}


OsInfo * OsSource::findOs(QString name)
{
    MYDEBUG
    QMap<QString,OsInfo *>::Iterator i = oses.find(name);
    if (i != oses.end())
        return (i.value());
    else
        return(NULL);
}
