#include "backupthread.h"
#include "config.h"
#include "json.h"
#include "util.h"
#include "mbr.h"
#include "partitioninfo.h"
#include "osinfo.h"
#include "mydebug.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSettings>
#include <QTime>
#include <QVariantList>
#include <QVariantMap>
#include <unistd.h>
#include <linux/fs.h>
#include <sys/ioctl.h>


BackupThread::BackupThread(QObject *parent, QString local) :
    QThread(parent),  _local(local), _extraSpacePerPartition(0), _part(5)
{
    /* local is "/tmp/media/sd*" or "/mnt" (in future) */
}

void BackupThread::addImage(const QVariantMap &entry)
{
    _images.append(entry);
}

void BackupThread::run()
{
    /* Calculate space requirements */
    quint64 totalDownloadSize = 0;
    foreach (QVariantMap entry, _images)
    {
        quint64 downloadSize = entry.value("backupsize").toULongLong();
        totalDownloadSize += downloadSize;
    }

    emit parsedImagesize(totalDownloadSize);

    /* Process each image */
    foreach (QVariantMap entry, _images)
    {
        // for each partition
        if (!processImage(entry))
            return;
    }

    emit statusUpdate(tr("Finish writing (sync)"));
    sync();
    emit completed();
}

#define copyentry(key1,key2) \
    if (entry.contains(key2)) \
        ventry[key1] = entry.value(key2)

bool BackupThread::processImage(const QVariantMap & entry)
{
    MYDEBUG
    qDebug() << entry;

    QString backupFolder = entry.value("backupFolder").toString();

    //Read the os.json and partitions.json files into pInfo
    OsInfo * pOsInfo = new OsInfo(backupFolder, "", this);
    QList<PartitionInfo *> *partitions = pOsInfo->partitions();
    int i=0;
    QVariantList partdevices = entry.value("partitions").toList();
    qint64 downloadSize=0L;

    QDir dir;
    if (!dir.exists("/tmp/src"))
        dir.mkdir("/tmp/src");

    foreach (PartitionInfo * pPart, *partitions)
    {
        QString label = pPart->label();
        QString dev = partdevices[i].toString();
        QByteArray fstype   = pPart->fsType();

        //Check for fstype=="raw" ->image file. Check partition size is still the same and hasn't been expanded
        //Check for emptyfs. Is it still empty? If not, remove attribute.
        //Add "supports_archive" to prevent unsupported OSes from being backed up.
        //If it was raw or empty & change to fat or ext4 tar file, change partition parameters.
        //what if partclone?

        //   Mount it
        QProcess::execute("mount -o ro "+dev+" /tmp/src");
        emit newDrive(dev);
        //   tar gzip
        QString cmd = "sh -c \"cd /tmp/src; tar -c . | gzip > "+ backupFolder+"/"+label+".tar.gz\"";
        qDebug()<<cmd;
        QProcess::execute(cmd);
        QFileInfo fi(backupFolder+"/"+label+".tar.gz");
        downloadSize += fi.size();
        //   UnMount it
        QProcess::execute("umount /tmp/src");
        i++;
    }

    dir.rmdir("/tmp/src");

    //Update JSON files
    //os.json
    QVariantMap ventry = Json::loadFromFile(backupFolder+"/os.json").toMap();
    copyentry("name","name");
    //  name = copied from installed_os.json (to include nickname etc. + backup date
    copyentry("name", "backupName");
    copyentry("description","description");
    copyentry("group","group");
    copyentry("password","password");
    copyentry("release_date","release_date");
    copyentry("username","username");
    //  download_size = sum of tar.gz files sizes in bytes?
    ventry["download_size"] = downloadSize;
    // no icon entry to use default
    Json::saveToFile(backupFolder+"/os.json", ventry);

    //partitions.json
    QVariantList tarballsizes = entry.value("partsizes").toList(); //in MB
    QVariantMap backupmap = Json::loadFromFile(backupFolder+"/partitions.json").toMap();
    if (backupmap.contains("partitions"))
    {
        QVariantList backuplist = backupmap.value("partitions").toList();
        for (int i=0; i<backuplist.count(); i++)
        {
            QVariantMap pmap = backuplist[i].toMap();
            pmap.remove("tarball");
            pmap["uncompressed_tarball_size"] = tarballsizes[i].toULongLong();
            if (i==0)
                pmap["partition_size_nominal"] = tarballsizes[i].toULongLong()+100; //extra 100MB for boot
            else
                pmap["partition_size_nominal"] = tarballsizes[i].toULongLong()+500; //extra 500MB for other partitions
            backuplist[i] = pmap;
        }
        backupmap["partitions"] = backuplist;
    }
    Json::saveToFile(backupFolder+"/partitions.json", backupmap);

    return(true);
}


