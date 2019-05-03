#include "util.h"
#include "mbr.h"
#include "json.h"
#include "mydebug.h"

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
#include <QStringRef>
#include <QMessageBox>
/*
 * Convenience functions
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 */

struct partid_t
{
    int start;
    int length;
    const char * delim;
};


QByteArray getFileContents(const QString &filename)
{
    QByteArray r;
    QFile f(filename);
    f.open(f.ReadOnly);
    r = f.readAll();
    f.close();

    return r;
}

void putFileContents(const QString &filename, const QByteArray &data)
{
    QFile f(filename);
    f.open(f.WriteOnly);
    f.write(data);
    f.close();
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

QString readexec(int log, const QString &cmd, int &errorcode)
{
    //NOTE: Often need to use "sh -c \"...\""

    QProcess proc;
    QString output;

    proc.setProcessChannelMode(proc.MergedChannels);
    proc.start(cmd);
    proc.waitForFinished(-1);
    errorcode = proc.exitCode();
    output = proc.readAll();

    if (log)
        qDebug() << cmd << "\n" << output << "\n";
    return (output);
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

bool nameMatchesWindows(const QString &name)
{
    return name.contains("windows_10", Qt::CaseInsensitive);
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

bool setRebootPartition(QByteArray partition)
{
    putFileContents("/run/reboot_part", partition+"\n");
    return true;
}

void reboot()
{
    QByteArray reboot_part = getFileContents("/run/reboot_part").trimmed();

    ::sync();
    // Reboot
    ::syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, reboot_part.constData());
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

bool isURL(const QString &s)
{
    return s.startsWith("http:") || s.startsWith("https:");
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

QByteArray getDevice(const QString & partuuid)
{
    QByteArray device;

    if (partuuid.left(4)=="/dev")
        device = partuuid.toAscii();
    else
    {
        QProcess p;
        p.start("/sbin/blkid -t "+partuuid+" -s UUID");
        p.waitForFinished();

        if (p.exitCode() == 0)
        {
            device = p.readAll().trimmed();
            int colonpos = device.indexOf(':');
            device = device.left(colonpos);
        }
    }
    return device;
}

QString getNickNameParts(const QString& input, eNAMEPARTS flags)
{
    TRACE
    /* Same as getNameParts(),
     * but replaces any eBASE or eFLAVOUR parts
     * with eNICKNAME if it exists (no flavour)
     */

    QString nickname;
    QString output;

    nickname = getNameParts(input, eNICKNAME);
    if (!nickname.isEmpty())
    {   //If there is a nickname, request that instead of the basename & flavour
        flags &= (eNAMEPARTS) ~eCORE;
        flags |= (eNAMEPARTS) eNICKNAME;
        output = getNameParts(input, flags);
        //Remove the '=' prefix regardless of eSPLIT, to act the same as eCORE
        if (output.left(1)=="=")
            output = output.mid(1);
    }
    else
    {
        output = getNameParts(input, flags);
    }
    return(output);
}

QString getNameParts(const QString& input, eNAMEPARTS flags)
{
    /* Splits an OS name into its consitutent parts and then
     * concatenates specific parts back together again
     * "Name - flavour=nickname#2018-12-31@1"
     */

    int i,j;
    int index;

    struct partid_t parts[eNUMPARTS]=
    {
        {-1,  -1,  ""},    //Base
        {-1,  -1,  " - "}, //Flavour
        {-1,  -1,  "="},   //NickName
        {-1,  -1,  "#"},   //Date
        {-1,  -1,  "@"}    //Partition
    };
    QString output;
    if (!input.isEmpty())
    {
        //Identify which parts are present
        parts[0].start = 0;
        for (i=1; i<eNUMPARTS; i++)
        {
            index = input.indexOf(parts[i].delim);
            if (index != -1)
            {
                parts[i].start = index;
                for (j=i-1; j>=0; j--)
                {
                    if ((parts[j].length == -1) && (parts[j].start != -1))
                        parts[j].length = index - parts[j].start;
                }
            }
        }
        for (j=i-1; j>=0; j--)
        {
            if ((parts[j].length == -1) && (parts[j].start != -1))
                parts[j].length = input.length() - parts[j].start;
        }

        for (i=0; i<eNUMPARTS; i++)
        {
            int offset = (flags & eSPLIT) ? strlen(parts[i].delim) : 0;
            if (parts[i].start != -1 && parts[i].length != -1)
            {
                QString value = QStringRef(&input,parts[i].start+offset, (int)parts[i].length-offset).toString();
                if (flags & (1<<i))
                {
                    output += value;
                }
            }
        }
    }
    return(output);
}

QStringList splitNameParts(const QString& input)
{
    QStringList list;
    /* Splits an OS name into its consitutent parts
     * "Name - flavour=nickname#2018-12-31@1"
     */

    int i,j;
    int index;

    struct partid_t parts[eNUMPARTS]=
    {
        {-1,  -1,  ""},    //Base
        {-1,  -1,  " - "}, //Flavour
        {-1,  -1,  "="},   //NickName
        {-1,  -1,  "#"},   //Date
        {-1,  -1,  "@"}    //Partition
    };

    if (!input.isEmpty())
    {
        //Identify which parts are present
        parts[0].start = 0;
        for (i=1; i<eNUMPARTS; i++)
        {
            index = input.indexOf(parts[i].delim);
            if (index != -1)
            {
                parts[i].start = index;
                for (j=i-1; j>=0; j--)
                {
                    if ((parts[j].length == -1) && (parts[j].start != -1))
                        parts[j].length = index - parts[j].start;
                }
            }
        }
        for (j=i-1; j>=0; j--)
        {
            if ((parts[j].length == -1) && (parts[j].start != -1))
                parts[j].length = input.length() - parts[j].start;
        }

        list.clear();
        for (i=0; i<eNUMPARTS; i++)
        {
            int offset = strlen(parts[i].delim);
            if (parts[i].start != -1 && parts[i].length != -1)
            {
                QString value = QStringRef(&input,parts[i].start+offset, (int)parts[i].length-offset).toString();
                list.append(value);
            }
            else
                list.append("");
        }
    }
    return(list);
}

void setNameParts(QStringList& parts, eNAMEPARTS flags, const QString& value )
{
    for (int i=0; i<eNUMPARTS; i++)
    {
        if (flags & (1<<i))
        {
            parts[i]=value;
        }
    }
}

QString joinNameParts(QStringList input)
{
    struct partid_t parts[eNUMPARTS]=
    {
        {-1,  -1,  ""},    //Base
        {-1,  -1,  " - "}, //Flavour
        {-1,  -1,  "="},   //NickName
        {-1,  -1,  "#"},   //Date
        {-1,  -1,  "@"}    //Partition
    };
    QString output;
    int i;

    for (i=0; i<eNUMPARTS; i++)
    {
        if (!input[i].isEmpty())
            output += parts[i].delim+input[i];
    }
    return(output);
}


int extractPartitionNumber(QByteArray& partition)
{
    //Partition may be in format /dev/mmcblk0pD or PARTUUID=0000ABCD-HH
    //Converts from decimal or hex accordingly

    int partitionNr=800;
    QRegExp parttype("^PARTUUID");

    if (parttype.indexIn(partition) == -1)
    {   // Old style /dev/mmcblk0pDD
        QRegExp partnrRx("([0-9]+)$");
        if (partnrRx.indexIn(partition) == -1)
        {
            QMessageBox::critical(NULL, "installed_os.json corrupt", "Not a valid partition: "+partition);
            return partitionNr;
        }
        else
        {
            partitionNr    = partnrRx.cap(1).toInt();
        }
    }
    else
    {   //USB style PARTUUID=000dbedf-XX
        QRegExp partnrRx("([0-9a-f][0-9a-f])$");
        if (partnrRx.indexIn(partition) == -1)
        {
            QMessageBox::critical(NULL, "installed_os.json corrupt", "Not a valid partition: "+partition);
            return partitionNr;
        }
        else
        {
            bool ok;
            partitionNr    = partnrRx.cap(1).toInt(&ok, 16);
        }
    }
    return partitionNr;
}

QString getDescription(const QString &folder, const QString &flavour)
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

bool updatePartitionScript(QVariantMap & entry, QListWidgetItem * witem)
{   //Entry.value("supports_backup").toString() == "update"
    TRACE
    /*
     * Read local partition_setup.sh in /settings/os/os_name
     * If 2nd line contains "backup supported"
     *    Return true
     * endif
     * find latest entry of corresponding os_name in os_list
     * get "partition_setup" url
     * Read remote partition_setup.sh script
     * if 2nd line contains "backup supported"
     *    save new partition_setup.sh to /settings/os/os_name
     *    set "supports_backup": true in installed_os.json
     *    return true
     * endif
     *
     */

    //Check if the partition_setup.sh script has already been updated.
    QString localScriptFilename = "/settings/os/"+entry.value("name").toString()+"/partition_setup.sh";
    QByteArray localContents = getFileContents(localScriptFilename);
    if (localContents.contains("supports_backup"))
    {
        qDebug() << localScriptFilename << " Supports backups";
        //qDebug() << localContents;
        return (true);
    }

    if (witem)
    {   //An installable version of this OS is accessible
        QByteArray scriptContents;
        QString fileLocation;
        QVariantMap newEntry = witem->data(Qt::UserRole).toMap();

        DBG("Checking new OS");
        //Check if OS is local or remote
        if (!newEntry.contains("folder"))
        {   //Installable OS is remote
            DBG("remote");
            fileLocation = newEntry.value("partition_setup").toString();
            scriptContents = getRemoteFile(fileLocation);
        }
        else
        {   //Copy files from local storage to /settings folder
            DBG("Local");
            fileLocation = newEntry.value("folder").toString()+"/partition_setup.sh";;
            scriptContents = getFileContents(fileLocation);
        }
        if (scriptContents.contains("supports_backup"))
        {
            qDebug() << "Copying new partition_setup.sh from " << fileLocation;
            putFileContents(localScriptFilename,scriptContents);
            return (true);
        }
        qDebug() << "No new partition_setup.sh script found";
        return(false);
    }
    qDebug() << "No OS installation found";
    return false;
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
