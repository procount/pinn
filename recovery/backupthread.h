#ifndef BackupThread_H
#define BackupThread_H

#include <QThread>
#include <QStringList>
#include <QMultiMap>
#include <QVariantList>

class BackupThread : public QThread
{
    Q_OBJECT
public:
    explicit BackupThread(QObject *parent = 0, QString local="/mnt/");
    void addImage(const QString &folder, const QString &flavour);

protected:
    virtual void run();
    bool processImage(const QString &folder, const QString &flavour);
    bool isLabelAvailable(const QByteArray &label);
    QByteArray getLabel(const QString part);
    QByteArray getUUID(const QString part);
    void patchConfigTxt();
    QString getDescription(const QString &folder, const QString &flavour);
    QString _local;

    /* key: folder, value: flavour */
    QMultiMap<QString,QString> _images;
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
