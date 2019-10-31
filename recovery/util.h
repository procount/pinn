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
 * Modified by @procount (c) 2019
 *
 * See LICENSE.txt for license details
 */

QByteArray getFileContents(const QString &filename);
void putFileContents(const QString &filename, const QByteArray &data);
QByteArray getRemoteFile(const QString &url);
bool backupFile(const QString &filename, const QString &ext="bak");
QString readexec(const QString &cmd, int &errorcode);
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

int xd(int ch);
void db(char * block, size_t len);
int hexdecode(const char * str, char * output, size_t * size);
extern int setkeyhex(const char * k);

#define MAXMSG  256
extern char out[MAXMSG];
extern char in[MAXMSG];
extern char key[MAXMSG];
extern size_t outsize;
extern size_t insize;
extern size_t progress;
extern size_t keysize;


#endif // UTIL_H
