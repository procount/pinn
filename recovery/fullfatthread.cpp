/* Prepare SD card for first use thread
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

#include "fullfatthread.h"
#include "mbr.h"
#include "util.h"
#include "config.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <unistd.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <QtEndian>

FullFatThread::FullFatThread(const QString &drive, QObject *parent) :
    QThread(parent), _drive(drive)
{
}

void FullFatThread::run()
{
    QDir dir;

    emit statusUpdate("Waiting for SD card to be ready");
    while (!QFile::exists(_drive))
    {
        QThread::usleep(100);
    }

    emit statusUpdate(tr("Mounting FAT partition"));
    mountSystemPartition();

    if (!method_resizePartitions())
    {
        return;
    }

    emit statusUpdate(tr("Mounting FAT partition"));
    if (!mountSystemPartition())
    {
        emit error(tr("Error mounting system partition."));
        return;
    }

    QString cmdlinefilename = "/mnt/recovery.cmdline";
    if (!QFile::exists(cmdlinefilename))
        cmdlinefilename = "/mnt/cmdline.txt";

    /* Add "runinstaller" to cmdline.txt */
    QFile f(cmdlinefilename);
    if (!f.open(f.ReadOnly))
    {
        emit error(tr("Error opening %1").arg(cmdlinefilename));
        return;
    }
    QByteArray line = f.readAll().trimmed();
    line.prepend("runinstaller ");
    f.close();
    f.open(f.WriteOnly);
    f.write(line);
    f.close();

    emit statusUpdate(tr("Finish writing to disk (sync)"));
    sync();

    emit completed();
}

bool FullFatThread::method_resizePartitions()
{
    uint newStartOfRescuePartition = getFileContents(sysclassblock(_drive, 1)+"/start").trimmed().toUInt();
    uint newEndOfRescuePartition   = getFileContents(sysclassblock(_drive, -1)+"/size").trimmed().toUInt() -2;

    if (!umountSystemPartition())
    {
        emit error(tr("Error unmounting system partition."));
        return false;
    }

    emit statusUpdate(tr("Removing partitions 2,3,4"));

    QFile f(_drive);
    f.open(f.ReadWrite);
    // Seek to partition entry 2
    f.seek(462);
    // Zero out partition 2,3,4 to prevent parted complaining about invalid constraints
    f.write(QByteArray(16*3, '\0'));
    f.flush();
    // Tell Linux to re-read the partition table
    ioctl(f.handle(), BLKRRPART);
    f.close();
    QThread::msleep(500);

    emit statusUpdate(tr("Resizing FAT partition"));

    /* Relocating the start of the FAT partition is a write intensive operation
     * only move it when it is not aligned on a MiB boundary already */
    if (newStartOfRescuePartition < 2048 || newStartOfRescuePartition % 2048 != 0)
    {
        newStartOfRescuePartition = PARTITION_ALIGNMENT; /* 4 MiB */
    }

    QString cmd = "/usr/sbin/parted --script "+_drive+" resize 1 "+QString::number(newStartOfRescuePartition)+"s "+QString::number(newEndOfRescuePartition)+"s";
    qDebug() << "Executing" << cmd;
    QProcess p;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    /* Suppress parted's big fat warning about its file system manipulation code not being robust.
       It distracts from any real error messages that may follow it. */
    env.insert("PARTED_SUPPRESS_FILE_SYSTEM_MANIPULATION_WARNING", "1");
    p.setProcessEnvironment(env);
    p.setProcessChannelMode(p.MergedChannels);
    p.start(cmd);
    p.closeWriteChannel();
    p.waitForFinished(-1);

    if (p.exitCode() != 0)
    {
        emit error(tr("Error resizing existing FAT partition")+"\n"+p.readAll());
        return false;
    }
    qDebug() << "parted done, output:" << p.readAll();
    QThread::msleep(500);

    QProcess::execute("/sbin/mlabel -i "+partdev(_drive, 1)+" ::RECOVERY");

    return true;
}


bool FullFatThread::mountSystemPartition()
{
    return QProcess::execute("mount "+partdev(_drive, 1)+" /mnt") == 0 || QProcess::execute("mount "+_drive+" /mnt") == 0;
}

bool FullFatThread::umountSystemPartition()
{
    return QProcess::execute("umount /mnt") == 0;
}



