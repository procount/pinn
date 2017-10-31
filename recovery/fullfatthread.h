#ifndef FullFatThread_H
#define FullFatThread_H

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

class FullFatThread : public QThread
{
    Q_OBJECT
public:
    explicit FullFatThread(const QString &drive, QObject *parent = 0);
    bool formatUsbDrive();

protected:
    virtual void run();

    bool method_resizePartitions();
    bool mountSystemPartition();
    bool umountSystemPartition();

    QString _drive;

signals:
    void error(const QString &msg);
    void statusUpdate(const QString &msg);
    void completed();
    void query(const QString &msg, const QString &title, QMessageBox::StandardButton* answer);

public slots:

};

#endif // FullFatThread_H
