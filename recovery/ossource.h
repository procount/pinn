#ifndef OSSOURCE_H
#define OSSOURCE_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QMap>
#include <QVariant>
#include "osinfo.h"

class OSSource : public QObject
{
    Q_OBJECT
public:
    explicit OSSource(QObject *parent = 0);
    void setSource(const char * type);
    void setDevice(const char * id);
    void setLocation(const char * locn);
    void listImagesInDir();

protected:

signals:

public slots:
    void checkDeviceExists();
private:
    QString source;     // SDcard/USB/NETWORK etc.
    QString device;     // dev/sda1
    QString location;   // /media or http://...
    QMap<QString,QVariantMap> images;
    QMap<QString,OsInfo *> oses;
};



#endif // OSSource_H
