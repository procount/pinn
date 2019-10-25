#include "util.h"
#include "mbr.h"
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QList>
#include <QtEndian>

/*
 * Convenience functions
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 */

char out[MAXMSG]={0};
char in[MAXMSG]={0};
char key[MAXMSG]={0};
size_t outsize=0;
size_t insize=0;
size_t progress=0;
size_t keysize=0;

QByteArray getFileContents(const QString &filename)
{
    QByteArray r;
    QFile f(filename);
    f.open(f.ReadOnly);
    r = f.readAll();
    f.close();

    return r;
}

QByteArray getRemoteFile(const QString &url)
{
    QProcess p;
    QString cmd;
    QByteArray result;
    cmd = "wget --no-verbose --tries=inf -O- "+url;

    //p.setProcessChannelMode(p.MergedChannels);
    p.start(cmd);
    p.closeWriteChannel();
    p.waitForFinished(-1);

    if (p.exitCode() != 0)
        result="";
    else
        result =p.readAllStandardOutput();
    return(result);
}


void putFileContents(const QString &filename, const QByteArray &data)
{
    QFile f(filename);
    f.open(f.WriteOnly);
    f.write(data);
    f.close();
}

bool backupFile(const QString &filename, const QString &ext)
{
    QString backupName = filename + "." + ext;

    if (QFile::exists(backupName))
    {
        QFile::remove(backupName);
    }
    return  QFile::rename(filename,backupName);
}

/* Utility function to query current overscan setting */
#define VCMSG_GET_OVERSCAN 0x0004000a
#define VCMSG_SET_OVERSCAN 0x0004800a
#define IOCTL_MBOX_PROPERTY _IOWR(100, 0, char *)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

struct vc_msg_overscan {
    uint32_t msg_size; /* sizeof(struct vc_msg_overscan) */
    uint32_t request_code;
    uint32_t tag_id;
    uint32_t buffer_size; /* 16 bytes */
    uint32_t request_size;
    uint32_t top;
    uint32_t bottom;
    uint32_t left;
    uint32_t right;
    uint32_t end_tag; /* an end identifier (NULL) */
} __attribute__ ((packed));

void getOverscan(int &top, int &bottom, int &left, int &right)
{
    int fd;
    vc_msg_overscan msg = {0};

    if (!QFile::exists("/dev/mailbox"))
        QProcess::execute("mknod /dev/mailbox c 100 0");

    fd = ::open("/dev/mailbox", 0);
    if (!fd)
    {
        qDebug() << "Error opening mailbox";
    }
    else
    {
        msg.msg_size = sizeof(msg);
        msg.tag_id = VCMSG_GET_OVERSCAN;
        msg.buffer_size = 16;
        if (ioctl(fd, IOCTL_MBOX_PROPERTY, &msg) != 0)
        {
            qDebug() << "Error getting mailbox property";
        }
        else
        {
            top = msg.top;
            left = msg.left;
            right = msg.right;
            bottom = msg.bottom;
        }

        ::close(fd);
    }
}

bool nameMatchesRiscOS(const QString &name)
{
    return name.contains("risc", Qt::CaseInsensitive);
}

static uint revision = 0;
uint readBoardRevision()
{
    if (revision == 0)
    {
        QProcess proc;
        proc.start("vcgencmd otp_dump");
        proc.waitForFinished();
        QList<QByteArray> lines = proc.readAll().split('\n');
        for (int i=0; i < lines.size(); i++)
        {
            if (lines.at(i).startsWith("30:"))
            {
                bool ok;
                revision = lines.at(i).right(8).toUInt(&ok, 16) & 0xFFFFFF;
                break;
            }
        }
    }
    return revision;
}

/* Whether this OS should be displayed in the list of bootable OSes */
bool canBootOs(const QString& name, const QVariantMap& values)
{
    /* Can't simply pull "name" from "values" because in some JSON files it's "os_name" and in others it's "name" */

    /* Check if it's explicitly not bootable */
    bool bootable = values.value("bootable", true).toBool();
    if (!bootable)
    {
        return false;
    }

    /* Data Partition isn't bootable */
    if (name == "Data Partition")
    {
        return false;
    }

    return true;
}

void setRebootPartition(QByteArray partition)
{
    putFileContents("/run/reboot_part", partition+"\n");
}

/* Returns device name for drive and partition number
 *
 * partdev("mmcblk0",1) -> mmcblk0p1
 * partdev("sda",2) -> sda2
 */
QByteArray partdev(const QString &drivedev, int nr)
{
    if (drivedev.right(1).at(0).isDigit())
        return drivedev.toAscii()+"p"+QByteArray::number(nr);
    else
        return drivedev.toAscii()+QByteArray::number(nr);
}

/* Returns /sys/class/block path for given drive and optional partition number */
QByteArray sysclassblock(const QString &drivedev, int partnr)
{
    QByteArray b;

    if (partnr == -1)
        b = drivedev.toAscii();
    else
        b = partdev(drivedev, partnr);

    if (b.startsWith("/dev/"))
        b = b.mid(5);

    return "/sys/class/block/"+ b;
}

QByteArray getLabel(const QString part)
{
    QByteArray result;
    QProcess p;
    p.start("/sbin/blkid -s LABEL -o value "+part);
    p.waitForFinished();

    if (p.exitCode() == 0)
        result = p.readAll().trimmed();

    return result;
}

QByteArray getUUID(const QString part)
{
    QByteArray result;
    QProcess p;
    p.start("/sbin/blkid -s UUID -o value "+part);
    p.waitForFinished();

    if (p.exitCode() == 0)
        result = p.readAll().trimmed();

    return result;
}

QByteArray getDiskId(const QString &device)
{
    mbr_table mbr;

    QFile f(device);
    f.open(f.ReadOnly);
    f.read((char *) &mbr, sizeof(mbr));
    f.close();

    quint32 diskid = qFromLittleEndian<quint32>(mbr.diskid);
    return QByteArray::number(diskid, 16).rightJustified(8, '0');;
}

QByteArray getPartUUID(const QString &devpart)
{
    QByteArray r;

    QRegExp partnrRx("([0-9]+)$");
    if (partnrRx.indexIn(devpart) != -1)
    {
        QString drive = devpart.left(partnrRx.pos());
        if (drive.endsWith("p"))
            drive.chop(1);

        r = "PARTUUID="+getDiskId(drive);
        int partnr = partnrRx.cap(1).toInt();
        QByteArray partnrstr = QByteArray::number(partnr, 16).rightJustified(2, '0');
        r += '-'+partnrstr;
    }

    return r;
}


QString getCsumType(const QVariantMap &partition)
{
    QStringList options;
    options << "sha512sum" <<"sha256sum" <<"sha1sum"<<"md5sum";

    foreach (QString csumType, options)
    {
        if (partition.contains(csumType))
            return(csumType);
    }
    return("");
}

QString getCsum(const QVariantMap &partition, const QString &csumType)
{
    QString csum     = partition.value(csumType, "").toString();

    if (csum.startsWith("http"))
    {
        csum = QString(getRemoteFile(csum)).split(" ").first();
    }
    return(csum);
}

int decrypt(int ch)
{
    if (keysize)
    {
        ch ^= key[progress];
        progress = (progress+1)%keysize;
    }
    return(ch);
}

void decryptblock(char * block, int len)
{
    int i;
    progress=0;
    for (i=0; i< len; i++)
    {
        *block = decrypt(*block);
        block++;
    }
}

void hexdecode(char * str, char * output, size_t * size)
{
    *size=0;
    char buff[3];
    buff[2]='\0';
    if ((strlen(str) % 2))
        return;
    while (*str)
    {
        buff[0]=*str++;
        buff[1]=*str++;
        long int num = strtol(buff, NULL, 16);
        *output++ = (char)num;
        (*size)++;
    }
}
