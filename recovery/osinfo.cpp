#include "osinfo.h"
#include "partitioninfo.h"
#include "json.h"
#include "util.h"
#include "config.h"
#include <QDebug>

bool OsInfo::_showAll=false;

OsInfo::OsInfo()
{
    _folder="";
    _flavour="";
    _name="";
    _description="";
    _version="";
    _releaseDate="";
    _username="";
    _password="";
    _source="";
    _icon="";
    _osInfo="";
    _partitionsInfo="";
    _partitionSetup="";
    _marketingInfo="";
    _url="";
    _bootable=true;
    _recommended=false;
    _riscosOffset=0;
    _nominalSize=0;
    //QList<PartitionInfo *> _partitions;
    //QStringList _models;
}

OsInfo::OsInfo(const QString &folder, const QString &source, const QString &flavour, QObject *parent) :
    QObject(parent), _folder(folder), _flavour(flavour)
{
    _source = source;

    QVariantMap m = Json::loadFromFile(folder+"/os.json").toMap();
    importMap(m);

    QVariantList parts = Json::loadFromFile(folder+"/partitions.json").toMap().value("partitions").toList();
    importParts(parts);
}

void OsInfo::importMap(QVariantMap& m)
{
    for(QVariantMap::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        QString key = iter.key();
        if ((key == "name") || (key=="os_name"))
            _name = iter.value().toString();
        else if (key == "version")
            _version = iter.value().toString();
        else if (key == "description")
            _description = iter.value().toString();
        else if (key == "release_date")
            _releaseDate = iter.value().toString();
        else if (key == "bootable")
            _bootable = iter.value().toBool();
        else if (key == "riscos_offset")
            _riscosOffset = iter.value().toInt();
        else if (key == "username")
            _username = iter.value().toString();
        else if (key == "password")
            _password = iter.value().toString();
        else if (key == "icon")
            _icon = iter.value().toString();
        else if (key == "marketing_info")
            _marketingInfo = iter.value().toString();
        else if (key == "nominal_size")
            _nominalSize = iter.value().toInt();
        else if (key == "os_info")
            _osInfo = iter.value().toString();
        else if (key == "partition_setup")
            _partitionSetup = iter.value().toString();
        else if (key == "partitions_info")
            _partitionsInfo = iter.value().toString();
        else if (key == "folder")
            _folder = iter.value().toString();
        else if (key == "url")
            _url = iter.value().toString();
        else if (key == "source")
            _source = iter.value().toString();
        else if (key == "recommended")
            _recommended = iter.value().toBool();
        else if ((key == "kernel") || (key == "feature_level") || (key == "supported_hex_revisions") || (key=="slice_exclude"))
            ;
        else if (key == "supported_models")
        {
            _models = iter.value().toStringList();
        }
        else if (key=="tarballs")
        {
            //@@ Maybe check if the partitions already exist and set them, or create new.
            _partitions.clear();
            QStringList pvl = iter.value().toStringList();
            foreach (QString tarball, pvl)
            {
                PartitionInfo *part = new PartitionInfo();
                part->setTarball(tarball);
                _partitions.append(part);
            }
        }
        else
            qDebug() << "Unknown: " << key << iter.value();
    }
}

void OsInfo::importParts(QVariantList& parts)
{
    foreach (QVariant pv, parts)
    {
        _partitions.append(new PartitionInfo(pv.toMap(), this));
    }

}

void OsInfo::print()
{
    qDebug() << "{";
    qDebug() << "_name=" <<_name;
    qDebug() << "_version=" <<_version;
    qDebug() << "_description=" <<_description;
    qDebug() << "_releaseDate=" <<_releaseDate;
    qDebug() << "_bootable=" <<_bootable;
    qDebug() << "_riscosOffset=" <<_riscosOffset;
    qDebug() << "_username=" <<_username;
    qDebug() << "_password=" <<_password;
    qDebug() << "_source=" <<_source;
    qDebug() << "_folder=" << _folder;
    qDebug() << "_flavour=" << _flavour;
    qDebug() << "_icon=" << _icon;
    qDebug() << "_osinfo=" << _osInfo;
    qDebug() << "_partitionsInfo=" << _partitionsInfo;
    qDebug() << "_partitionSetup=" << _partitionSetup;
    qDebug() << "_marketingInfo=" << _marketingInfo;
    qDebug() << "_url=" << _url;
    qDebug() << "_recommended=" << _recommended;
    qDebug() << "_showAll=" << _showAll;
    qDebug() << "_nominalSize=" << _nominalSize;
    qDebug() << "_models=" <<_models;
    qDebug() << "_source=" <<_source;
    foreach (PartitionInfo * p, _partitions)
        p->print();
    qDebug() << "}";
}

/* Whether this OS should be displayed in the list of bootable OSes */
bool OsInfo::canBootOs()
{
    /* Can't simply pull "name" from "values" because in some JSON files it's "os_name" and in others it's "name" */

    if (!_bootable)
    {
        //qDebug() <<"Not bootable";
        return false;
    }

    /* Data Partition isn't bootable */
    if (_name == "Data Partition")
    {
        //qDebug() <<"Data";
        return false;
    }

    return true;
}

/* Whether this OS should be displayed in the list of installable OSes */
bool OsInfo::canInstallOs()
{
    /* Can't simply pull "name" from "values" because in some JSON files it's "os_name" and in others it's "name" */

    /* If it's not bootable, it isn't really an OS, so is always installable */
    if (!canBootOs())
    {
        //qDebug()<<"Installable";
        return true;
    }

    /* RISC_OS needs a matching riscos_offset */
    if (nameMatchesRiscOS(_name))
    {
        if (_riscosOffset != RISCOS_OFFSET)
        {
            return false;
        }
    }

    /* Display OS in list if it is supported or "showall" is specified in recovery.cmdline */
    if (_showAll)
    {
        //qDebug()<<"SHowall:installable";
        return true;
    }
    else
    {
        return isSupportedOs();
    }
}

/* Whether this OS is supported */
bool OsInfo::isSupportedOs()
{
    /* Can't simply pull "name" from "values" because in some JSON files it's "os_name" and in others it's "name" */
    /* If it's not bootable, it isn't really an OS, so is always supported */
    if (!canBootOs())
    {
        //qDebug() << "Supported";
        return true;
    }

    QString model = getFileContents("/proc/device-tree/model");

    //qDebug() << "Checking " << _name << ": " << model << ": " << _models;
    if (!_models.isEmpty())
    {
        foreach (QString m, _models)
        {
            /* Check if the full formal model name (e.g. "Raspberry Pi 2 Model B Rev 1.1")
             * contains the string we are told to look for (e.g. "Pi 2") */
            if (model.contains(m, Qt::CaseInsensitive))
            {
                //qDebug() <<"found";
                return true;
            }
        }
        return false;
    }

    return true;
}
