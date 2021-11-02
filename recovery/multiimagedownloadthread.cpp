#include "multiimagedownloadthread.h"
#include "config.h"
#include "json.h"
#include "util.h"
#include "mbr.h"
#include "partitioninfo.h"
#include "progressslideshowdialog.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSettings>
#include <QTime>
#include <unistd.h>
#include <linux/fs.h>
#include <sys/ioctl.h>

MultiImageDownloadThread::MultiImageDownloadThread(QObject *parent, QString local, QString drive) :
    QThread(parent),  _local(local), _extraSpacePerPartition(0), _part(5), _drive(drive)
{
    /* local is "/tmp/media/sd*" or "/mnt" (in future) */
    allowResume(false);
}

void MultiImageDownloadThread::addImage(const QString &folder, const QString &flavour)
{
    _images.insert(folder, flavour);
}

void MultiImageDownloadThread::allowResume(bool allow)
{
    _allowResume=allow;
}

void MultiImageDownloadThread::run()
{
    /* Calculate space requirements */
    quint64 totalDownloadSize = 0;
    emit newDrive(_drive, ePM_WRITEDF);

    foreach (QString folder, _images.keys())
    {
        quint64 downloadSize=Json::loadFromFile(folder+"/os.json").toMap().value("download_size").toULongLong();
        totalDownloadSize += downloadSize;
    }

    emit parsedImagesize(totalDownloadSize);

    /* Process each image */
    for (QMultiMap<QString,QString>::const_iterator iter = _images.constBegin(); iter != _images.constEnd(); iter++)
    {
        if (!processImage(iter.key(), iter.value()))
            return;
    }

    emit statusUpdate(tr("Finish writing (sync)"));
    sync();
    emit completed();
}

bool MultiImageDownloadThread::processImage(const QString &folder, const QString &flavour)
{
    Q_UNUSED(flavour);

    QStringList splitted = folder.split("/");
    QString os_name   = splitted.last();
    bool downloadError;

    QVariantList partitions = Json::loadFromFile(folder+"/partitions.json").toMap().value("partitions").toList();
    foreach (QVariant pv, partitions)
    {
        QVariantMap partition = pv.toMap();
        QString tarball  = partition.value("download").toString();
        bool emptyfs     = partition.value("empty_fs", false).toBool();
        QString csumType = getCsumType(partition);
        QString csum     = getCsum(partition,csumType);
        downloadError = false;

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

        QString errmsg = tr("Downloading %1:%2").arg(os_name,split_tar.last());
        qDebug() << errmsg;
        emit statusUpdate(errmsg);

        //Check for file transfer resuming
        QFile f(filename);
        if (!_allowResume)
        {
            // If it already exists, delete it to avoid .1 .2 filenames
            if (f.exists())
                f.remove();
        }

        /* Download the compressed tarball */
        //"sh -o pipefail -c \"";
        QString cmd = "wget --continue --retry-connrefused --read-timeout=20 --waitretry=1 --no-verbose --tries=inf --directory-prefix "+folder+" "+tarball;

        QTime t1;
        t1.start();
        qDebug() << "Executing:" << cmd;

        emit newDrive(_drive, ePM_WRITEDF);
        emit startAccounting();
        QProcess p;
        p.setProcessChannelMode(p.MergedChannels);
        p.start(cmd);
        p.closeWriteChannel();
        p.waitForFinished(-1);
        emit stopAccounting();

        QByteArray msg = p.readAll();

        if (p.exitCode() == 0)
        { //download was ok

            if (csumType.isEmpty())
                csumType="sha512sum";

            errmsg = tr("Checking %1:%2").arg(os_name,split_tar.last());
            qDebug() << errmsg;
            emit statusUpdate(errmsg);

            int errorcode;
            QString csum_download = readexec(1,csumType+" "+filename, errorcode).split(" ").first();

            if ((csum != "") && (csum_download != csum))
            {
                qDebug()<< "Expected csum= "<<csum;
                emit error(tr("Error in checksum"));
                downloadError = true;
            }
        }

        if (p.exitCode() != 0)
        {
            qDebug() << msg;
            emit errorContinue(tr("Error downloading or extracting tarball")+"\n"+msg);
            downloadError = true;
        }

        emit consolidate();

        if (downloadError)
        {
            //Delete os.json to hide partial OS from PINN
            QString filename   = folder;
            if (filename.right(1) !="/")
                filename += "/";
            filename += "os.json";
            QFile g(filename);
            if (g.exists())
                g.remove();
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

    emit statusUpdate(tr("Finished downloading %1").arg(os_name));

    QString Imagefile = folder+"/os.json";
    emit imageWritten(Imagefile);

    return true;
}



QByteArray MultiImageDownloadThread::getLabel(const QString part)
{
    QByteArray result;
    QProcess p;
    p.start("/sbin/blkid -s LABEL -o value "+part);
    p.waitForFinished();

    if (p.exitCode() == 0)
        result = p.readAll().trimmed();

    return result;
}

QByteArray MultiImageDownloadThread::getUUID(const QString part)
{
    QByteArray result;
    QProcess p;
    p.start("/sbin/blkid -s UUID -o value "+part);
    p.waitForFinished();

    if (p.exitCode() == 0)
        result = p.readAll().trimmed();

    return result;
}

