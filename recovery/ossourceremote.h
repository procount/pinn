#ifndef OsSourceRemote_H
#define OsSourceRemote_H

#include "osinfo.h"
#include "ossource.h"

#include <QObject>
#include <QThread>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QDialog>
#include <QtNetwork/QNetworkAccessManager>
#include <QIcon>
#include <QString>

class OsSourceRemote : public OsSource
{
    Q_OBJECT
public:
    explicit OsSourceRemote(QObject *parent = 0);

signals:
//    void newSource(OsSourceRemote *src);
    void iconDownloaded(QString, QIcon );

protected:
    QDialog *_qpd;
    int _numIconsToDownload;
    QNetworkAccessManager *_netaccess;

    void downloadList(const QString &urlstring);
    void processJson(QVariant json);
    void processJsonOs(const QString &name, QVariantMap &new_details, QSet<QString> &iconurls);
    void downloadIcon(const QString &urlstring, const QString &originalurl);

protected slots:
    void downloadListRedirectCheck();
    void downloadListComplete();
    void downloadIconRedirectCheck();
    void downloadIconComplete();

public slots:
    virtual void monitorNetwork(QNetworkAccessManager *netaccess);
};



#endif // OsSourceRemote_H

