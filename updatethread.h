#ifndef UPDATETHREAD_H
#define UPDATETHREAD_H

#include <QObject>
#include <QThread>

extern int downloading;

class updateThread : public QThread
{
    Q_OBJECT
public:
    explicit updateThread(QObject *parent = 0);

protected:
    virtual void run();

signals:
    void resultReady(const QString &s);

public slots:
    void doDownload(QString src, QString dst);

private:
    QString _src;
    QString _dst;
};

#endif // UPDATETHREAD_H
