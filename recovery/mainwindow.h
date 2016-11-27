#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/* Main window
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

#include "drvmon.h"
#include "languagedialog.h"
#include "ossource.h"

#include <QMainWindow>
#include <QModelIndex>
#include <QSplashScreen>
#include <QMessageBox>
#include <QTimer>
#include <QTime>

namespace Ui {
class MainWindow;
}
class QProgressDialog;
class QSettings;
class QListWidgetItem;
class QNetworkAccessManager;
class QMessageBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &defaultDisplay, QSplashScreen *splash, bool noobsconfig, QWidget *parent = 0);
    ~MainWindow();
    QList<OsSource *> source;
    OsSource uiSource;

protected:
    Ui::MainWindow *ui;
    QDialog *_qpd;
    QList <int> _kc;
    int _kcpos;
    const QString _defaultDisplay;
    bool _silent, _allowSilent;
    static bool _partInited;
    static int _currentMode;
    QSplashScreen *_splash;
    QSettings *_settings;
    bool _hasWifi;
    bool _networkOK;
    QMessageBox *_displayModeBox;
    bool _hasUSB;
    int _numInstalledOS;
    QNetworkAccessManager *_netaccess;
    int _neededMB, _availableMB, _numMetaFilesToDownload, _numIconsToDownload, _numBuildsToDownload;
    QTimer _networkStatusPollTimer;
    QTime _time;
    QString _model;
    int _noobsconfig;
    drvmon drive_monitor;

    QMap<QString,QVariantMap> listImages();
    QMap<QString,QVariantMap> listImagesInDir(const QString &mountpoint, const QString &source);
    virtual void changeEvent(QEvent * event);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    void inputSequence();
    void repopulate();
    void displayMode(int modenr, bool silent = false);
    void update_window_title();
    bool requireNetwork();
    bool isOnline();
    QStringList getFlavours(const QString &folder);
    void rebuildInstalledList();
    QListWidgetItem *findItem(const QVariant &name);
    QList<QListWidgetItem *> selectedItems();
    void updateNeeded();
    void downloadMetaFile(const QString &url, const QString &saveAs);
    void downloadIcon(const QString &urlstring, const QString &originalurl);
    void downloadList(const QString &urlstring);
    void downloadLists();
    void checkForUpdates(void);
    void downloadUpdate(const QString &urlstring, const QString &saveAs);
    void startImageWrite();
    bool canInstallOs(const QString &name, const QVariantMap &values);
    bool isSupportedOs(const QString &name, const QVariantMap &values);
    void on_newVersion();

public slots:
    void onNewSource(OsSource *src);
    void onDrivesChanged(void);
    void onIconDownloaded(QString, QIcon );

protected slots:
    void populate();
    void startBrowser();
    void copyWpa();
    void startNetworking();
    void pollNetworkStatus();
    void onOnlineStateChanged(bool online);
    void downloadListComplete();
    void processJson(QVariant json);
    void processJsonOs(const QString &name, QVariantMap &details, QSet<QString> &iconurls);
    /* Events from ImageWriterThread */
    void onError(const QString &msg);
    void onCompleted();
    void onCloneError(const QString &msg);
    void onCloneCompleted();
    void downloadIconRedirectCheck();
    void downloadIconComplete();
    void downloadMetaRedirectCheck();
    void downloadMetaComplete();
    void downloadListRedirectCheck();
    void downloadUpdateRedirectCheck();
    void downloadUpdateComplete();
    void onQuery(const QString &msg, const QString &title, QMessageBox::StandardButton* answer);
    void hideDialogIfNoNetwork();
    void onKeyPress(int key);
    void closeEvent(QCloseEvent *event);

private slots:
    /* UI events */
    void on_actionWrite_image_to_disk_triggered();
    void on_actionCancel_triggered();
    void on_list_currentRowChanged();
    void on_actionAdvanced_triggered(bool checked);
    void on_actionEdit_config_triggered();
    void on_actionBrowser_triggered();
    void on_list_doubleClicked(const QModelIndex &index);
    void on_list_itemChanged(QListWidgetItem *item);
    void on_actionWifi_triggered();
    void on_actionAdvanced_triggered();
    void on_actionPassword_triggered();

    void on_actionClone_triggered();

signals:
    void networkUp();
    void newVersion();
};

#endif // MAINWINDOW_H
