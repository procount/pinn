#include "partitioninfo.h"
#include <QDebug>

PartitionInfo::PartitionInfo(QObject *parent) :
    QObject(parent)
{
    init();
}

PartitionInfo::PartitionInfo(const QVariantMap &m, QObject *parent) :
    QObject(parent)
{
    init();
    importMap(m);
}

PartitionInfo::PartitionInfo(int partitionNr, int offset, int sectors, const QByteArray &partType, QObject *parent) :
    QObject(parent), _partitionType(partType), _requiresPartitionNumber(partitionNr), _offset(offset), _partitionSizeSectors(sectors), _active(false)
{
    _fstype="";
    _mkfsOptions="";
    _label="";
    _partitionDevice="";
    _tarball="";
    _partitionSizeNominal=0;
    _uncompressedTarballSize=0;
    _emptyFS=false;
    _wantMaximised=false;
    //_partitionType="";
    //_requiresPartitionNumber=0;
    //_offset=0;
    //_partitionSizeSectors=0;
    //_active=false;
}

void PartitionInfo::init()
{
    _fstype="";
    _mkfsOptions="";
    _label="";
    _partitionDevice="";
    _tarball="";
    _partitionSizeNominal=0;
    _uncompressedTarballSize=0;
    _emptyFS=false;
    _wantMaximised=false;
    _partitionType="";
    _requiresPartitionNumber=0;
    _offset=0;
    _partitionSizeSectors=0;
    _active=false;
}

void PartitionInfo::importMap(const QVariantMap& m)
{
    for(QVariantMap::const_iterator iter = m.constBegin(); iter != m.constEnd(); ++iter)
    {
        QString key = iter.key();
        if (key == "filesystem_type")
            _fstype        = iter.value().toByteArray().toLower();
        else if (key == "mkfs_options")
            _mkfsOptions   = iter.value().toByteArray();
        else if (key == "label")
            _label         = iter.value().toByteArray();
        else if (key == "tarball")
            _tarball       = iter.value().toString();
        else if (key == "want_maximised")
            _wantMaximised = iter.value().toBool();
        else if (key == "empty_fs")
            _emptyFS       = iter.value().toBool();
        else if (key == "offset_in_sectors")
            _offset        = iter.value().toInt();
        else if (key == "partition_size_nominal")
            _partitionSizeNominal = iter.value().toInt();
        else if (key == "requires_partition_number")
            _requiresPartitionNumber = iter.value().toInt();
        else if (key == "uncompressed_tarball_size")
            _uncompressedTarballSize = iter.value().toInt();
        else if (key == "active")
            _active        = iter.value().toBool();
        else
            qDebug() << "Unknown: " << key << iter.value();

    }
    QByteArray defaultPartType;
    if (_fstype.contains("fat"))
        defaultPartType = "0c"; /* FAT32 LBA */
    else if (_fstype == "swap")
        defaultPartType = "82";
    else if (_fstype.contains("ntfs"))
        defaultPartType = "07";
    else
        defaultPartType = "83"; /* Linux native */

    if (m.contains("partition_type"))
        _partitionType = m.value("partition_type").toByteArray();
    else
        _partitionType =  defaultPartType;
}

void PartitionInfo::print()
{
    qDebug() << "  {";
    qDebug() << "  _fstype="<<_fstype;
    qDebug() << "  _mkfsOptions"<<_mkfsOptions;
    qDebug() << "  _label="<<_label;
    qDebug() << "  _tarball="<<_tarball;
    qDebug() << "  _wantMaximised="<<_wantMaximised;
    qDebug() << "  _emptyFS="<<_emptyFS;
    qDebug() << "  _offset="<<_offset;
    qDebug() << "  _partitionSizeNominal="<<_partitionSizeNominal;
    qDebug() << "  _requiresPartitionNumber="<<_requiresPartitionNumber;
    qDebug() << "  _uncompressedTarballSize="<<_uncompressedTarballSize;
    qDebug() << "  _active="<<_active;
    qDebug() << "  _partitionType="<<_partitionType;
    qDebug() << "  _partitionDevice=" << _partitionDevice;
    qDebug() << "  _partitionSizeSectors=" << _partitionSizeSectors;
    qDebug() << "  }";
}
