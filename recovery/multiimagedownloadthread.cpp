#include "multiimagedownloadthread.h"
#include "config.h"
#include "json.h"
#include "util.h"
#include "mbr.h"
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

MultiImageDownloadThread::MultiImageDownloadThread(QObject *parent, QString local) :
    QThread(parent), _extraSpacePerPartition(0), _part(5), _local(local)
{
    /* local is "/mnt" or "/media/usb" */

}

void MultiImageDownloadThread::addImage(const QString &folder, const QString &flavour)
{
    _images.insert(folder, flavour);
}

int MultiImageDownloadThread::sizeofBootFilesInKB()
{
    QProcess proc;
    proc.start("du -s "+_local);
    proc.waitForFinished();
    return proc.readAll().split('\t').first().toInt();
}

void MultiImageDownloadThread::run()
{
    /* Calculate space requirements */
    int totalnominalsize = 0, totaluncompressedsize = 0, numparts = 0, numexpandparts = 0, numext4expandparts = 0;
    bool RiscOSworkaround = false;


    foreach (QString folder, _images.keys())
    {
        QVariantList partitions = Json::loadFromFile(folder+"/partitions.json").toMap().value("partitions").toList();
        if (partitions.isEmpty())
        {
            emit error(tr("partitions.json invalid"));
            return;
        }

        foreach (QVariant pv, partitions)
        {
            QVariantMap partition = pv.toMap();
            numparts++;
        }
    }


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

    int firstPartition = _part;

    QStringList splitted = folder.split("/");
    QString os_name   = splitted.last();

    qDebug() << "Processing OS:" << os_name;

    QVariantList partitions = Json::loadFromFile(folder+"/partitions.json").toMap().value("partitions").toList();
    foreach (QVariant pv, partitions)
    {
        QVariantMap partition = pv.toMap();
        QString tarball  = partition.value("tarball").toString();
        bool emptyfs     = partition.value("empty_fs", false).toBool();

        if (!emptyfs && tarball.isEmpty())
        {
            /* If no tarball URL is specified, What are we doing here? */

            emit error(tr("File '%1' does not need downloading").arg(tarball));
            return false;
        }

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
            return false;
        }
        qDebug() << "finished writing filesystem in" << (t1.elapsed()/1000.0) << "seconds";

        _part++;
    }


    emit statusUpdate(tr("%1: Unmounting FAT partition").arg(os_name));
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

QString MultiImageDownloadThread::getDescription(const QString &folder, const QString &flavour)
{
    if (QFile::exists(folder+"/flavours.json"))
    {
        QVariantMap v = Json::loadFromFile(folder+"/flavours.json").toMap();
        QVariantList fl = v.value("flavours").toList();

        foreach (QVariant f, fl)
        {
            QVariantMap fm  = f.toMap();
            if (fm.value("name").toString() == flavour)
            {
                return fm.value("description").toString();
            }
        }
    }
    else if (QFile::exists(folder+"/os.json"))
    {
        QVariantMap v = Json::loadFromFile(folder+"/os.json").toMap();
        return v.value("description").toString();
    }

    return "";
}
