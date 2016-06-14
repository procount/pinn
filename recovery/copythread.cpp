#include "copythread.h"
#include <QString>
#include <QProcess>
#include <QThread>
int copying;

copyThread::copyThread(QObject *parent) :
    QThread(parent)
{
}

void copyThread::doCopy(QString src, QString dst)
{
    _src=src;
    _dst=dst;
}

void copyThread::run()
{
    copying = 1;
    QString result="rsync -ax "+QString(_src)+"/. "+QString(_dst)+"/.";
    QProcess::execute (result);
    copying = 0;
    emit resultReady("DONE "+result);
}

