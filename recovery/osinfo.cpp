#include "osinfo.h"
#include "partitioninfo.h"
#include "json.h"
#include "util.h"

OsInfo::OsInfo(const QString &folder, const QString &flavour, QObject *parent) :
    QObject(parent), _folder(folder), _flavour(flavour)
{
    QVariantMap m = Json::loadFromFile(folder+"/os.json").toMap();
    _name = m.value("name").toString();
    _version = m.value("version").toString();
    _description = m.value("description").toString();
    _releaseDate = m.value("release_date").toString();
    _bootable = m.value("bootable", true).toBool();
    _riscosOffset = m.value("riscos_offset").toInt();
    _supportedModels = m.value("supported_models").toStringList();
    _username = m.value("username").toString();
    _password = m.value("password").toString();
    _url = m.value("url").toString();
    _group = m.value("group").toString();
    _replacedName = "";

    _csumType      = getCsumType(m);
    if (_csumType != "")
        _csum = getCsum(m,_csumType);


    if (m.contains("supports_backup"))
    {
        if (m.value("supports_backup","false").toString()=="update")
            _supports_backup = QString ("update");
        else if (m.value("supports_backup","false").toBool() == true)
            _supports_backup = QString("true");
        else if (m.value("supports_backup","false").toBool() == false)
            _supports_backup = QString("false");
        else
            _supports_backup = QString("");
    }
    else
        _supports_backup="";

    _use_partuuid  = m.value("use_partuuid",true).toBool();

    QVariantMap p = Json::loadFromFile(folder+"/partitions.json").toMap();
    QVariantList parts = p.value("partitions").toList();
    _configpath = p.value("configpath").toString();
    foreach (QVariant pv, parts)
    {
        _partitions.append(new PartitionInfo(pv.toMap(), this));
    }
}

