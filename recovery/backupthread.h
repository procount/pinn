#ifndef BackupThread_H
#define BackupThread_H

#include <QThread>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>

class BackupThread : public QThread
{
    Q_OBJECT
public:
    explicit BackupThread(QObject *parent = 0, QString local="/mnt/");
    void addImage(const QVariantMap &entry);

protected:
    virtual void run();
    bool processImage(const QVariantMap &entry);
    bool isLabelAvailable(const QByteArray &label);
    void patchConfigTxt();
    QString _local;

    /* key: folder, value: flavour */
    QList<QVariantMap> _images;
    int _extraSpacePerPartition, _sectorOffset, _part;
    QVariantList installed_os;
    
signals:
    void error(const QString &msg);
    void statusUpdate(const QString &msg);
    void parsedImagesize(qint64 size);
    void completed();
    void runningMKFS();
    void finishedMKFS();
    
public slots:
    
};

#endif // BackupThread_H
