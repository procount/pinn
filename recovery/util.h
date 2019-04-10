#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QListWidgetItem>
#include <sys/syscall.h>
#include <linux/reboot.h>

/*
 * Convenience functions
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 */

typedef int eNAMEPARTS;

#define eBASE 1
#define eFLAVOUR 2
#define eNICKNAME 4
#define eDATE 8
#define ePART 16

#define eNUMPARTS 5

//Do not return prefix
#define eSPLIT 32

#define eCORE (eBASE|eFLAVOUR)           //Normally used name
#define eBACKUP (eBASE|eFLAVOUR|eDATE)   //Normal name + backup date
#define eUNIQUE (eBASE|eFLAVOUR|ePART)   //Name name + partition info
#define eFULL (eBASE|eFLAVOUR|eNICKNAME|eDATE|ePART) //Everything

QByteArray getFileContents(const QString &filename);
void putFileContents(const QString &filename, const QByteArray &data);
QByteArray getRemoteFile(const QString &url);
QString readexec(int log, const QString &cmd, int &errorcode);
void getOverscan(int &top, int &bottom, int &left, int &right);
bool nameMatchesRiscOS(const QString &name);
bool nameMatchesWindows(const QString &name);
uint readBoardRevision();
bool canBootOs(const QString& name, const QVariantMap& values);
bool setRebootPartition(QByteArray partition);
void reboot();
QByteArray partdev(const QString &drivedev, int nr);
QByteArray sysclassblock(const QString &drivedev, int partnr = -1);
bool isURL(const QString &s);

QByteArray getLabel(const QString part);
QByteArray getUUID(const QString part);
QByteArray getDiskId(const QString &device);
QByteArray getPartUUID(const QString &devpart);
QByteArray getDevice(const QString & partuuid);

QString getDescription(const QString &folder, const QString &flavour);

QString getNameParts(const QString& input, eNAMEPARTS flags);
QString getNickNameParts(const QString& input, eNAMEPARTS flags);
QStringList splitNameParts(const QString& input);
void setNameParts(QStringList& list, eNAMEPARTS flags, const QString& part );
QString joinNameParts(QStringList input);
QString getCsumType(const QVariantMap &partition);
QString getCsum(const QVariantMap &partition, const QString &csumType);

bool updatePartitionScript(QVariantMap & entry, QListWidgetItem * witem);

#define NICKNAME(x) getNickNameParts(x, eCORE|eSPLIT);

int extractPartitionNumber(QByteArray& partition);

#define CORE(x) getNameParts(x, eCORE)


#endif // UTIL_H
