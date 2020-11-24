#include "osinfo.h"
#include "partitioninfo.h"
#include "json.h"
#include <QDebug>
#include "util.h"

OsInfo::OsInfo(const QString &folder, const QString &flavour, QObject *parent) :
    QObject(parent), _folder(folder), _flavour(flavour)
{
    QVariantMap m = Json::loadFromFile(folder+"/os.json").toMap();

    //Although this was already done when loading the initial lists, we're reading them from disk again, so repeating it.
    OverrideJson(m);

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
    _supports_usb_boot = m.value("supports_usb_boot",true).toBool();
    _supports_usb_root = m.value("supports_usb_root",true).toBool();
    _supports_sda_boot = m.value("supports_sda_boot",true).toBool();
    _supports_sda_root = m.value("supports_sda_root",true).toBool();
    _use_partuuid  = m.value("use_partuuid",true).toBool();

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

    QVariantMap p = Json::loadFromFile(folder+"/partitions.json").toMap();
    QVariantList parts = p.value("partitions").toList();
    _configpath = p.value("configpath").toString();
    foreach (QVariant pv, parts)
    {
        _partitions.append(new PartitionInfo(pv.toMap(), this));
    }
}

QDebug operator<<(QDebug dbg, const OsInfo &os)
{
    dbg.nospace() << "OsInfo{";
    dbg.nospace() << "Name="<<os.name();
/*
    dbg.nospace() << ", Flavour ="<<os.flavour();
    dbg.nospace() << ", Folder="<<os.folder();
    dbg.nospace() << ", Description="<<os.description();
    dbg.nospace() << ", Version="<<os.version();
    dbg.nospace() << ", ReleaseData="<<os.releaseDate();
    dbg.nospace() << ", username="<<os.username();
    dbg.nospace() << ", Password="<<os.password();
    dbg.nospace() << ", Url="<<os.url();
    dbg.nospace() << ", Group="<<os.group();
    dbg.nospace() << ", Configpath="<<os.configPath();
    dbg.nospace() << ", ReplacedName="<<os.replacedName();
    dbg.nospace() << ", SupportsBackup="<<os.supportsBackup();
*/
    dbg.nospace() << ", UsePartuuid="<<os.use_partuuid();
    dbg.nospace() << ", SupportsUSBboot="<<os.supports_usb_boot();
    dbg.nospace() << ", SupportsUSBroot="<<os.supports_usb_root();
    dbg.nospace() << ", SupportsSDAboot="<<os.supports_sda_boot();
    dbg.nospace() << ", SupportsSDAroot="<<os.supports_sda_root();
/*
    dbg.nospace() << ", csum="<<os.csum();
    dbg.nospace() << ", CsumType="<<os.csumType();
    dbg.nospace() << ", Bootable="<<os.bootable();
    dbg.nospace() << ", SupportedModels="<<os.supportedModels();
    dbg.nospace() << ", riscosOffset="<<os.riscosOffset();
*/
    dbg.nospace() << "}";

    return dbg.maybeSpace();
}
