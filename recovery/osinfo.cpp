#include "osinfo.h"
#include "partitioninfo.h"
#include "json.h"

OsInfo::OsInfo()
{
}

OsInfo::OsInfo(const QString &folder, const QString &flavour, QObject *parent) :
    QObject(parent), _folder(folder), _flavour(flavour)
{
    QVariantMap m = Json::loadFromFile(folder+"/os.json").toMap();
    importMap(m);

    QVariantList parts = Json::loadFromFile(folder+"/partitions.json").toMap().value("partitions").toList();
    foreach (QVariant pv, parts)
    {
        _partitions.append(new PartitionInfo(pv.toMap(), this));
    }
}

void OsInfo::importMap(QVariantMap& m)
{
    _name = m.value("name").toString();
    _version = m.value("version").toString();
    _description = m.value("description").toString();
    _releaseDate = m.value("release_date").toString();
    _bootable = m.value("bootable", true).toBool();
    _riscosOffset = m.value("riscos_offset").toInt();
    _username = m.value("username").toString();
    _password = m.value("password").toString();
}
