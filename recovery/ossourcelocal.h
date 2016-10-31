#ifndef OsSourceLocal_H
#define OsSourceLocal_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QMap>
#include <QVariant>
#include "osinfo.h"
#include "ossource.h"

class OsSourceLocal : public OsSource
{
    Q_OBJECT
public:
    explicit OsSourceLocal(QObject *parent = 0);

    virtual void readImages();

signals:
    void newSource(OsSourceLocal *src);

protected:


public slots:
    virtual void monitorDevice();
};



#endif // OsSourceLocal_H
