#ifndef PICLONETHREAD_H
#define PICLONETHREAD_H

#include <QThread>
#include <QString>

class piCloneThread : public QThread
{
    Q_OBJECT
public:

    explicit piCloneThread(QString src_dev, QString dst_dev, QObject *parent = 0);
    virtual void run();

protected slots:

    void handleCopyResults(QString msg);

signals:
    void error(const QString &msg);
    void statusUpdate(const QString &msg);
    void setMaxProgress(qint64 size);
    void setProgress(qint64 size);
    void secondaryUpdate(const QString &msg);
    void doCopy(QString src_mnt, QString dst_mnt );
    void completed();

private:
    QString _src;
    QString _dst;
};




#endif // PICLONETHREAD_H
