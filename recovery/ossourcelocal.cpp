#include "ossourcelocal.h"
#include "config.h"
#include "json.h"
#include "osinfo.h"

#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QProcess>

OsSourceLocal::OsSourceLocal(QObject *parent) :
    OsSource(parent)
{
}


void OsSourceLocal::monitorDevice()
{
    static int counter=0;
    counter +=1;
    qDebug() << "OsSourceLocal::monitorDevices";

    qDebug() << "Checking "<< device<< " "<<counter;
    if (QFile::exists(device))
    {
        qDebug() << device << "detected after "<<counter;
        counter=0;
        QDir dir;
        dir.mkpath(location);
        QString cmd = "mount -o ro -t vfat " + device + " " + location;
        if (QProcess::execute(cmd) == 0)
        {
            //_hasUSB = true;
            qDebug() << device <<" mounted at " << location;
        }
        readImages();
        emit newSource(this);
        //set timer to check for device gone?
    }
    else
    {
        QTimer::singleShot(1000, this, SLOT(monitorDevice()));
    }
}


void OsSourceLocal::readImages()
{
    qDebug() << "OsSourceLocal::readImages";
    QString path=location+"/os";
    QDir dir(path, "", QDir::Name, QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList list = dir.entryList();

    foreach (QString image,list)
    {
        QString imagefolder = path+"/"+image;
        if (!QFile::exists(imagefolder+"/os.json"))
            continue;
        QVariantMap osv = Json::loadFromFile(imagefolder+"/os.json").toMap();
        osv["source"] = source;

        QString basename = osv.value("name").toString();

        //if (canInstallOs(basename, osv))
        //We will list ALL images that are available and filter them later
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
                        OsInfo * newOs = new OsInfo();
                        newOs->importMap(fm);

                        QVariantList parts = Json::loadFromFile(imagefolder+"/partitions.json").toMap().value("partitions").toList();
                        newOs->importParts(parts);
                        oses[name] = newOs;
                    }
                }
            }
            else
            {
                QString name = basename;
                if (name == RECOMMENDED_IMAGE)
                    osv["recommended"] = true;
                osv["folder"] = imagefolder;

                OsInfo * newOs = new OsInfo();
                newOs->importMap(osv);
                QVariantList parts = Json::loadFromFile(imagefolder+"/partitions.json").toMap().value("partitions").toList();
                newOs->importParts(parts);
                oses[name] = newOs;
            }
        }
    }
}
