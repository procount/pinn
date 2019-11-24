#include "multiimagewritethread.h"
#include "json.h"
#include "util.h"
#include "mbr.h"
#include "osinfo.h"
#include "partitioninfo.h"
#include "util.h"
#include "mydebug.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSettings>
#include <QTime>
#include <QUrl>
#include <unistd.h>
#include <linux/fs.h>
#include <linux/magic.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>
#include <QtEndian>

#include <sys/types.h>
#include <sys/stat.h>

QString readexec(int log, const QString& cmd, int & errorcode);

MultiImageWriteThread::MultiImageWriteThread(const QString &bootdrive, const QString &rootdrive, bool noobsconfig, bool partition, enum ModeTag mode, QObject *parent) :
    QThread(parent), _drive(rootdrive), _bootdrive(bootdrive), _extraSpacePerPartition(0), _part(5), _noobsconfig(noobsconfig), _partition(partition), _downloadMode(mode)
{
    QDir dir;
    _multiDrives = (bootdrive != rootdrive);

    if (!dir.exists("/mnt2"))
        dir.mkdir("/mnt2");
}

void MultiImageWriteThread::addImage(const QString &folder, const QString &flavour)
{
    _images.append(new OsInfo(folder, CORE(flavour), this));
}

void MultiImageWriteThread::addInstalledImage(const QString &folder, const QString &flavour, const QVariantMap &entry,
                                              const QString &replacedName)
{   /* Copy the previously installed partitions to the new OS and add the OS to the list of OSes to be installed */
    OsInfo * pInfo = new OsInfo(folder, getNameParts(flavour, eCORE|eNICKNAME), this);

    //Get the list of partitions where this OS is already installed
    QVariantList list = entry.value("partitions").toList(); //of QVariant Strings
    //Get ptr to partitions for new OS
    QList<PartitionInfo *> * tParts = pInfo->partitions();
    int i=0;
    foreach (PartitionInfo *partition, *tParts)
    {
        //Set partition of NEW os to be the same as the previously INSTALLED OS
        partition->setPartitionDevice( list[i].toByteArray() );
        i++;
    }

    if (! replacedName.isEmpty())
        pInfo->setReplacedName(replacedName);
    else
        pInfo->setReplacedName(entry.value("name").toString());

    _images.append(pInfo);
}

void MultiImageWriteThread::run()
{
    TRACE
    if (_partition)
    {
        /* Calculate space requirements, and check special requirements */
        uint totalnominalsize = 0, totaluncompressedsize = 0, numparts = 0, numexpandparts = 0;
        uint startSector = getFileContents(sysclassblock(_drive, 5)+"/start").trimmed().toUInt()
                        + getFileContents(sysclassblock(_drive, 5)+"/size").trimmed().toUInt();
        uint totalSectors = getFileContents(sysclassblock(_drive)+"/size").trimmed().toUInt();
        uint availableMB = (totalSectors-startSector)/2048;
        /* key: partition number, value: partition information */
        QMap<int, PartitionInfo *> partitionMap, bootPartitionMap;

        foreach (OsInfo *image, _images)
        {
            QList<PartitionInfo *> *partitions = image->partitions();
            if (partitions->isEmpty())
            {
                emit error(tr("partitions.json invalid"));
                return;
            }

            if (nameMatchesRiscOS( image->folder() ))
            {
                /* Check the riscos_offset in os.json matches what we're expecting.
                   In theory we shouldn't hit either of these errors because the invalid RISC_OS
                   should have been filtered out already (not added to OS-list) in mainwindow.cpp */
                if (image->riscosOffset())
                {
                    if (image->riscosOffset() != RISCOS_OFFSET)
                    {
                        emit error(tr("RISCOS cannot be installed. RISCOS offset value mismatch."));
                        return;
                    }
                }
                else
                {
                    emit error(tr("RISCOS cannot be installed. RISCOS offset value missing."));
                    return;
                }
                if (startSector > RISCOS_SECTOR_OFFSET-2048)
                {
                    emit error(tr("RISCOS cannot be installed. Size of recovery partition too large."));
                    return;
                }

                totalnominalsize += (RISCOS_SECTOR_OFFSET - startSector)/2048;

                partitions->first()->setRequiresPartitionNumber(6);
                partitions->first()->setOffset(RISCOS_SECTOR_OFFSET);
                partitions->last()->setRequiresPartitionNumber(7);
            }

            foreach (PartitionInfo *partition, *partitions)
            {
                numparts++;
                if ( partition->wantMaximised() )
                    numexpandparts++;
                uint nominalsize = partition->partitionSizeNominal();
                totalnominalsize += nominalsize;
                totaluncompressedsize += partition->uncompressedTarballSize();

                if (partition->fsType() == "ext4")
                {
                    totaluncompressedsize += nominalsize / 20 ; /* overhead for file system meta data */
                }
                int reqPart = partition->requiresPartitionNumber();
                if (reqPart)
                {
                    if (partitionMap.contains(reqPart))
                    {
                        emit error(tr("More than one operating system requires partition number %1").arg(reqPart));
                        return;
                    }
                    if (reqPart == 1 || reqPart == 5)
                    {
                        emit error(tr("Operating system cannot require a system partition (1,5)"));
                        return;
                    }
                    if ((reqPart == 2 && partitionMap.contains(4)) || (reqPart == 4 && partitionMap.contains(2)))
                    {
                        emit error(tr("Operating system cannot claim both primary partitions 2 and 4"));
                        return;
                    }

                    partition->setPartitionDevice(partdev(_drive, reqPart));
                    partitionMap.insert(reqPart, partition);
                }

                /* Maximum overhead per partition for alignment */
    #ifdef SHRINK_PARTITIONS_TO_MINIMIZE_GAPS
                if (partition->wantMaximised() || (partition->partitionSizeNominal()*2048) % PARTITION_ALIGNMENT != 0)
                    totalnominalsize += PARTITION_ALIGNMENT/2048;
    #else
                totalnominalsize += PARTITION_ALIGNMENT/2048;
    #endif
            }
        }

        if (numexpandparts)
        {
            /* Extra spare space available for partitions that want to be expanded */
            _extraSpacePerPartition = (availableMB-totalnominalsize)/numexpandparts;
        }

        emit parsedImagesize(qint64(totaluncompressedsize)*1024*1024);

        if (totalnominalsize > availableMB)
        {
            emit error(tr("Not enough disk space. Need %1 MB, got %2 MB").arg(QString::number(totalnominalsize), QString::number(availableMB)));
            return;
        }

        /* Assign logical partition numbers to partitions that did not reserve a special number */
        int pnr, bootpnr;
        uint offset = 0;
        if (partitionMap.isEmpty())
            pnr = 6;
        else
            pnr = qMax(partitionMap.keys().last(), 5)+1;

        if (_multiDrives)
        {
            bootpnr = 6;
            offset = getFileContents(sysclassblock(_bootdrive, 5)+"/start").trimmed().toUInt()
                   + getFileContents(sysclassblock(_bootdrive, 5)+"/size").trimmed().toUInt();
        }

        foreach (OsInfo *image, _images)
        {
            foreach (PartitionInfo *partition, *(image->partitions()))
            {
                if (!partition->requiresPartitionNumber())
                {
                    if (_multiDrives && partition->bootable() && !partition->wantMaximised() )
                    {
                        if (bootpnr >=63 )
                        {
                            emit error(tr("Cannot boot partitions > 62. Reduce the number of OSes"));
                            return;
                        }
                        bootPartitionMap.insert(bootpnr, partition);
                        partition->setPartitionDevice(partdev(_bootdrive, bootpnr));
                        bootpnr++;
                        offset += PARTITION_GAP;

                        /* Align at 4 MiB offset */
                        if (offset % PARTITION_ALIGNMENT != 0)
                        {
                                offset += PARTITION_ALIGNMENT-(offset % PARTITION_ALIGNMENT);
                        }
                        partition->setOffset(offset);
                        uint partsizeSectors = partition->partitionSizeNominal() * 2048;
                        partition->setPartitionSizeSectors(partsizeSectors);
                        offset += partsizeSectors;
                    }
                    else
                    {
                        if (partition->bootable() && pnr >=63 )
                        {
                            emit error(tr("Cannot boot partitions > #62. Reduce the number of OSes"));
                            return;
                        }
                        partitionMap.insert(pnr, partition);
                        partition->setPartitionDevice(partdev(_drive, pnr));
                        pnr++;
                    }
                }
            }
        }

        /* Set partition starting sectors and sizes.
         * First allocate space to all logical partitions, then to primary partitions */
        QList<PartitionInfo *> log_before_prim = partitionMap.values();
        if (!log_before_prim.isEmpty() && log_before_prim.first()->requiresPartitionNumber() == 2)
            log_before_prim.push_back(log_before_prim.takeFirst());
        if (!log_before_prim.isEmpty() && log_before_prim.first()->requiresPartitionNumber() == 3)
            log_before_prim.push_back(log_before_prim.takeFirst());
        if (!log_before_prim.isEmpty() && log_before_prim.first()->requiresPartitionNumber() == 4)
            log_before_prim.push_back(log_before_prim.takeFirst());

        offset = startSector;

        foreach (PartitionInfo *p, log_before_prim)
        {
            if (p->offset()) /* OS wants its partition at a fixed offset */
            {
                if (p->offset() <= offset)
                {
                    emit error(tr("Fixed partition offset too low"));
                    return;
                }

                offset = p->offset();
            }
            else
            {
                offset += PARTITION_GAP;
                /* Align at 4 MiB offset */
                if (offset % PARTITION_ALIGNMENT != 0)
                {
                        offset += PARTITION_ALIGNMENT-(offset % PARTITION_ALIGNMENT);
                }

                p->setOffset(offset);
            }

            uint partsizeMB = p->partitionSizeNominal();
            if ( p->wantMaximised() )
                partsizeMB += _extraSpacePerPartition;
            uint partsizeSectors = partsizeMB * 2048;

            if (p == log_before_prim.last())
            {
                /* Let last partition have any remaining space that we couldn't divide evenly */
                uint spaceleft = totalSectors - offset - partsizeSectors;

                if (spaceleft > 0 && p->wantMaximised())
                {
                    partsizeSectors += spaceleft;
                }
            }
            else
            {
    #ifdef SHRINK_PARTITIONS_TO_MINIMIZE_GAPS
                if (partsizeSectors % PARTITION_ALIGNMENT == 0 && p->fsType() != "raw")
                {
                    /* Partition size is dividable by 4 MiB
                       Take off a couple sectors of the end of our partition to make room
                       for the EBR of the next partition, so the next partition can
                       align nicely without having a 4 MiB gap */
                    partsizeSectors -= PARTITION_GAP;
                }
    #endif
                if (p->wantMaximised() && (partsizeSectors+PARTITION_GAP) % PARTITION_ALIGNMENT != 0)
                {
                    /* Enlarge partition to close gap to next partition */
                    partsizeSectors += PARTITION_ALIGNMENT-((partsizeSectors+PARTITION_GAP) % PARTITION_ALIGNMENT);
                }
            }

            p->setPartitionSizeSectors(partsizeSectors);
            offset += partsizeSectors;
        }
        /* Delete information about previously installed operating systems */
        QFile f("/settings/installed_os.json");
        if (f.exists())
            f.remove();

        emit statusUpdate(tr("Writing partition table"));
        if (!writePartitionTable(_drive, partitionMap))
            return;

        /* Zero out first sector of partitions, to make sure to get rid of previous file system (label) */
        emit statusUpdate(tr("Zero'ing start of each partition"));
        foreach (PartitionInfo *p, partitionMap.values())
        {
            if (p->partitionSizeSectors())
                QProcess::execute("/bin/dd count=3 bs=512 if=/dev/zero of="+p->partitionDevice());
        }

        /* Write partition table to boot drive (if using multiple drives) */
        if (_multiDrives)
        {
            emit statusUpdate(tr("Writing boot partition table"));

            if (!writePartitionTable(_bootdrive, bootPartitionMap))
                return;

            /* Zero out first sector of partitions, to make sure to get rid of previous file system (label) */
            emit statusUpdate(tr("Zero'ing start of each partition"));
            foreach (PartitionInfo *p, bootPartitionMap.values())
            {
                if (p->partitionSizeSectors())
                    QProcess::execute("/bin/dd count=3 bs=512 if=/dev/zero of="+p->partitionDevice());
            }

            if (QProcess::execute("mount -t ext4 "+partdev(_bootdrive, SETTINGS_PARTNR)+" /mnt2") == 0)
            {
                QFile f("/mnt2/installed_os.json");
                if (f.exists())
                    f.remove();

                QProcess::execute("umount /mnt2");
            }
        }

    }
    else    // !_partition / reinstall
    {   //Calculate amount to write to disk
        uint totaluncompressedsize = 0;
        foreach (OsInfo *image, _images)
        {
            QList<PartitionInfo *> *partitions = image->partitions();
            foreach (PartitionInfo *partition, *partitions)
            {
                uint uncompressedSize = partition->uncompressedTarballSize(); //in MB
                uint nominalsize = partition->partitionSizeNominal();

                totaluncompressedsize += partition->uncompressedTarballSize();
                if (partition->fsType() == "ext4")
                {
                    totaluncompressedsize += nominalsize / 20 ; /* overhead for file system meta data */
                }

                if (partition->partitionDevice().contains("PARTUUID"))
                {
                    QByteArray pdev = getDevice(partition->partitionDevice());
                    partition->setPartitionDevice(pdev);
                }

                uint partitionSectors = getFileContents(sysclassblock(partition->partitionDevice())+"/size").trimmed().toUInt();
                if (uncompressedSize*2048 > partitionSectors)
                {
                    qDebug () << "UncompressedSize=" <<uncompressedSize*2048<<". partitionSectors=" <<partitionSectors;
                    emit error(tr("Cannot Reinstall/Replace ")+image->name()+tr(".\nPartition not big enough for new image."));
                    return;
                }
            }
        }
        emit parsedImagesize(qint64(totaluncompressedsize)*1024*1024);
    }

    if (!_partition)
    {   //If we are re-installing, then re-read the OSes that are already installed.
        installed_os = Json::loadFromFile("/settings/installed_os.json").toList();
    }

    /* Install each operating system */
    int oserrors=0;
    foreach (OsInfo *image, _images)
    {
        QMessageBox::ButtonRole reply;

        reply=processImage(image);

        if (_checksumError || _setupError )
            oserrors++;

        if ((reply == QMessageBox::RejectRole) || (reply == QMessageBox::DestructiveRole))
        {
            //Delete the faulty OS.
            oserrors++;
            QList<PartitionInfo *> *partitions = image->partitions();
            foreach (PartitionInfo *p, *partitions)
            {
                //mount partition
                QByteArray partdevice = p->partitionDevice();
                if (QProcess::execute("mount "+partdevice+" /mnt2") == 0)
                {
                    //remove all files
                    QProcess::execute("sh -c \" cd /mnt2;rm -rf *\"");
                    //Umount
                    QProcess::execute("umount /mnt2");
                }
            }
        }

        if (reply == QMessageBox::DestructiveRole)
        {   //Cancel all downloads
            sync();
            emit error(tr("Operation cancelled by user"));
            return;
        }

    }

    emit statusUpdate(tr("Finish writing (sync)"));
    sync();
    emit completed(oserrors);
}

bool MultiImageWriteThread::writePartitionTable(const QString &drive, const QMap<int, PartitionInfo *> &pmap)
{
    TRACE
    /* Write partition table using sfdisk */

    /* Fixed PINN partition */
    uint startP1 = getFileContents(sysclassblock(drive, 1)+"/start").trimmed().toUInt();
    uint sizeP1  = getFileContents(sysclassblock(drive, 1)+"/size").trimmed().toUInt();
    /* Fixed start of extended partition. End is not fixed, as it depends on primary partition 3 & 4 */
    int startExtended = startP1+sizeP1;
    /* Fixed settings partition */
    uint startP5 = getFileContents(sysclassblock(drive, SETTINGS_PARTNR)+"/start").trimmed().toUInt();
    uint sizeP5  = getFileContents(sysclassblock(drive, SETTINGS_PARTNR)+"/size").trimmed().toUInt();

    if (!startP1 || !sizeP1 || !startP5 || !sizeP5)
    {
        emit error(tr("Error reading existing partition table"));
        return false;
    }

    /* Clone partition map, and add our system partitions to it */
    QMap<int, PartitionInfo *> partitionMap(pmap);

    partitionMap.insert(1, new PartitionInfo(1, startP1, sizeP1, "0E", NULL)); /* FAT boot partition */
    partitionMap.insert(5, new PartitionInfo(5, startP5, sizeP5, "L", NULL)); /* Ext4 settings partition */

    uint sizeExtended = partitionMap.values().last()->endSector() - startExtended;
    if (!partitionMap.contains(2))
    {
        partitionMap.insert(2, new PartitionInfo(2, startExtended, sizeExtended, "E", NULL));
    }
    else
    {
        /* If an OS already claimed primary partition 2, use out-of-order partitions, and store extended at partition 4 */
        partitionMap.insert(4, new PartitionInfo(4, startExtended, sizeExtended, "E", NULL));
    }

    /* Add partitions */
    qDebug() << "partition map:" << partitionMap;

    QByteArray partitionTable;
    for (int i=1; i <= partitionMap.keys().last(); i++)
    {
        if (partitionMap.contains(i))
        {
            PartitionInfo *p = partitionMap.value(i);

            partitionTable += QByteArray::number(p->offset())+","+QByteArray::number(p->partitionSizeSectors())+","+p->partitionType();
            if (p->active())
                partitionTable += " *";
            partitionTable += "\n";
        }
        else
        {
            partitionTable += "0,0\n";
        }
    }

    qDebug() << "New partition table:";
    qDebug() << partitionTable;

    /* Unmount everything before modifying partition table */
    QString driveP1 = partdev(drive, 1).replace("/dev/", "");
    if (drive == _bootdrive)
    {
        QProcess::execute("umount /mnt");
        QProcess::execute("umount /settings");
    }
    if (QFile::exists("/tmp/media/"+driveP1))
    {
        QProcess::execute("umount /tmp/media/"+driveP1);
    }

    /* Let sfdisk write a proper partition table */
    QProcess proc;
    proc.setProcessChannelMode(proc.MergedChannels);
    proc.start("/sbin/sfdisk -uS --force "+drive);
    proc.write(partitionTable);
    proc.closeWriteChannel();
    proc.waitForFinished(-1);

    qDebug() << "sfdisk done, output:" << proc.readAll();
    ::sync();
    QThread::msleep(500);

    QProcess::execute("/usr/sbin/partprobe");
    QThread::msleep(500);

    /* Remount */
    if (drive == _bootdrive)
    {
        QProcess::execute("mount -o ro -t vfat "+partdev(drive, 1)+" /mnt");
        QProcess::execute("mount -t ext4 "+partdev(drive, SETTINGS_PARTNR)+" /settings");
    }
    if (QFile::exists("/tmp/media/"+driveP1))
    {
        QProcess::execute("mount -o ro -t vfat /dev/"+driveP1+" /tmp/media/"+driveP1);
    }

    if (proc.exitCode() != 0)
    {
        emit error(tr("Error creating partition table")+"\n"+proc.readAll());
        return false;
    }

    //Forcefully delete the PartitionInfo entries as the parent was set to NULL
    for (int i=1; i <= partitionMap.keys().last(); i++)
    {
        if (partitionMap.contains(i))
        {
            PartitionInfo *p = partitionMap.value(i);
            if (p->parent() == NULL)
                delete p;
        }
    }

    return true;
}

QString MultiImageWriteThread::findTarballExt(QString base, QString exts)
{
    QString tarball;
    QStringList extList = exts.split(",");
    foreach (QString ext, extList)
    {
        tarball = base + ext;
        if (QFile::exists(tarball))
        {
            break;
        }
    }
    //This doesn't necessarily indicate it is found!
    return (tarball);
}

QString MultiImageWriteThread::shorten(QString example, int maxLabelLen)
{
    QString test;
    if (example.size()<=maxLabelLen)
    {
        return(example);
    }
    example.replace("_","#");
    example.replace("-","#");
    example.replace(" ","#");

    QStringList parts = example.split("#", QString::SkipEmptyParts);
    int numParts = qMin(3, parts.count());
    int r;
    int len;
    int l1,l2;
    int rem;
    switch (numParts)
    {
        case 3:
            len=parts.last().size();
            r=qMin((maxLabelLen-4),len);
            rem = maxLabelLen -r-2;
            l1 = rem/2;
            l2 = rem-l1;
            test= parts.first().left(l1)+"_"+parts.at(1).right(l2)+"_"+parts.last().left(r);
            break;

        case 2:
            len=parts.last().size();
            r=qMin(maxLabelLen-2, len);
            test = parts.first().left(maxLabelLen-r-1) + "_" + parts.last().left(r);
            break;

        default:
            test = parts.first();
            test=test.left(maxLabelLen);
            break;
    }
    return(test);
}


QByteArray MultiImageWriteThread::makeLabelUnique(QByteArray label, int maxLabelLen, const QByteArray &device)
{
    if (_partition)
    {
        if (label.size() > maxLabelLen)
        {   //restrict to maximum size
            label = shorten(label, maxLabelLen).toAscii();
        }

        if (!isLabelAvailable(label, device))
        {
            if (label.size() == maxLabelLen)
            {   //Make room for extra digit
                label = label.left(maxLabelLen-1);
            }
            for (int i=0; i<10; i++)
            {
                if (isLabelAvailable(label+QByteArray::number(i), device))
                {
                    label = label+QByteArray::number(i);
                    return(label);
                }
            }
            //Let's add some more, just in case
            for (char c='A'; c<='Z'; c++)
            {
                if (isLabelAvailable(label+c, device))
                {
                    label = label+c;
                    return(label);
                }
            }
            //No hope if we get to here
            label="";
        }
    }
    return (label);
}

QMessageBox::ButtonRole MultiImageWriteThread::processImage(OsInfo *image)
{
    TRACE
    QString os_name = image->name();
    qDebug() << "Processing OS:" << os_name;

    _setupError = false;

    QList<PartitionInfo *> *partitions = image->partitions();
    _checksumError = 0;

    foreach (PartitionInfo *p, *partitions)
    {
        QByteArray fstype   = p->fsType();
        QByteArray mkfsopt  = p->mkfsOptions();
        QByteArray label = p->label();
        QString tarball  = p->tarball();
        bool emptyfs     = p->emptyFS();
        QString csumType = p->csumType();
        QString csum     = p->csum();

        if (csumType=="")
            csumType="sha512sum";

        if (!emptyfs && tarball.isEmpty())
        {
            /* If no tarball URL is specified, we expect the tarball to reside in the folder and be named <label.tar.xz> */
            if (fstype == "raw" || fstype.startsWith("partclone"))
                tarball = findTarballExt(image->folder()+"/"+label,".img.lzo,.img.gz,.img.bz2,.img.zip,.img.xz,.lzo,.gz,.bz2,.zip,.xz");
            else
                tarball = findTarballExt(image->folder()+"/"+label,".tar.lzo,.tar.gz,.tar.bz2,.tar.zip,.tar.xz,.tar");

            if (!QFile::exists(tarball))
            {
                emit error(tr("File '%1' does not exist").arg(tarball));
                return QMessageBox::RejectRole;
            }
        }
        else if (!emptyfs && !isURL(tarball))
        {
            /* non-URL tarball is specified */
            if (!tarball.contains("/"))
            {
                /* Assume just the filename was specified, so add the path */
                tarball = image->folder()+"/"+tarball;
            }
        }

        QByteArray partdevice = p->partitionDevice();

        if ( (!_partition) && ( (_downloadMode == MODE_REINSTALL) || (_downloadMode == MODE_REINSTALLNEWER))) //@@ (not for replace)
        {   //Use the existing partition label
            int errorcode;
            QString s= readexec(0, "lsblk -noheadings -o label "+QString(partdevice), errorcode);
            s.remove('\n');
            s.remove('\r');
            s=s.trimmed();
            label = s.toLocal8Bit();
        }

        if ((p->requiresLabel()) && (!isLabelAvailable(label,partdevice)))
        {
            emit error(tr("OS: '%1' needs a partition label of '%2' which is not available").arg(image->name(),QString(label)));
            return QMessageBox::RejectRole;
        }

        QMessageBox::ButtonRole result = QMessageBox::AcceptRole;
        _checksumError++;
        do
        {
            _checksumError--;

            if (fstype == "raw")
            {
                emit statusUpdate(tr("%1: Writing OS image").arg(os_name));
                emit newDrive(partdevice,ePM_WRITESTATS);
                emit startAccounting();
                result = dd(tarball, csumType, csum, partdevice);
            }
            else if (fstype.startsWith("partclone"))
            {
                emit statusUpdate(tr("%1: Writing OS image").arg(os_name));
                emit newDrive(partdevice,ePM_WRITESTATS);
                emit startAccounting();
                result = partclone_restore(tarball, csumType, csum, partdevice);
            }
            else if (fstype != "unformatted")
            {
                emit statusUpdate(tr("%1: Creating filesystem (%2)").arg(os_name, QString(fstype)));

                emit idle();
                if (!mkfs(partdevice, fstype, label, mkfsopt))
                    return QMessageBox::RejectRole;
                emit cont();

                if (!emptyfs)
                {
                    emit statusUpdate(tr("%1: Mounting file system").arg(os_name));
                    if (QProcess::execute("mount "+partdevice+" /mnt2") != 0)
                    {
                        emit error(tr("%1: Error mounting file system").arg(os_name));
                        return QMessageBox::RejectRole;
                    }

                    if (tarball.startsWith("http"))
                        emit statusUpdate(tr("%1: Downloading and extracting filesystem").arg(os_name));
                    else
                        emit statusUpdate(tr("%1: Extracting filesystem").arg(os_name));

                    emit newDrive(partdevice,ePM_WRITEDF);
                    emit startAccounting();
                    result = untar(tarball,csumType, csum);

                    emit statusUpdate(tr("Syncing Filesystem"));
                    emit idle();
                    QProcess::execute("umount /mnt2");
                    emit cont();
                }
            }
            emit stopAccounting();

        } while (result == QMessageBox::NoRole); //retry

        emit consolidate();

        if (result != QMessageBox::AcceptRole)
            return result;

        _part++;
    } //next partition

    emit statusUpdate(tr("%1: Mounting FAT partition").arg(os_name));
    if (QProcess::execute("mount "+partitions->first()->partitionDevice()+" /mnt2") != 0)
    {
        emit error(tr("%1: Error mounting file system").arg(os_name));
        return QMessageBox::RejectRole;
    }

    emit statusUpdate(tr("%1: Creating os_config.json").arg(os_name));

    QString description = getDescription(image->folder(), image->flavour());
    QVariantList vpartitions;
    foreach (PartitionInfo *p, *partitions)
    {
        QString part = p->partitionDevice();
        if ((part.left(8) != "PARTUUID") && (part.left(13) != "/dev/mmcblk0p"))
        {
            part = getPartUUID(part);
        }
        vpartitions.append(part);
    }
    QSettings settings("/settings/noobs.conf", QSettings::IniFormat);
    int videomode = settings.value("display_mode", 0).toInt();
    QString language = settings.value("language", "en").toString();
    QString keyboard = settings.value("keyboard_layout", "gb").toString();

    QVariantMap qm;
    qm.insert("flavour", image->flavour());
    qm.insert("release_date", image->releaseDate());
    qm.insert("imagefolder", CORE(image->folder()));
    qm.insert("description", description);
    qm.insert("videomode", videomode);
    qm.insert("partitions", vpartitions);
    qm.insert("language", language);
    qm.insert("keyboard", keyboard);
    qm.insert("bootable", image->bootable());
    qm.insert("group", image->group());


    Json::saveToFile("/mnt2/os_config.json", qm);

    emit statusUpdate(tr("%1: Saving display mode to config.txt").arg(os_name));
    patchConfigTxt();

    /* Partition setup script can either reside in the image folder
     * or inside the boot partition tarball */
    QString postInstallScript = image->folder()+"/partition_setup.sh";
    if (!QFile::exists(postInstallScript))
        postInstallScript = "/mnt2/partition_setup.sh";

    if (QFile::exists(postInstallScript))
    {
        QString csumType = image->csumType();
        QString csum     = image->csum();

        if ((csum != "") && (csumType != ""))
        {
            int errorcode;
            QString filecsum = readexec(1,csumType+" "+postInstallScript, errorcode).split(" ").first();
            qDebug()<< "Expected partition_setup.sh csum= "<<csum<<" Calculated= "<<filecsum;
            if (filecsum != csum)
            {
                emit errorContinue(tr("Error in checksum for partition_setup.sh"));
                _setupError = true;   //Force OS to be non-bootable.
            }
        }

        if ( !_setupError )
        {
            emit statusUpdate(tr("%1: Running partition setup script").arg(os_name));
            QProcess proc;
            QProcessEnvironment env;
            QStringList args(postInstallScript);
            env.insert("PATH", "/bin:/usr/bin:/sbin:/usr/sbin");

            /* - Parameters to the partition-setup script are supplied both as
             *   command line parameters and set as environement variables
             * - Boot partition is mounted, working directory is set to its mnt folder
             *
             *  partition_setup.sh part1=/dev/mmcblk0p3 id1=LABEL=BOOT part2=/dev/mmcblk0p4
             *  id2=UUID=550e8400-e29b-41d4-a716-446655440000
             */
            int pnr = 1;
            foreach (PartitionInfo *p, *partitions)
            {
                QString part  = p->partitionDevice();
                QString nr    = QString::number(pnr);
                QString uuid  = getUUID(part);
                QString label = getLabel(part);
                QString partuuid = getPartUUID(part);
                QString id;
                if (!label.isEmpty())
                    id = "LABEL="+label;
                else
                    id = "UUID="+uuid;
                if (_drive != "/dev/mmcblk0")
                    part = partuuid;

                qDebug() << "part" << part << uuid << label;

                args << "part"+nr+"="+part << "id"+nr+"="+id;
                env.insert("part"+nr, part);
                env.insert("id"+nr, id);
                env.insert("partuuid"+nr, partuuid);
                pnr++;
            }

            //If this is a backup, there maybe things in the partition_setup we don't want to repeat
            //So set "restore=true"
            if ( !getNameParts(os_name, eDATE).isEmpty() )
                env.insert("restore", "true");

            qDebug() << "Executing: sh" << args;
            qDebug() << "Env:" << env.toStringList();
            proc.setProcessChannelMode(proc.MergedChannels);
            proc.setProcessEnvironment(env);
            proc.setWorkingDirectory("/mnt2");
            proc.start("/bin/sh", args);
            proc.waitForFinished(-1);
            qDebug() << proc.exitStatus();

            if (proc.exitCode() != 0)
            {
                QProcess::execute("umount /mnt2");
                emit statusUpdate(tr("%1: Error executing partition setup script").arg(os_name)+"\n"+proc.readAll());
                return QMessageBox::RejectRole;
            }
        }

    }

    emit statusUpdate(tr("%1: Unmounting FAT partition").arg(os_name));
    if (QProcess::execute("umount /mnt2") != 0)
    {
        emit errorContinue(tr("%1: Error unmounting").arg(os_name));
    }

    /* Now see if there are any customisations
     */
    if ((_noobsconfig) && (getNameParts(os_name, eDATE).isEmpty() )) //Only process flavour if it is NOT a backup
    {
        emit statusUpdate(tr("%1: Configuring flavour").arg(os_name));
        qDebug() <<"Checking for partition customisations"    ;
        foreach (PartitionInfo *p, *partitions)
        {
            QByteArray part = p->partitionDevice();
            QByteArray label = p->label();
            QString customName = image->flavour() + "_"+label;

            //qDebug() << "part" <<  part << label << customName;
            postInstallConfig(image, part, customName);
        }
    }

    emit statusUpdate(tr("%1: Mounting FAT partition").arg(os_name));
    QString cmd ="mount "+partitions->first()->partitionDevice()+" /mnt2";
    if (QProcess::execute(cmd) != 0)
    {
        emit error(tr("%1: Error mounting file system").arg(os_name));
        return QMessageBox::RejectRole;
    }

    if (!g_nofirmware)
    {
        emit statusUpdate(tr("%1: Checking firmware update").arg(os_name));
        qDebug() <<"Checking for firmware Overrides";
        QDir dir ("/mnt/firmware.override");
        if (dir.exists())
        {
            dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
            dir.setSorting(QDir::Size | QDir::Reversed);

            QFileInfoList list = dir.entryInfoList();
            for (int i = 0; i < list.size(); ++i)
            {
                QFileInfo fileInfo = list.at(i);
                qDebug() << "Examining " << fileInfo.absoluteFilePath() << " " << fileInfo.lastModified();
                QFileInfo fileInfoOS( "/mnt2/" + fileInfo.fileName() ) ;
                qDebug() << "against " << fileInfoOS.absoluteFilePath() << " " << fileInfoOS.lastModified();
                if (fileInfoOS.exists())
                {
                    if (fileInfo.lastModified() > fileInfoOS.lastModified())
                    {
                        qDebug() << "Copying...";
                        if (QProcess::execute("cp -p /mnt/firmware.override/"+fileInfo.fileName() + " /mnt2") != 0)
                            qDebug() <<"ERROR!";
                    }
                }
            }
        }
    }

    emit statusUpdate(tr("%1: Unmounting FAT partition").arg(os_name));
    if (QProcess::execute("umount /mnt2") != 0)
    {
        emit errorContinue(tr("%1: Error unmounting").arg(os_name));
    }

    /* Save information about installed operating systems in installed_os.json */
    QVariantMap ventry;
    ventry["name"]        = image->flavour();
    ventry["description"] = description;
    ventry["folder"]      = CORE(image->folder());
    ventry["release_date"]= image->releaseDate();
    ventry["partitions"]  = vpartitions;
    ventry["bootable"]    = image->bootable();
    if (!image->supportedModels().isEmpty())
        ventry["supported_models"] = image->supportedModels();
    ventry["username"]    = image->username();
    ventry["password"]    = image->password();
    ventry["url"]         = image->url();
    ventry["group"]       = image->group();

    if (_checksumError || _setupError )
        ventry["bootable"]    = false;

    QString backup        = image->supportsBackup();
    if (backup == "true")
        ventry["supports_backup"] = true;
    else if (backup == "false")
        ventry["supports_backup"] = false;
    else if (backup == "update")
        ventry["supports_backup"] = "update";

    QString iconfilename  = image->folder()+"/"+CORE(image->flavour())+".png";
    iconfilename.replace(" ", "_");
    if (QFile::exists(iconfilename))
    {
        if (iconfilename.startsWith("/tmp/media/"))
        {
            /* Copy icon to settings folder, as USB storage may take longer to get ready on boot */
            QDir dir;
            QString dirname = "/settings/os/"+CORE(image->flavour()).replace(" ", "_");
            dir.mkpath(dirname);
            QFile::copy(iconfilename, dirname+"/icon.png");
            iconfilename = dirname+"/icon.png";
        }

        ventry["icon"] = iconfilename;
    }
    else if (QFile::exists(image->folder()+"/icon.png"))
        ventry["icon"] = image->folder()+"/icon.png";

    if (_partition)
    {
        installed_os.append(ventry);
    }
    else
    {
        int i=0;
        //If we are not replacing a dfferent OS, search for the same name to replace
        if (image->replacedName().isEmpty())
            image->setReplacedName(image->flavour());

        foreach (QVariant v, installed_os)
        {
            QVariantMap m = v.toMap();

            //We use getDevice() to ensure both partition references are in /dev/xxx format, thus removing PARTUUIDS
            if ( (m.value("name").toString() == image->replacedName()) &&
                 (getDevice(m.value("partitions").toList().at(0).toString())  == getDevice(image->partitions()->at(0)->partitionDevice())) )
            {
                installed_os.replace(i,ventry);
            }
            i++;
        }
    }
    Json::saveToFile("/settings/installed_os.json", installed_os);

    return QMessageBox::AcceptRole;
}

void MultiImageWriteThread::postInstallConfig(OsInfo *image, const QString &part, const QString &customName)
{
    const QString folder = image->folder();

    QString cmdline = getFileContents("/proc/cmdline");
    QByteArray searchFor = "configpath="; //This will override the default location
    if (cmdline.contains(searchFor))
    {
        int searchForLen = searchFor.length();
        int pos = cmdline.indexOf(searchFor);
        int end = cmdline.indexOf(' ', pos+searchForLen);
        if (end != -1)
            end = end-pos-searchForLen;
        _srcFolder = cmdline.mid(pos+searchForLen, end).trimmed();
    }
    else
    {
        _srcFolder = image->configPath(); //URL
        if (_srcFolder.isEmpty())
            _srcFolder = folder;
    }
    _dstFolder = "/mnt2";

    //Mount the newly installed partition
    if (QProcess::execute("mount "+part+" "+_dstFolder) != 0)
    {
        qDebug() << (tr("%1: Error mounting file system").arg(customName));
        return;
    }

    QString nameWithoutSpaces = customName;
    nameWithoutSpaces.replace(' ', '_');

    testForCustomFile(nameWithoutSpaces, ".tar");
    testForCustomFile(nameWithoutSpaces, ".tar.xz");
    testForCustomFile(nameWithoutSpaces, ".txt");

    QProcess::execute("umount "+_dstFolder);
}

QStringList MultiImageWriteThread::parseQuotedString(const QString &tarfile, int nArgs)
{
    QStringList argList;
    QString arg;
    int inQuote=0;
    int inArg=0;
    int pos;

    for (pos=0; pos < tarfile.size(); pos++)
    {
        int add=0;
        int inc=0;
        QChar c = tarfile.at(pos);

        switch(c.toAscii())
        {
        case '\"':
            inQuote = 1-inQuote;
            if (inQuote == 0)
                inc =1; //indicate end of an argument
            else
                inArg=1; //indicate we have started an argument
            break;

        case ' ':
            if (inArg==1)
            {
                if (inQuote==0)
                {
                    inc=1; //indicate end of an argument
                }
                else
                {
                    add=1; //add embedded space to argument
                }
            }
            break;
        default:
            inArg=1;
            add=1;
            break;
        }
        if (add==1)
            arg+=c;
        if (inc==1)
        {
           argList.append(arg);
           inArg=0;
           arg="";
        }
    }
    if (inArg==1)
    {
        if (arg=="#")
            arg="";
        argList.append(arg);
        inArg=0;
        arg="";
    }

    while (argList.count()>nArgs)
        argList.removeLast();

    while (argList.count()<nArgs)
        argList.append("");

    return(argList);
}

void MultiImageWriteThread::testForCustomFile(const QString &baseName, const QString &ext)
{
    //Try to process a customName.tar file
    QString testfile = _srcFolder+"/"+baseName+ext;
    QFileInfo fi(testfile);
    //if (fi.exists()) #We can't test a URL for existence this way
    {
        QString tarfile = "@"+ baseName +ext;
        processEntry(_srcFolder, tarfile);
    }
}

void MultiImageWriteThread::processEntry(const QString &srcFolder, const QString &entry1)
{
    TRACE
    if (entry1.length()>0 && entry1.at(0)=='@')
    {
        DBG( "Processing " + entry1 + " in " + srcFolder);
        //This file needs processing

        QString entry = entry1.mid(1);  //Remove the @

        if (entry.startsWith('/'))
        {
            entry = _srcFolder+"/"+entry.mid(1);   //reset to root _srcFolder
            DBG("using srcfolder: " + entry);
        }
        else if (entry.startsWith("push="))
        {
            stack.append(_srcFolder);
            _srcFolder = entry.mid(5).trimmed();
            DBG("Changed srcFolder to "+_srcFolder);
            return;
        }
        else if (entry == "pop")
        {
            if (!stack.isEmpty())
            {
                _srcFolder = stack.takeLast();
                DBG("Restored srcFolder to "+_srcFolder);
            }
            return;
        }
        else if (!isURL(entry))
        {
            entry = srcFolder+"/"+entry;    //keep relative folder
            DBG("Using relative folder: " + entry);
        }

        QFileInfo fi(entry);
        if (fi.completeSuffix() == "txt")
        {   //Read file & process each line (no additional fields).
            QStringList lines;
            if (isURL(fi.filePath()))
            {
                DBG("Reading remote: @" + fi.filePath());
                lines = QString(getRemoteFile(fi.filePath())).split('\n');
            }
            else
            {
                DBG("Reading local: @" + fi.filePath());
                lines = QString(getFileContents(fi.filePath())).split('\n'); //QString()?
            }

            foreach (QString line, lines)
            {
                if ((line.length()>1) && (line.at(0)!='#'))
                {
                    processEntry(fi.path(),line);
                }
            }
        }
        else
        {   //assume it is a compressed or plain tar file
            untar (entry, "sha512sum", "", true);
        }
    }
    else
    {   //Just a normal file so copy it
        DBG( "Copying " + entry1 + " in " + srcFolder);
        QStringList args = parseQuotedString(entry1,5);
        QString tarfile = args.at(0);
        QString dstSubFolder = args.at(1);
        QString attributes = args.at(2);
        QString user = args.at(3);
        QString group = args.at(4);
        QString cmd;
        QString ug;
        QString fname;

        QString entry = tarfile;

        if (entry.startsWith('/'))
        {
            entry = _srcFolder+entry;   //reset to root _srcFolder
            DBG("using srcfolder: " + entry);
        }
        else if (!isURL(entry))
        {
            entry = srcFolder+"/"+entry;    //keep relative folder if not a URL
            DBG("Using relative folder: " + entry);
        }

        QFileInfo fi(entry);
        fname=fi.fileName();

        if (!isURL(entry))
        {
            if (fi.exists())
            {
                DBG( "Copying file: " + fname);

                QString dstfolder = _dstFolder + dstSubFolder;
                if (dstfolder.at( dstfolder.size()-1) !='/')
                    dstfolder += "/";
                QProcess::execute("mkdir -p " + _dstFolder + dstSubFolder);
                cmd = "cp " + entry + " " + dstfolder + fname;
                DBG(cmd);
                QProcess::execute(cmd);
            }
            else
            {
                qDebug() << "WARNING: Source file: "<< entry << " does not exist";
            }
        }
        else
        { //URL
            DBG( "Downloading file: " + entry);
            QString cmd = "wget --retry-connrefused  --read-timeout=120 --no-verbose --tries=inf -O "+_dstFolder + dstSubFolder + "/" +fname + " " +entry;
            DBG(cmd);
            QProcess::execute(cmd);
        }

        //Set attributes
        if (attributes != "")
        {
            cmd = "chmod "+attributes+ " "+ _dstFolder + dstSubFolder+"/"+fname;
            QProcess::execute(cmd);
        }
        //Set ownership
        if (user!="")
            ug=user;
        else
            ug="";

        if (group!="")
            ug=ug+":"+group;

        if (ug != "")
        {
            cmd = "chown "+ug + " " + _dstFolder + dstSubFolder + "/" + fname;
            QProcess::execute(cmd);
        }
    }
}

bool MultiImageWriteThread::mkfs(const QByteArray &device, const QByteArray &fstype, const QByteArray &label, const QByteArray &mkfsopt)
{
    TRACE
    QString cmd;

    if (fstype == "fat" || fstype == "FAT")
    {
        cmd = "/sbin/mkfs.fat ";
        if (!label.isEmpty())
        {
            cmd += "-n "+makeLabelUnique(label, 11, device)+" ";
        }
    }
    else if (fstype == "ext4")
    {
        cmd = "/usr/sbin/mkfs.ext4 ";
        if (!label.isEmpty())
        {
            cmd += "-L "+makeLabelUnique(label, 16, device)+" ";
        }
    }
    else if (fstype == "ntfs")
    {
        cmd = "/sbin/mkfs.ntfs --fast ";
        if (!label.isEmpty())
        {
            cmd += "-L "+makeLabelUnique(label, 32, device)+" ";
        }
    }

    if (!mkfsopt.isEmpty())
        cmd += mkfsopt+" ";

    cmd += device;

    qDebug() << "Executing:" << cmd;
    QProcess p;
    p.setProcessChannelMode(p.MergedChannels);
    p.start(cmd);
    p.closeWriteChannel();
    p.waitForFinished(-1);

    if (p.exitCode() != 0)
    {
        emit error(tr("Error creating file system")+"\n"+p.readAll());
        return false;
    }

    return true;
}

bool MultiImageWriteThread::isLabelAvailable(const QByteArray &label, const QByteArray &device)
{
    int result;
    QString part = readexec(1, "/sbin/findfs LABEL="+label, result);
    if (!result)
    {   // label found - already exists
        if ( !device.isEmpty() && (part==device))
        {   //A device is specified and it matches part
            result=1;   //pretend it wasn't found
            return ( result != 0);
        }
    }
    return ( result != 0);
}


QMessageBox::ButtonRole  MultiImageWriteThread::untar(const QString &tarball, const QString &csumType, const QString &csum, bool bSuppressError)
{
    TRACE

    QFile f("/tmp/fifo");
    if (f.exists())
        f.remove();
    mkfifo("/tmp/fifo", S_IRUSR | S_IWUSR);

    QString cmd = "sh -o pipefail -c \"";

    QString tarballPath = QString(tarball);
    if (isURL(tarball)){
        tarballPath = QUrl::fromUserInput(tarball).toString(QUrl::RemoveQuery);
        cmd += "wget  --retry-connrefused  --read-timeout=120 --tries=inf --no-verbose -O- "+tarball;
    }
    else
        cmd += "cat "+tarball;

    cmd += " | tee /tmp/fifo";

    if (tarballPath.endsWith(".gz"))
    {
        cmd += " | gzip -dc";
    }
    else if (tarballPath.endsWith(".xz"))
    {
        cmd += " | xz -dc";
    }
    else if (tarballPath.endsWith(".bz2"))
    {
        cmd += " | bzip2 -dc";
    }
    else if (tarballPath.endsWith(".lzo"))
    {
        cmd += " | lzop -dc";
    }
    else if (tarballPath.endsWith(".zip"))
    {
        /* Note: the image must be the only file inside the .zip. {This does not make sense for a tarball!} */
        cmd += " "; /* Actually, if we just use bsdtar to unzip it, we may be able to unzip multiple files */
    }
    else if (tarballPath.endsWith(".tar"))
    {
        /* Note: No decompression needed for a plain tar file */
        ;
    }
    else
    {
        emit error(tr("Unknown compression format file extension. Expecting .lzo, .gz, .xz, .bz2 or .zip."));
        return QMessageBox::RejectRole ;
    }

    cmd += " | bsdtar -xf - -C /mnt2 ";

    struct statfs st;
    if (statfs("/mnt2", &st) == 0 && st.f_type == MSDOS_SUPER_MAGIC)
    {
        /* File system does not support uid/gid, tell bsdtar not to set those or it will error out */
        cmd += " --no-same-owner ";
    }

    cmd += " | "+ csumType +" /tmp/fifo > /tmp/sha1.out.txt";

    cmd += "\"";

    QTime t1;
    t1.start();
    qDebug() << "Executing:" << cmd;

    QProcess p;
    p.setProcessChannelMode(p.MergedChannels);
    p.start(cmd);
    p.closeWriteChannel();
    p.waitForFinished(-1);

    f.remove(); //rm /tmp/fifo";

    if (p.exitCode() == 0)
    { //download was ok
        QString csum_download = getFileContents("/tmp/sha1.out.txt");
        csum_download=csum_download.split(" ").first();
        qDebug() << "Calculated Checksum="<<csum_download;
        if (csum != "")
        {
            qDebug()<< "Expected csum= "<<csum;
            if (csum_download != csum)
            {
                QMessageBox::ButtonRole answer;
                QString msg = tr("An incorrect file checksum has been detected in %1").arg(tarball);
                QString title = tr("Checksum error");
                qDebug() << msg;
                _checksumError++;

                emit checksumError( msg, title, &answer); //@@

                if (answer != QMessageBox::AcceptRole)
                    return(answer);
            }
            else
                emit statusUpdate(tr("Checksum OK"));
        }
        else
            emit statusUpdate(tr("Finished downloading"));
    }

    if (p.exitCode() != 0)
    {
        QByteArray msg = p.readAll();
        qDebug() << msg;
        //if (!bSuppressError)
        //S    emit error(tr("Error downloading or extracting tarball")+"\n"+msg);
        return QMessageBox::RejectRole ;
    }
    qDebug() << "finished writing filesystem in" << (t1.elapsed()/1000.0) << "seconds";

    return QMessageBox::AcceptRole;
}

QMessageBox::ButtonRole MultiImageWriteThread::dd(const QString &imagePath, const QString &csumType, const QString &csum, const QString &device)
{
    TRACE
    QFile f("/tmp/fifo");
    if (f.exists())
        f.remove();
    mkfifo("/tmp/fifo", S_IRUSR | S_IWUSR);

    QString cmd = "sh -o pipefail -c \"";

    if (isURL(imagePath))
        cmd += "wget  --retry-connrefused --read-timeout=120 --tries=inf --no-verbose -O- "+imagePath;
    else
        cmd += "cat "+imagePath;

    cmd += " | tee /tmp/fifo";

    if (imagePath.endsWith(".gz"))
    {
        cmd += " | gzip -dc";
    }
    else if (imagePath.endsWith(".xz"))
    {
        cmd += " | xz -dc";
    }
    else if (imagePath.endsWith(".bz2"))
    {
        cmd += " | bzip2 -dc";
    }
    else if (imagePath.endsWith(".lzo"))
    {
        cmd += " | lzop -dc";
    }
    else if (imagePath.endsWith(".zip"))
    {
        /* Note: the image must be the only file inside the .zip */
        cmd += " | unzip -p";
    }
    else
    {
        emit error(tr("Unknown compression format file extension. Expecting .lzo, .gz, .xz, .bz2 or .zip."));
        return QMessageBox::RejectRole;
    }

    cmd += " | dd of="+device+" conv=fsync obs=4M";

    cmd += " | "+ csumType +" /tmp/fifo > /tmp/sha1.out.txt;\"";

    QTime t1;
    t1.start();
    qDebug() << "Executing:" << cmd;

    QProcess p;
    p.setProcessChannelMode(p.MergedChannels);
    p.start(cmd);
    p.closeWriteChannel();
    p.waitForFinished(-1);

    f.remove(); //rm /tmp/fifo";

    if (p.exitCode() == 0)
    { //download was ok
        QString csum_download = getFileContents("/tmp/sha1.out.txt");
        csum_download=csum_download.split(" ").first();
        qDebug() << "Calculated Checksum="<<csum_download;
        if (csum != "")
        {
            qDebug()<< "Expected csum= "<<csum;
            if (csum_download != csum)
            {
                QMessageBox::ButtonRole answer;
                QString msg = tr("An incorrect file checksum has been detected in %1").arg(imagePath);
                qDebug() << msg;
                _checksumError++;

                emit checksumError( msg,tr("Checksum error"), &answer);

                if (answer != QMessageBox::AcceptRole)
                    return(answer);
            }
        }
    }

    if (p.exitCode() != 0)
    {
        emit error(tr("Error downloading or writing OS to SD card")+"\n"+p.readAll());
        return QMessageBox::RejectRole;
    }
    qDebug() << "finished writing filesystem in" << (t1.elapsed()/1000.0) << "seconds";

    return QMessageBox::AcceptRole;
}

QMessageBox::ButtonRole MultiImageWriteThread::partclone_restore(const QString &imagePath, const QString &csumType, const QString &csum, const QString &device)
{
    TRACE
    QFile f("/tmp/fifo");
    if (f.exists())
        f.remove();
    mkfifo("/tmp/fifo", S_IRUSR | S_IWUSR);

    QString cmd = "sh -o pipefail -c \"";

    if (isURL(imagePath))
        cmd += "wget --retry-connrefused --read-timeout=120 --tries=inf --no-verbose --tries=inf -O- "+imagePath;
    else
        cmd += " "+imagePath;

    cmd += " | tee /tmp/fifo";

    if (imagePath.endsWith(".gz"))
    {
        cmd += " | gzip -dc";
    }
    else if (imagePath.endsWith(".xz"))
    {
        cmd += " | xz -dc";
    }
    else if (imagePath.endsWith(".bz2"))
    {
        cmd += " | bzip2 -dc";
    }
    else if (imagePath.endsWith(".lzo"))
    {
        cmd += " | lzop -dc";
    }
    else if (imagePath.endsWith(".zip"))
    {
        /* Note: the image must be the only file inside the .zip */
        cmd += " | unzip -p";
    }
    else
    {
        emit error(tr("Unknown compression format file extension. Expecting .lzo, .gz, .xz, .bz2 or .zip."));
        return QMessageBox::RejectRole;
    }

    cmd += " | partclone.restore -q -s - -o "+device;

    cmd += " | "+ csumType +" /tmp/fifo > /tmp/sha1.out.txt;\"";

    QTime t1;
    t1.start();
    qDebug() << "Executing:" << cmd;

    QProcess p;
    p.setProcessChannelMode(p.MergedChannels);
    p.start(cmd);
    p.closeWriteChannel();
    p.waitForFinished(-1);

    f.remove(); //rm /tmp/fifo";

    if (p.exitCode() == 0)
    { //download was ok
        QString csum_download = getFileContents("/tmp/sha1.out.txt");
        csum_download=csum_download.split(" ").first();
        qDebug() << "Calculated Checksum="<<csum_download;
        if (csum != "")
        {
            qDebug()<< "Expected csum= "<<csum;
            if (csum_download != csum)
            {
                QMessageBox::ButtonRole answer;
                QString msg = tr("An incorrect file checksum has been detected in %1").arg(imagePath);
                qDebug() << msg;
                _checksumError++;

                emit checksumError( msg,tr("Checksum error"), &answer);

                if (answer != QMessageBox::AcceptRole)
                    return(answer);
            }
        }
    }

    if (p.exitCode() != 0)
    {
        emit error(tr("Error downloading or writing OS to SD card")+"\n"+p.readAll());
        return QMessageBox::RejectRole;
    }
    qDebug() << "finished writing filesystem in" << (t1.elapsed()/1000.0) << "seconds";

    return QMessageBox::AcceptRole;
}

void MultiImageWriteThread::patchConfigTxt()
{

        QSettings settings("/settings/noobs.conf", QSettings::IniFormat);
        int videomode = settings.value("display_mode", 0).toInt();

        QByteArray dispOptions;

        switch (videomode)
        {
        case 0: /* HDMI PREFERRED */
            dispOptions = "\r\n";   //removed for Pi4
            break;
        case 1: /* HDMI VGA */
            dispOptions = "hdmi_ignore_edid=0xa5000080\r\nhdmi_force_hotplug=1\r\nhdmi_group=2\r\nhdmi_mode=4\r\n";
            break;
        case 2: /* PAL */
            dispOptions = "hdmi_ignore_hotplug=1\r\nsdtv_mode=2\r\n";
            break;
        case 3: /* NTSC */
            dispOptions = "hdmi_ignore_hotplug=1\r\nsdtv_mode=0\r\n";
            break;
        }


        QFile f("/mnt2/config.txt");
        f.open(f.Append);
        f.write("\r\n# NOOBS Auto-generated Settings:\r\n"+dispOptions);
        f.close();

}



