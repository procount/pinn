#include "updatethread.h"

#include <QString>
#include <QProcess>
#include <QThread>

updateThread::updateThread(QObject *parent) :
    QThread(parent)
{

}

void updateThread::doDownload(QString src, QString dst)
{
    _src=src;
    _dst=dst;
}

void updateThread::run()
{
    downloading = 1;
    QString result="rsync -ax "+QString(_src)+"/. "+QString(_dst)+"/.";
    QProcess::execute (result);
    downloading = 0;
    emit resultReady("DONE "+result);
}

