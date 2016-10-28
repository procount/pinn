#include "ossource.h"
#include "config.h"
#include "json.h"

#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QProcess>

OSSource::OSSource(QObject *parent) :
    QObject(parent)
{
}

void OSSource::setSource(const char * type)
{
    source = type;
}

void OSSource::setDevice(const char * id)
{
    device = id;
}

void OSSource::setLocation(const char * locn)
{
    location = locn;
}

void OSSource::checkDeviceExists()
{
    static int counter=0;
    counter +=1;
    qDebug() << "Checking "<< device<< " "<<counter;
    if (QFile::exists(device))
    {
        qDebug() << device << "detected after "<<counter;
        counter=0;
        QDir dir;
        dir.mkdir(location);
        QString cmd = "mount -o ro -t vfat " + device + " " + location;
        if (QProcess::execute(cmd) == 0)
        {
            //_hasUSB = true;
            qDebug() << device <<" mounted at " << location;
        }
        listImagesInDir();
        //set timer to check for device gone?
    }
    else
    {
        QTimer::singleShot(1000, this, SLOT(checkDeviceExists()));
    }
}


void OSSource::listImagesInDir()
{
    QDir dir(location, "", QDir::Name, QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList list = dir.entryList();

    foreach (QString image,list)
    {
        QString imagefolder = location+"/"+image;
        if (!QFile::exists(imagefolder+"/os.json"))
            continue;
        QVariantMap osv = Json::loadFromFile(imagefolder+"/os.json").toMap();
        osv["source"] = source;

        QString basename = osv.value("name").toString();

        //if (canInstallOs(basename, osv))
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
}
