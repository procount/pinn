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

#include "languagedialog.h"
#include <QMainWindow>
#include <QModelIndex>
#include <QSplashScreen>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <QHostAddress>

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
    explicit MainWindow(const QString &drive, const QString &defaultDisplay, QSplashScreen *splash, bool noobsconfig, QWidget *parent = 0);
    ~MainWindow();

protected:
    Ui::MainWindow *ui;
    QDialog *_qpd;
    QList <int> _kc;
    int _kcpos;
    const QString _defaultDisplay;
    bool _silent, _allowSilent, _showAll, _fixate, _usbimages;
    static bool _partInited;
    static int _currentMode;
    QSplashScreen *_splash;
    QSettings *_settings;
    bool _hasWifi;
    int _numInstalledOS, _devlistcount;
    QNetworkAccessManager *_netaccess;
    uint _neededMB, _availableMB;
    int _numMetaFilesToDownload, _numIconsToDownload, _numBuildsToDownload;
    QMessageBox *_displayModeBox;
    QTimer _networkStatusPollTimer, _piDrivePollTimer;
    QTime _time;
    QString _model, _repo, _drive, _bootdrive;
    int _noobsconfig;
    QHostAddress _ipaddress;

    QMap<QString,QVariantMap> listImages(const QString &folder = "/mnt/os", bool includeInstalled = true);
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
    void addImagesFromUSB(const QString &device);
    void filterList();
    void copyWpa();
    void on_newVersion();

    void downloadRepoList(const QString &urlstring);
    void processRepoListJson(QVariant json);

protected slots:
    void populate();
    void startBrowser();
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
    void downloadRepoListComplete();
    void downloadRepoListRedirectCheck();
    void onQuery(const QString &msg, const QString &title, QMessageBox::StandardButton* answer);
    void hideDialogIfNoNetwork();
    void pollForNewDisks();
    void onKeyPress(int key);
    void closeEvent(QCloseEvent *event);

private slots:
    /* UI events */
    void on_actionWrite_image_to_disk_triggered();
    void on_actionCancel_triggered();
    void on_list_currentRowChanged();
    void on_actionAdvanced_triggered();
    void on_actionEdit_config_triggered();
    void on_actionBrowser_triggered();
    void on_list_doubleClicked(const QModelIndex &index);
    void on_list_itemChanged(QListWidgetItem *item);
    void on_actionWifi_triggered();
    void on_actionPassword_triggered();
    void on_targetCombo_currentIndexChanged(int index);
    void on_actionClone_triggered();
    void on_actionInfo_triggered();

signals:
    void networkUp();
    void newVersion();
};

#endif // MAINWINDOW_H
