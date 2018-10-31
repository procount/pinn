#include "piclonethread.h"
#include "copythread.h"
#include <QDebug>
#include <QProcess>

#define CANCEL_CHECK
#define MAXPART 20

#define ERROR(msg) \
    qDebug() << msg; \
    emit error(msg); \
    return;

typedef struct
{
    int pnum;
    long start;
    long end;
    char ptype[10];
    char ftype[20];
    char flags[10];
    char name[32];
} partition_t;

partition_t parts[MAXPART];

/* mount points */
char src_mnt[32], dst_mnt[32];


piCloneThread::piCloneThread(QString src_dev, QString dst_dev, bool resize, QObject *parent) :
    QThread(parent), _src(src_dev), _dst(dst_dev), _resize(resize)
{
}

static int get_string (const char *cmd, char *name)
{
    FILE *fp;
    char buf[64];

    fp = popen (cmd, "r");
    if (fp == NULL || fgets (buf, sizeof (buf) - 1, fp) == NULL) return 0;
    pclose(fp);
    return sscanf (buf, "%s", name);
}

QString partition_name (QString device)
{
    if (device.startsWith("/dev/mmcblk"))
        device.append("p");
    return device;
}


void piCloneThread::run()
{
    char buffer[256], res[256];
    char cc_src[32];
    char cc_dst[32];
    int n, p;
    long  srcsz, dstsz, stime;
    qint64 prog;
    FILE *fp;

    strncpy(cc_src, _src.toUtf8().constData(), qMin (31,_src.length()));
    strncpy(cc_dst, _dst.toUtf8().constData(), qMin (31,_dst.length()));

    emit setMaxProgress(qint64(5*512));
    qDebug() << "Clonethread running....";

    sprintf (buffer, "parted %s unit s print | tail -n +4 | head -n 1", cc_src);
    fp = popen (buffer, "r");
    if (fp == NULL || fgets (buffer, sizeof (buffer) - 1, fp) == NULL)
    {
        ERROR("Unable to read source.");
    }
    pclose(fp);
    if (strncmp (buffer, "Partition Table: msdos", 22))
    {
        ERROR("Non-MSDOS partition table on source.");
        return;
    }

    CANCEL_CHECK;

    emit setMaxProgress(512);
    emit statusUpdate(tr("Preparing target...").toUtf8().constData());
    emit setProgress(0);

    // unmount any partitions on the target device
    for (n = MAXPART; n >= 1; n--)
    {
        //qDebug()<< "Unmount partition " << n;
        QProcess::execute ("sh -c \"umount "+partition_name (_dst)+ QString::number(n)+"\"");
        CANCEL_CHECK;
    }

    // wipe the FAT on the target
    qDebug() << "Wipe Partition";
    QProcess::execute ("dd if=/dev/zero of=" +_dst+" bs=512 count=1");
    CANCEL_CHECK;

    // prepare temp mount points
    strcpy(src_mnt, "/tmp/src");
    strcpy(dst_mnt,"/tmp/dst");
    QProcess::execute("mkdir -p "+QString(src_mnt));
    QProcess::execute("mkdir -p "+QString(dst_mnt));
    //qDebug() <<"src_mnt=" <<src_mnt;
    //qDebug() <<"dst_mnt=" <<dst_mnt;
    // prepare the new FAT
    qDebug() << "mklabel";
    QProcess::execute ("sh -c \"parted -s " + _dst + " mklabel msdos\"");
    CANCEL_CHECK;

    emit setMaxProgress(512);
    emit statusUpdate(tr("Reading Partitions...").toUtf8().constData());
    emit setProgress(0);


    // read in the source partition table
    n = 0;
    //QString cmd = "parted "+_src+" unit s print | tail -n +8 | head -n -1";
    sprintf (buffer, "parted %s unit s print | tail -n +7 | head -n -1", cc_src);   //Changed 8 to 7 Why?

    fp = popen (buffer, "r");
    if (fp != NULL)
    {
        while (1)
        {
            if (fgets (buffer, sizeof (buffer) - 1, fp) == NULL) break;
            if (n<MAXPART)
            {
                sscanf (buffer, "%d %lds %lds %*ds %s %s %s", &(parts[n].pnum), &(parts[n].start),
                    &(parts[n].end), &(parts[n].ptype[0]), &(parts[n].ftype[0]), &(parts[n].flags[0]));
                //qDebug() << buffer;
                qDebug() << parts[n].pnum << parts[n].start << (parts[n].end) << (parts[n].ptype) << (parts[n].ftype) << (parts[n].flags);
                //Just counting the number of partitions
                n++;
            }
            else
            {
                ERROR("Too many partitions");
            }
        }
        pclose(fp);
    }
    CANCEL_CHECK;

    //read partition names
    for (p = 0; p < n; p++)
    {
        QString req_name = "lsblk -o label "+partition_name(_src) + QString::number(parts[p].pnum)+ " | tail -n +2";
        get_string(req_name.toUtf8().constData(), parts[p].name);
        qDebug() << "PartName " << p << parts[p].name;
    }

    //gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress), 1.0);
    //gtk_label_set_text (GTK_LABEL (status), _("Preparing partitions..."));
    //gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress), 0.0);
    emit setMaxProgress(n*512);
    emit statusUpdate(tr("Preparing Partitions...").toUtf8().constData());
    emit secondaryUpdate("");
    emit setProgress(0);

    // recreate the partitions on the target
    for (p = 0; p < n; p++)
    {
        QString display = tr("Creating partition %1 of %2 ").arg(QString::number(p+1), QString::number(n));
        // create the partition
        if (!strcmp (parts[p].ptype, "extended"))
        {
            qDebug() << "Creating extended partition" << p;
            display.append("(extended)");
            emit secondaryUpdate(display);
            if (int result = QProcess::execute("sh -c \"parted -s "+_dst+" -- mkpart extended "+QString::number(parts[p].start)+"s -1s\""))
            {
                ERROR(QString("mkpart failed with error code: ")+QString::number(result));
            }

        }
        else
        {
            if ((p == (n - 1)) && (_resize))
            {
                qDebug() << "Creating last partition" << p;
                display.append(&parts[p].ptype[0]+QString(" ")+&parts[p].ftype[0]);
                emit secondaryUpdate(display);
                if (QProcess::execute("sh -c \"parted -s "+_dst+" -- mkpart "+parts[p].ptype+" "+parts[p].ftype+" "+QString::number(parts[p].start)+"s -1s\""))
                {
                    ERROR("Failed to create last partition");
                }
            }
            else
            {
                qDebug() << "Creating partition" << p;
                display.append(&parts[p].ptype[0]+QString(" ")+&parts[p].ftype[0]);
                emit secondaryUpdate(display);
                if (QProcess::execute("sh -c \"parted -s "+_dst+" -- mkpart "+parts[p].ptype+" "+parts[p].ftype+" "+QString::number(parts[p].start)+"s "+QString::number(parts[p].end)+"s\""))
                {
                    ERROR("Failed to create partition");
                }
            }
        }
        CANCEL_CHECK;

        // refresh the kernel partion table
        system ("partprobe");
        CANCEL_CHECK;

        // create file systems
        if (!strncmp (parts[p].ftype, "fat", 3))
        {
            qDebug()<<"Format FAT";
            QString cmd = "sh -c \"mkfs.fat "+ partition_name(_dst) + QString::number(parts[p].pnum);
            if (strlen(parts[p].name) >0)
                    cmd += QString(" -n ") + QString(parts[p].name);
            cmd += "\"";
            if (QProcess::execute(cmd))
            {
                ERROR("Error Creating FAT partition");
            }
        }
        CANCEL_CHECK;

        if (!strcmp (parts[p].ftype, "ext4"))
        {
            qDebug() << "Format ext4";
            QString cmd = "sh -c \"mkfs.ext4 -F "+partition_name (_dst)+QString::number(parts[p].pnum);
            if (strlen(parts[p].name) >0)
                    cmd += QString(" -L ") + QString(parts[p].name);
            cmd += "\"";

            if (QProcess::execute(cmd))
            {
                ERROR("Error Creating EXT4 partition");
            }
        }
        CANCEL_CHECK;

        // set the flags
        if (!strcmp (parts[p].flags, "lba"))
        {
            qDebug() <<"lba";
            if (QProcess::execute ("sh -c \"parted -s " +_dst+" set "+QString::number(parts[p].pnum)+" lba on\""))
            {
                ERROR("Error creating lba flags");
            }
        }
        else
        {
            if (QProcess::execute ("sh -c \"parted -s " +_dst+" set "+QString::number(parts[p].pnum)+" lba off\""))
            {
                ERROR("Error creating lba flags");
            }
        }
        CANCEL_CHECK;

        prog = p + 1;

        //gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress), prog);
        emit setProgress(prog*512);
    }

    QProcess::execute("sh -c \"umount /mnt \"");
    system("partprobe");


    // do the copy for each partition
    for (p = 0; p < n; p++)
    {
        // don't try to copy extended partitions
        if (!strcmp (parts[p].ptype, "extended")) continue;

        sprintf (buffer, tr("Copying partition %d of %d...").toUtf8().constData(), p + 1, n);
        //gtk_label_set_text (GTK_LABEL (status), buffer);
        emit statusUpdate(buffer);
        emit secondaryUpdate("");
        //gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress), 0.0);
        emit setProgress(0);

        // mount partitions
        QString tmp;
        tmp="sh -c \"mount "+partition_name(_src)+QString::number(parts[p].pnum)+" "+QString(src_mnt)+"\"";
        qDebug() << tmp;
        if (QProcess::execute(tmp))
        {
            ERROR(QString("Cannot mount source ")+src_mnt);
        }
        CANCEL_CHECK;
        tmp="sh -c \"mount "+partition_name(_dst)+QString::number(parts[p].pnum)+" "+QString(dst_mnt)+"\"";
        qDebug() << tmp;
        if (QProcess::execute(tmp))
        {
            ERROR(QString("Cannot mount destination ")+dst_mnt);
        }
        CANCEL_CHECK;

        // check there is enough space...
        sprintf (buffer, "df %s | tail -n 1 | tr -s \" \" \" \" | cut -d ' ' -f 3", src_mnt);
        get_string (buffer, res);
        sscanf (res, "%ld", &srcsz);
        qDebug() << "Source size = " << srcsz;

        sprintf (buffer, "df %s | tail -n 1 | tr -s \" \" \" \" | cut -d ' ' -f 4",dst_mnt);
        get_string (buffer, res);
        sscanf (res, "%ld", &dstsz);
        qDebug() << "Dest size = " << dstsz;

        if (srcsz >= dstsz)
        {
            QProcess::execute("sh -c \"umount "+ QString(dst_mnt)+"\"");
            QProcess::execute("sh -c \"umount "+ QString(src_mnt)+"\"");
            ERROR("Insufficient space. Backup aborted.");
        }

        // start the copy itself in a new thread
        copyThread * myCopyThread = new copyThread();
        connect(myCopyThread, SIGNAL(resultReady(QString)), this, SLOT(handleCopyResults(QString)));
        connect(myCopyThread, SIGNAL(finished()), myCopyThread, SLOT(deleteLater()));
        connect(this, SIGNAL(doCopy(QString, QString)), myCopyThread, SLOT(doCopy(QString, QString)));

        emit doCopy(QString(src_mnt), QString(dst_mnt) );

        myCopyThread->start();
        //g_thread_new (NULL, copy_thread, NULL);

        // get the size to be copied
        sprintf (buffer, "df %s | tail -n 1 | tr -s \" \" \" \" | cut -d ' ' -f 3",src_mnt);
        get_string (buffer, res);
        sscanf (res, "%ld", &srcsz);
        if (srcsz < 50000) stime = 1;
        else if (srcsz < 500000) stime = 5;
        else stime = 10;
        emit setMaxProgress(512*srcsz);
        // wait for the copy to complete, while updating the progress bar...
        sprintf (buffer, "df %s | tail -n 1 | tr -s \" \" \" \" | cut -d ' ' -f 3",dst_mnt);
        //sprintf (buffer, "du -s %s", dst_mnt);
        while (copying)
        {
            get_string (buffer, res);
            sscanf (res, "%ld", &dstsz);
            dstsz=qMin(dstsz,srcsz);
            //gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress), prog);
            emit setProgress(512*dstsz);
            QString display = tr("Copying %1 MB of %2 MB").arg(QString::number(dstsz/1000),QString::number(srcsz/1000));
            qDebug() << display;
            emit secondaryUpdate(display);
            sleep (stime);
            CANCEL_CHECK;
        }
        myCopyThread->exit();

        //gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress), 1.0);
        emit setProgress(512);

        // unmount partitions
        QProcess::execute("sh -c \"umount "+ QString(dst_mnt)+"\"");
        CANCEL_CHECK;
        QProcess::execute("sh -c \"umount "+ QString(src_mnt)+"\"");
        CANCEL_CHECK;
    }
    //terminate_dialog (_("Copy complete."));
    QProcess::execute("sh -c \"mount -o ro /dev/mmcblk0p1 /mnt\"");
    QProcess::execute("rmdir "+QString(src_mnt));
    QProcess::execute("rmdir "+QString(dst_mnt));

    qDebug() << "Clonethread finished....";
    emit completed();
}

void piCloneThread::handleCopyResults(QString msg)
{
    qDebug()<<msg;
}
