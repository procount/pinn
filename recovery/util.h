#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QByteArray>
#include <QVariant>

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

QByteArray getFileContents(const QString &filename);
void putFileContents(const QString &filename, const QByteArray &data);
QByteArray getRemoteFile(const QString &url);
bool backupFile(const QString &filename, const QString &ext="bak");
void getOverscan(int &top, int &bottom, int &left, int &right);
bool nameMatchesRiscOS(const QString &name);
uint readBoardRevision();
bool canBootOs(const QString& name, const QVariantMap& values);
void setRebootPartition(QByteArray partition);
QByteArray partdev(const QString &drivedev, int nr);
QByteArray sysclassblock(const QString &drivedev, int partnr = -1);

QByteArray getLabel(const QString part);
QByteArray getUUID(const QString part);
QByteArray getDiskId(const QString &device);
QByteArray getPartUUID(const QString &devpart);
QString getCsumType(const QVariantMap &partition);
QString getCsum(const QVariantMap &partition, const QString &csumType);

#endif // UTIL_H
