#include "partitioninfo.h"
#include <QDebug>

PartitionInfo::PartitionInfo(QObject *parent) :
    QObject(parent)
{
}

PartitionInfo::PartitionInfo(const QVariantMap &m, QObject *parent) :
    QObject(parent)
{
    importMap(m);
}

PartitionInfo::PartitionInfo(int partitionNr, int offset, int sectors, const QByteArray &partType, QObject *parent) :
    QObject(parent), _partitionType(partType), _requiresPartitionNumber(partitionNr), _offset(offset), _partitionSizeSectors(sectors), _active(false)
{
}

void PartitionInfo::importMap(const QVariantMap& m)
{
    _fstype        = m.value("filesystem_type").toByteArray().toLower();
    _mkfsOptions   = m.value("mkfs_options").toByteArray();
    _label         = m.value("label").toByteArray();
    _tarball       = m.value("tarball").toString();
    _wantMaximised = m.value("want_maximised", false).toBool();
    _emptyFS       = m.value("empty_fs", false).toBool();
    _offset        = m.value("offset_in_sectors").toInt();
    _partitionSizeNominal = m.value("partition_size_nominal").toInt();
    _requiresPartitionNumber = m.value("requires_partition_number").toInt();
    _uncompressedTarballSize = m.value("uncompressed_tarball_size").toInt();
    _active        = m.value("active", false).toBool();

    QByteArray defaultPartType;
    if (_fstype.contains("fat"))
        defaultPartType = "0c"; /* FAT32 LBA */
    else if (_fstype == "swap")
        defaultPartType = "82";
    else if (_fstype.contains("ntfs"))
        defaultPartType = "07";
    else
        defaultPartType = "83"; /* Linux native */

    _partitionType = m.value("partition_type", defaultPartType).toByteArray();
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
    qDebug() << "  }";
}
