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
#include "osgroup.h"
#include "countdownfilter.h"
#include "repair.h"
#include "splash.h"

#include <QMainWindow>
#include <QModelIndex>
//#include <QSplashScreen>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <QHostAddress>
#include <QProcess>

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
    explicit MainWindow(const QString &drive, const QString &defaultDisplay, KSplash *splash, bool noobsconfig, QWidget *parent = 0);
    ~MainWindow();

protected:
    Ui::MainWindow *ui;
    OsGroup * ug;
    QDialog *_qpd;
    QDialog *_qpssd;
    QList <int> _kc;
    QLabel *_info;
    int _infoDelay;
    int _kcpos;
    const QString _defaultDisplay;
    bool _silent, _allowSilent, _showAll, _fixate, _usbimages, _sdimages;
    static bool _partInited;
    static int _currentMode;
    KSplash *_splash;
    QSettings *_settings;
    bool _hasWifi;
    int _numInstalledOS, _numBootableOS, _devlistcount;
    QNetworkAccessManager *_netaccess;
    uint _neededMB, _availableMB, _availableDownloadMB;
    int _numMetaFilesToDownload, _numIconsToDownload, _numBuildsToDownload;
    int _numListsToDownload;
    QMessageBox *_displayModeBox;
    QTimer _networkStatusPollTimer, _piDrivePollTimer;
    QTime _time;
    QString _model, _repo, _drive, _bootdrive, _osdrive;
    int _noobsconfig;
    QHostAddress _ipaddress;
    QLabel *_menuLabel;
    QLabel *_checkLabel;
    QProcess * _pbackground;
    int _numFilesToCheck;
    bool _bdisplayUpdate;
    QSize _currentsize;
    CountdownFilter counter;
    int _networkTimeout;
    QList<QVariantMap> _newList;
    unsigned _availableImages;
    unsigned _selectImages;
    unsigned _waitforImages;
    unsigned _processedImages;
    QStringList _selectOsList;

    enum ModeTag {
        MODE_INSTALL=0,
        MODE_DOWNLOAD,
        MODE_REINSTALL,
        MODE_BACKUP
    } _eDownloadMode;

    QMap<QString,QVariantMap> listImages(const QString &folder = "/mnt/os");

    bool _bDownload;
    QString _local;
    uint _neededDownloadMB;
    int _listno;

    QList<QToolBar*> toolbars;
    int toolbar_index;

    QVariantMap _overrides;
    QProcess * _proc;

    void untarFirmware();
    void checkPinnFirmware();
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
    QListWidgetItem *findItemByName(const QString &name);
    QList<QListWidgetItem *> selectedItems();
    void updateNeeded();
    void updateActions();
    void downloadMetaFile(const QString &url, const QString &saveAs);
    void downloadIcon(const QString &urlstring, const QString &originalurl);
    void downloadList(const QString &urlstring);
    void downloadLists();
    void checkForUpdates(bool display = false);
    void downloadUpdate(const QString &urlstring, const QString &saveAs);
    void startImageWrite();
    void startImageReinstall();
    bool canInstallOs(const QString &name, const QVariantMap &values);
    bool isSupportedOs(const QString &name, const QVariantMap &values);
    void addImage(QVariantMap& m, QIcon &icon, bool &bInstalled);
    void updateInstalledStatus();
    void addInstalledImages();
    void addImagesFromUSB(const QString &device);
    void filterList();
    void copyWpa();
    void on_newVersion();
    QString menutext(int index);
    void fullFAT();
    void prepareMetaFiles();
    void startImageDownload();
    void startImageBackup();
    bool LooksLikePiDrive(QString devname);
    bool LooksLikeOSDrive(QString devname);
    void recalcAvailableMB();
    void checkFileSize(const QString &url, const QString &saveAs);
    void getDownloadSize(QVariantMap &new_details);

    void downloadRepoList(const QString &urlstring);
    void processRepoListJson(QVariant json);
    void loadOverrides(const QString &filename);
    void OverrideJson(QVariantMap& m);
    void createPinnEntry();

protected slots:

    void tick(int secs);
    void expired(void);

    void checkFileSizeRedirectCheck();
    void checkFileSizeComplete();
    void populate();
    void startBrowser();
    void startNetworking();
    void pollNetworkStatus();
    void onOnlineStateChanged(bool online);
    void downloadListComplete();
    void processJson(QVariant json);
    void processJsonOs(const QString &name, QVariantMap &details, QSet<QString> &iconurls);
    /* Events from ImageWriterThread */
    void onQpdError(const QString &msg);
    void onError(const QString &msg);
    void onCompleted(int arg=0);
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
    void on_actionReinstall_triggered();
    void on_actionCancel_triggered();
    void on_actionAdvanced_triggered();
    void on_actionBrowser_triggered();
    void on_actionWifi_triggered();
    void on_actionPassword_triggered();
    void on_actionEdit_config_triggered();
    void on_actionClone_triggered();
    void on_actionInfo_triggered();
    void on_actionInfoInstalled_triggered();
    void on_actionDownload_triggered();
    void on_actionWipe_triggered();
    void on_actionFschk_triggered();
    void on_actionRepair_triggered();
    void on_actionReplace_triggered();
    void on_actionBackup_triggered();
    void on_actionFirmware_triggered();
    void updateFirmware_button();

    void on_list_currentRowChanged();
    void on_list_doubleClicked(const QModelIndex &index);
    void on_list_itemChanged(QListWidgetItem *item);

    void on_targetCombo_currentIndexChanged(int index);
    void on_targetComboUsb_currentIndexChanged(int index);

    void on_actionClear_c_triggered();

    void on_actionTime_triggered();

signals:
    void networkUp();
    void newVersion();
};

#endif // MAINWINDOW_H
