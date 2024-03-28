#ifndef INITDRIVETHREAD_H
#define INITDRIVETHREAD_H

/* Prepare storage for first use thread
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

#include <QThread>
#include <QMessageBox>
#include "config.h"

class InitDriveThread : public QThread
{
    Q_OBJECT
public:
    explicit InitDriveThread(const QString &drive, QObject *parent = 0, const QString& P1size="+0", const uint provision=0);
    bool formatUsbDrive();
    static bool saveBootFiles();
    static bool restoreBootFiles();

protected:
    virtual void run();

    bool method_resizePartitions();
    int sizeofBootFilesInKB();
    uint sizeofSDCardInBlocks();
    bool mountSystemPartition();
    bool umountSystemPartition();
    bool zeroMbr();
    bool formatSettingsPartition();
#ifdef RISCOS_BLOB_FILENAME
    bool writeRiscOSblob();
#endif
    bool method_reformatDrive();
    bool formatBootPartition();
    bool partitionDrive();
    bool setDiskId();

    QString _drive;
    uint _size;
    QString _action;
    uint _provision;

signals:
    void error(const QString &msg);
    void statusUpdate(const QString &msg);
    void completed();
    void query(const QString &msg, const QString &title, QMessageBox::StandardButton* answer);

public slots:

};

#endif // INITDRIVETHREAD_H
