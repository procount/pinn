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

OsSource::OsSource(QObject *parent) :
    QObject(parent)
{
}

void OsSource::setSource(const char * type)
{
    source = type;
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

void OsSource::monitorDevice()
{
    qDebug() << "OsSource::monitorDevice";
}

void OsSource::monitorNetwork(QNetworkAccessManager *netaccess)
{
    qDebug() << "OsSource::monitorNetwork";
}
