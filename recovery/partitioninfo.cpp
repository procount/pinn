#include "partitioninfo.h"
#include "util.h"

PartitionInfo::PartitionInfo(const QVariantMap &m, QObject *parent) :
    QObject(parent)
{
    _fstype        = m.value("filesystem_type").toByteArray().toLower();
    _mkfsOptions   = m.value("mkfs_options").toByteArray();
    _label         = m.value("label").toByteArray();
    _tarball       = m.value("tarball").toString();
    _wantMaximised = m.value("want_maximised", false).toBool();
    _emptyFS       = m.value("empty_fs", false).toBool();
    _offset        = m.value("offset_in_sectors").toUInt();
    _partitionSizeNominal = m.value("partition_size_nominal").toUInt();
    _requiresPartitionNumber = m.value("requires_partition_number").toUInt();
    _uncompressedTarballSize = m.value("uncompressed_tarball_size").toUInt();
    _active        = m.value("active", false).toBool();
    _requiresLabel = m.value("requires_label",false).toBool();
    _csumType      = getCsumType(m);
    if (_csumType != "")
        _csum = getCsum(m,_csumType);

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

PartitionInfo::PartitionInfo(int partitionNr, uint offset, uint sectors, const QByteArray &partType, QObject *parent) :
    QObject(parent), _partitionType(partType), _requiresPartitionNumber(partitionNr), _offset(offset), _partitionSizeSectors(sectors), _active(false)
{
}
