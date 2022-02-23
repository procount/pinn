#ifndef OSINFO_H
#define OSINFO_H

/**
 * OS info model
 * Contains the information from os.json, and has a pointer to the partitions
 */

#include <QObject>
#include <QList>
#include <QStringList>

class PartitionInfo;

class OsInfo : public QObject
{
    Q_OBJECT
public:
    /* Constructor parses the json files in <folder>, and stores information in local variables */
    explicit OsInfo(const QString &folder, const QString &flavour = "", QObject *parent = 0);

    inline QString folder() const
    {
        return _folder;
    }

    inline QString flavour() const
    {
        return _flavour;
    }

    inline QString name() const
    {
        return _name;
    }

    inline QString description() const
    {
        return _description;
    }

    inline QString version() const
    {
        return _version;
    }

    inline QString releaseDate() const
    {
        return _releaseDate;
    }

    inline bool bootable() const
    {
        return _bootable;
    }

    inline QList<PartitionInfo *> *partitions()
    {
        return &_partitions;
    }

    inline int riscosOffset() const
    {
        return _riscosOffset;
    }

    inline QStringList supportedModels() const
    {
        return _supportedModels;
    }

    inline QString username() const
    {
        return _username;
    }

    inline QString password() const
    {
        return _password;
    }

    inline QString url() const
    {
        return _url;
    }

    inline QString group() const
    {
        return _group;
    }

    inline QString configPath() const
    {
        return _configpath;
    }

    inline QString replacedName() const
    {
        return _replacedName;
    }

    inline void setReplacedName(QString old_name)
    {
        _replacedName = old_name;
    }

    inline QString supportsBackup() const
    {
        return _supports_backup;
    }

    inline bool supports_usb_boot() const
    {
        return _supports_usb_boot;
    }

    inline bool supports_usb_root() const
    {
        return _supports_usb_root;
    }

    inline bool supports_sda_boot() const
    {
        return _supports_sda_boot;
    }

    inline bool supports_sda_root() const
    {
        return _supports_usb_root;
    }

    inline bool supports_nvme_boot() const
    {
        return _supports_nvme_boot;
    }

    inline bool supports_nvme_root() const
    {
        return _supports_nvme_root;
    }

    inline QString csumType() const
    {
        return (_csumType);
    }

    inline void setCsumType(const QString & type)
    {
        _csumType = type;
    }

    inline QString csum() const
    {
        return (_csum);
    }

    inline void setCsum(const QString & csum)
    {
        _csum = csum;
    }

    inline bool use_partuuid() const
    {
        return(_use_partuuid);
    }

    inline void set_partuuid(const bool useit)
    {
        _use_partuuid = useit;
    }


protected:
    QString _folder, _flavour, _name, _description, _version, _releaseDate;
    QString _username, _password, _url, _group, _configpath, _replacedName;
    bool _supports_usb_boot, _supports_usb_root;
    bool _supports_sda_boot, _supports_sda_root;
    bool _supports_nvme_boot, _supports_nvme_root;
    bool _bootable, _use_partuuid;
    QString _supports_backup;
    QString _csum, _csumType;
    QList<PartitionInfo *> _partitions;
    QStringList _supportedModels;
    int _riscosOffset;

};

QDebug operator<<(QDebug dbg, const OsInfo &os);

#endif // OSINFO_H
