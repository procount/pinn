#ifndef COPYTHREAD_H
#define COPYTHREAD_H
#include <QString>
#include <QProcess>

#include <QThread>

extern int copying;

class copyThread : public QThread
{
    Q_OBJECT
public:
    explicit copyThread(QObject *parent = 0);

protected:
    virtual void run();

signals:
    void resultReady(const QString &s);

public slots:
    void doCopy(QString src, QString dst);

private:
    QString _src;
    QString _dst;
};

#endif // COPYTHREAD_H
