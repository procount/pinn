#include "ossourceremote.h"
#include "config.h"
#include "json.h"
#include "osinfo.h"

#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QPixmap>
#include <QMessageBox>
#include <QIcon>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <sys/time.h>
#include <QDateTime>


OsSourceRemote::OsSourceRemote(QObject *parent) :
    OsSource(parent)
{
}

void OsSourceRemote::monitorNetwork(QNetworkAccessManager *netaccess)
{
    _netaccess = netaccess;
    qDebug() << "OsSourceRemote::monitorNetwork "<< location;
    downloadList(location);
}

void OsSourceRemote::downloadList(const QString &urlstring)
{
    QUrl url(urlstring);
    QNetworkRequest request(location);
    request.setRawHeader("User-Agent", AGENT);
    QNetworkReply *reply = _netaccess->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadListRedirectCheck()));
}


void OsSourceRemote::downloadListRedirectCheck()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString redirectionurl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    QByteArray dateStr = reply->rawHeader("Date");
    if (!dateStr.isEmpty() && QDate::currentDate().year() < 2015)
    {
        // Qt 4 does not have a standard function for parsing the Date header, but it does
        // have one for parsing a Last-Modified header that uses the same date/time format, so just use that
        QNetworkRequest dummyReq;
        dummyReq.setRawHeader("Last-Modified", dateStr);
        QDateTime parsedDate = dummyReq.header(dummyReq.LastModifiedHeader).toDateTime();
        struct timeval tv;
        tv.tv_sec = parsedDate.toTime_t();
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);

        qDebug() << "Time set to " << parsedDate;
    }

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        qDebug() << "Redirection - Re-trying download from" << redirectionurl;
        downloadList(redirectionurl);
    }
    else
        downloadListComplete();
}

void OsSourceRemote::downloadListComplete()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != reply->NoError || httpstatuscode < 200 || httpstatuscode > 399)
    {
        if (_qpd)
            _qpd->hide();
        //@@ _networkOK = false;

        qDebug() << "Error Downloading "<< reply->url()<<" reply: "<< reply->error() << " httpstatus: "<< httpstatuscode;
        QMessageBox::critical((QWidget*)parent(), tr("Download error"), tr("Error downloading distribution list from Internet"), QMessageBox::Close);
    }
    else
    {
        processJson(Json::parse( reply->readAll() ));
    }

    reply->deleteLater();
}

void OsSourceRemote::processJson(QVariant json)
{
    if (json.isNull())
    {
        QMessageBox::critical((QWidget*)parent(), tr("Error"), tr("Error parsing list.json downloaded from server"), QMessageBox::Close);
        return;
    }

    QSet<QString> iconurls;
    QVariantList list = json.toMap().value("os_list").toList();

    foreach (QVariant osv, list)
    {   //For each OS listed int JSON file
        QVariantMap  os = osv.toMap();

        QString basename = os.value("os_name").toString();
        //if (canInstallOs(basename, os))
        {
            if (os.contains("flavours"))
            {
                QVariantList flavours = os.value("flavours").toList();

                foreach (QVariant flv, flavours)
                {
                    QVariantMap flavour = flv.toMap();
                    QVariantMap item = os;
                    QString name        = flavour.value("name").toString();
                    QString description = flavour.value("description").toString();
                    QString iconurl     = flavour.value("icon").toString();

                    item.insert("name", name);
                    item.insert("description", description);
                    item.insert("icon", iconurl);
                    item.insert("feature_level", flavour.value("feature_level"));
                    item.insert("source", SOURCE_NETWORK);

                    processJsonOs(name, item, iconurls);
                }
            }
            if (os.contains("description"))
            {
                QString name = basename;
                os["name"] = name;
                os["source"] = SOURCE_NETWORK;
                processJsonOs(name, os, iconurls);
            }
        }
    }

    /* Download icons */
    if (!iconurls.isEmpty())
    {
         _numIconsToDownload += iconurls.count();
        foreach (QString iconurl, iconurls)
        {
            downloadIcon(iconurl, iconurl);
        }
    }
    else
    {
        if (_qpd)
        {
            _qpd->deleteLater();
            _qpd = NULL;
        }
    }
}

void OsSourceRemote::processJsonOs(const QString &name, QVariantMap &new_details, QSet<QString> &iconurls)
{
    QIcon internetIcon(":/icons/download.png");

    OsInfo * newOs = new OsInfo();
    newOs->importMap(new_details);
    //@@ Partitions? iconurls?
    oses[name] = newOs;
}

void OsSourceRemote::downloadIcon(const QString &urlstring, const QString &originalurl)
{
    QUrl url(urlstring);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, originalurl);
    request.setRawHeader("User-Agent", AGENT);
    QNetworkReply *reply = _netaccess->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadIconRedirectCheck()));
}

void OsSourceRemote::downloadIconRedirectCheck()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString redirectionurl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QString originalurl = reply->request().attribute(QNetworkRequest::User).toString();;

    if (httpstatuscode > 300 && httpstatuscode < 400)
    {
        qDebug() << "Redirection - Re-trying download from" << redirectionurl;
        downloadIcon(redirectionurl, originalurl);
    }
    else
        downloadIconComplete();
}

void OsSourceRemote::downloadIconComplete()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QString url = reply->url().toString();
    QString originalurl = reply->request().attribute(QNetworkRequest::User).toString();
    int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != reply->NoError || httpstatuscode < 200 || httpstatuscode > 399)
    {
        //QMessageBox::critical(parent(), tr("Download error"), tr("Error downloading icon '%1'").arg(reply->url().toString()), QMessageBox::Close);
        qDebug() << "Error downloading icon" << url;
    }
    else
    {
        QPixmap pix;
        pix.loadFromData(reply->readAll());
        QIcon icon(pix);

//@@        for (int i=0; i<ui->list->count(); i++)
//        {
//            QVariantMap m = ui->list->item(i)->data(Qt::UserRole).toMap();
//            ui->list->setIconSize(QSize(40,40));
//            if (m.value("icon") == originalurl)
//            {
//                ui->list->item(i)->setIcon(icon);
//            }
//        }
    }
    if (--_numIconsToDownload == 0 && _qpd)
    {
        _qpd->hide();
        _qpd->deleteLater();
        _qpd = NULL;
    }

    reply->deleteLater();
}
