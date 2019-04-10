#ifndef MultiImageDownloadThread_H
#define MultiImageDownloadThread_H

#include <QThread>
#include <QStringList>
#include <QMultiMap>
#include <QVariantList>

class MultiImageDownloadThread : public QThread
{
    Q_OBJECT
public:
    explicit MultiImageDownloadThread(QObject *parent = 0, QString local="/mnt/");
    void addImage(const QString &folder, const QString &flavour);
    void allowResume(bool allow);

protected:
    virtual void run();
    bool processImage(const QString &folder, const QString &flavour);
    bool isLabelAvailable(const QByteArray &label);
    QByteArray getLabel(const QString part);
    QByteArray getUUID(const QString part);
    void patchConfigTxt();
    QString _local;

    /* key: folder, value: flavour */
    QMultiMap<QString,QString> _images;
    int _extraSpacePerPartition, _sectorOffset, _part;
    QVariantList installed_os;
    bool _allowResume;

signals:
    void error(const QString &msg);
    void errorContinue(const QString &msg);
    void statusUpdate(const QString &msg);
    void parsedImagesize(qint64 size);
    void completed();
    void runningMKFS();
    void finishedMKFS();
    void imageWritten(QString Imagefile);

public slots:
    
};

#endif // MultiImageDownloadThread_H
