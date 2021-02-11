#ifndef MULTIIMAGEWRITETHREAD_H
#define MULTIIMAGEWRITETHREAD_H
#include "config.h"
#include "progressslideshowdialog.h"

#include <QThread>
#include <QStringList>
#include <QMultiMap>
#include <QVariantList>
#include <QList>
#include <QMessageBox>

class OsInfo;
class PartitionInfo;

class MultiImageWriteThread : public QThread
{
    Q_OBJECT
public:
    explicit MultiImageWriteThread(const QString &bootdrive, const QString &rootdrive,bool noobsconfig=false, bool partition=true,  enum ModeTag mode=MODE_INSTALL, QObject *parent = 0);
    void addImage(const QString &folder, const QString &flavour);
    void addInstalledImage(const QString& folder, const QString& flavour, const QVariantMap& sParts, const QString& replacedName="");
    QMessageBox::ButtonRole untar(const QString &tarball, const QString &csumType, const QString &csum, bool bSuppressError=false);

protected:
    virtual void run();
    QString findTarballExt(QString base, QString exts);
    void correct_media_permissions();
    QMessageBox::ButtonRole processImage(OsInfo *image);
    void postInstallConfig(OsInfo *image, const QString &part, const QString &customName);
    void testForCustomFile(const QString &baseName, const QString &ext);
    void processEntry(const QString &srcfolder, const QString & entry);
    QStringList parseQuotedString(const QString &tarfile, int args);
    bool addPartitionEntry(int sizeInSectors, int type, int specialOffset = 0);
    QString shorten(QString example, int maxLabelLen);
    QByteArray makeLabelUnique(QByteArray label, int maxLabelLen, const QByteArray &device = "");
    bool mkfs(const QByteArray &device, const QByteArray &fstype = "ext4", const QByteArray &label = "", const QByteArray &mkfsopt = "");
    QMessageBox::ButtonRole dd(const QString &imagePath, const QString &csumType, const QString &csum, const QString &device);
    QMessageBox::ButtonRole partclone_restore(const QString &imagePath, const QString &csumType, const QString &csum, const QString &device);
    bool isLabelAvailable(const QByteArray &label, const QByteArray &device = "");
//    QByteArray getLabel(const QString part);
//    QByteArray getUUID(const QString part);
    void patchConfigTxt();
    bool writePartitionTable(const QString &drive, const QMap<int, PartitionInfo *> &partitionMap);
    //bool isURL(const QString &s);
//    QByteArray getDiskId(const QString &device);
//    QByteArray getPartUUID(const QString &devpart);
    int _checksumError;
    bool _setupError;

    /* key: folder, value: flavour */
    QList<OsInfo *> _images;
    QString _drive, _bootdrive;
    int _extraSpacePerPartition, _sectorOffset, _part;
    QVariantList installed_os;
    bool _multiDrives;
    bool _noobsconfig;
    bool _partition;
    enum ModeTag _downloadMode;
    QString _srcFolder; //For noobsconfig
    QString _dstFolder;
    QStringList stack;

signals:
    void error(const QString &msg);
    void errorContinue(const QString &msg);
    void checksumError( const QString &msg,const QString &title, QMessageBox::ButtonRole *answer);
    void statusUpdate(const QString &msg);
    void parsedImagesize(qint64 size);
    void completed(int arg);
    void newDrive(const QString&, eProgressMode);
    void startAccounting();
    void stopAccounting();
    void consolidate();
    void finish();
    void idle();
    void cont();

public slots:
};

#endif // MULTIIMAGEWRITETHREAD_H
