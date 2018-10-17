#include "backupthread.h"
#include "config.h"
#include "json.h"
#include "util.h"
#include "mbr.h"
#include "partitioninfo.h"
#include "osinfo.h"
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
    qDebug() << entry;

    QString backupFolder = entry.value("backupFolder").toString();

    //Read the os.json and partitions.json files into pInfo
    OsInfo * pOsInfo = new OsInfo(backupFolder, "", this);
    QList<PartitionInfo *> *partitions = pOsInfo->partitions();
    int i=0;
    QVariantList partdevices = entry.value("partitions").toList();
    QVariantList gzsize;

    QDir dir;
    if (!dir.exists("/tmp/src"))
        dir.mkdir("/tmp/src");

    foreach (PartitionInfo * pPart, *partitions)
    {
        QString label = pPart->label();
        QString dev = partdevices[i].toString();

        //   Mount it
        QProcess::execute("mount -o ro "+dev+" /tmp/src");
        emit newDrive(dev);
        //   tar gzip
        QString cmd = "sh -c \"tar -c /tmp/src/ | gzip > "+ backupFolder+"/"+label+".tar.gz\"";
        qDebug()<<cmd;
        QProcess::execute(cmd);
        QFileInfo fi(backupFolder+"/"+label+".tar.gz");
        qint64 filesize = fi.size();
        gzsize.append(filesize);
        //   UnMount it
        QProcess::execute("umount /tmp/src");
        i++;
    }

    dir.rmdir("/tmp/src");

    //Update JSON files
    //os.json
    QVariantMap ventry;
    copyentry("name","name");
    //  name = copied from installed_os.json (to include nickname etc. + backup date
    copyentry("name", "backupName");
    copyentry("description","description");
    copyentry("feature_level","feature_level");
    copyentry("group","group");
    copyentry("kernel","kernel");
    copyentry("password","password");
    copyentry("release_date","release_date");
    copyentry("supported_hex_revisions","supported_hex_revisions");
    copyentry("supported_models","supported_models");
    copyentry("url","url");
    copyentry("username","username");
    copyentry("version","version");
    //  download_size = sum of tar.gz files sizes in bytes?
    copyentry("download_size","backupsize");
    //  icon=icon.png
    ventry["icon"] = "icon.png";
    Json::saveToFile(backupFolder+"/os.json", ventry);

    //partitions.json
    QVariantList tarballsizes = entry.value("partsizes").toList();
    QVariantMap backupmap = Json::loadFromFile(backupFolder+"/partitions.json").toMap();
    if (backupmap.contains("partitions"))
    {
        QVariantList backuplist = backupmap.value("partitions").toList();
        int i=0;
        foreach (QVariant sizeentry, gzsize)
        {
            QVariantMap pmap = backuplist[i].toMap();
            pmap["uncompressed_tarball_size"] = gzsize[i];
            if (i==0)
                pmap["partition_size_nominal"] = tarballsizes[i].toULongLong()+100; //extra 100MB for boot
            else
                pmap["partition_size_nominal"] = tarballsizes[i].toULongLong()+500; //extra 500MB for other partitions
            i++;
        }
    }
    Json::saveToFile(backupFolder+"/partitions.json", backupmap);

    //  uncompressed_tarball_size = size of tar file in MB
    //  partition_size_nominal = uncompressed_tarball_size + 100MB?



    return(true); //@@ STUB IT OUT

#if 0
    // get disk sizes with df
    // update partitions.json

    QString folder; //@@
    QStringList splitted = folder.split("/");
    QString os_name   = splitted.last();


    qDebug() << "Processing OS:" << os_name;

    QVariantList partitions = Json::loadFromFile(folder+"/partitions.json").toMap().value("partitions").toList();
    foreach (QVariant pv, partitions)
    {
        QVariantMap partition = pv.toMap();
        QString tarball  = partition.value("download").toString();
        bool emptyfs     = partition.value("empty_fs", false).toBool();

        if (emptyfs)
        {
            continue;
        }
        else if (tarball.isEmpty())
        {
            /* If no tarball URL is specified, What are we doing here? */

            emit error(tr("File '%1' does not need downloading").arg(tarball));
            return (false);
        }

        // Get the full pathname of the destination file
        QStringList split_tar = tarball.split("/");
        QString filename   = folder;
        if (filename.right(1) !="/")
            filename += "/";
        filename += split_tar.last();

        // If it already exists, delete it to avoid .1 .2 filenames
        QFile f(filename);
        if (f.exists())
            f.remove();

        /* Download the compressed tarball */
        //"sh -o pipefail -c \"";
        QString cmd = "wget --no-verbose --tries=inf --directory-prefix "+folder+" "+tarball;

        QTime t1;
        t1.start();
        qDebug() << "Executing:" << cmd;

        QProcess p;
        p.setProcessChannelMode(p.MergedChannels);
        p.start(cmd);
        p.closeWriteChannel();
        p.waitForFinished(-1);

        if (p.exitCode() != 0)
        {
            QByteArray msg = p.readAll();
            qDebug() << msg;
            emit error(tr("Error downloading or extracting tarball")+"\n"+msg);

            //Delete os.json to hide partial OS from PINN
            QString filename   = folder;
            if (filename.right(1) !="/")
                filename += "/";
            filename += "os.json";
            QFile g(filename);
            if (g.exists())
                g.remove();

            // return false; //@@Let's see what happens if we try to continue (maybe need to emit error with parameter to enable or not)
        }
        qDebug() << "finished downloading filesystem in" << (t1.elapsed()/1000.0) << "seconds";
        _part++;
    }

    QVariantMap json = Json::loadFromFile(folder+"/partitions.json").toMap();
    partitions = json["partitions"].toList();
    int i=0;
    foreach (QVariant pv, partitions)
    {   //Remove download URLs
        QVariantMap partition = pv.toMap();
        if (partition.contains("download")) //change to download
        {
            partition.remove("download");
            partitions[i] = partition;
        }
        i++;
    }
    json["partitions"] = partitions;
    Json::saveToFile(folder+"/partitions.json", json);
    emit statusUpdate(tr("Finished backing up %1").arg(os_name));
#endif
    return true;
}


