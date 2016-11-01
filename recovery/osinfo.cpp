#include "osinfo.h"
#include "partitioninfo.h"
#include "json.h"
#include <QDebug>

OsInfo::OsInfo()
{
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
    //From local
    _name = m.value("name").toString();
    _version = m.value("version").toString();
    _description = m.value("description").toString();
    _releaseDate = m.value("release_date").toString();
    _bootable = m.value("bootable", true).toBool();
    _riscosOffset = m.value("riscos_offset").toInt();
    _username = m.value("username").toString();
    _password = m.value("password").toString();
    _source = m.value("source").toString();


    qDebug() << m.value("partitions").toList();
    qDebug() << m.value("partitions").toMap();


    if (m.contains("partitions"))
    {
        //QVariantMap pv = m.value("partitions").toMap();
        QVariantList pvl = m.value("partitions").toList();
        importParts(pvl);
    }
/*
            "os_name": "Arch1",
            "description": "An Arch Linux port for ARM devices RPi1",
            "release_date": "2016-03-17",
            "feature_level": 35644412,
            "supported_hex_revisions": "2,3,4,5,6,7,8,9,d,e,f,10,11,12,13,14,19,0092",
            "supported_models": [
               "Pi Model",
               "Pi Compute Module",
               "Pi Zero"
            ],
            "os_info": "https://raw.githubusercontent.com/procount/pinn-os/master/os/Arch1/os.json",
            "partitions_info": "https://raw.githubusercontent.com/procount/pinn-os/master/os/Arch1/partitions.json",
            "icon": "https://raw.githubusercontent.com/procount/pinn-os/master/os/Arch1/Arch.png",
            "marketing_info": "https://raw.githubusercontent.com/procount/pinn-os/master/os/Arch1/marketing.tar",
            "partition_setup": "https://raw.githubusercontent.com/procount/pinn-os/master/os/Arch1/partition_setup.sh",
            "nominal_size": 1600,
            "tarballs": [
                "https://raw.githubusercontent.com/procount/pinn-os/master/os/Arch1/boot.tar.xz",
                "http://archlinuxarm.org/os/ArchLinuxARM-rpi-latest.tar.gz"
            ]
        },

 */
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
    foreach (PartitionInfo * p, _partitions)
        p->print();
    qDebug() << "}";
}
