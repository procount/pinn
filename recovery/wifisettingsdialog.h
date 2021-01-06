#ifndef WIFISETTINGSDIALOG_H
#define WIFISETTINGSDIALOG_H

/*
 * Wifi network selection dialog
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 * Modified by @procount (c) 2019
 * See LICENSE.txt for license details
 */

#include "WidgetKeyboard.h"
#include <QDialog>
#include <QDBusObjectPath>
#include <QVariantMap>
#include "input.h"
namespace Ui {
class WifiSettingsDialog;
}

class FiW1Wpa_supplicant1InterfaceInterface;
class AccessPoint;
class QListWidgetItem;
class QProgressDialog;

class WifiSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WifiSettingsDialog(const QString &preferredInterface = "wlan0", QWidget *parent = 0);
    ~WifiSettingsDialog();
    bool connectToWifi(const QString &ssid, const QString &username, const QString &password, bool useWPS = false);

protected:
    Ui::WifiSettingsDialog *ui;
    QString _ifname, _laststate;
    QDBusObjectPath _ifpath;
    FiW1Wpa_supplicant1InterfaceInterface *_interface;
    QDBusObjectPath _currentBSS;
    QProgressDialog *_qpd;
    bool _connecting;
    void msleep(int msec);
    QString removeQuotes(QString str);

private:
    WidgetKeyboard *virtualKeyBoard;
    navigate _nav;
    navigate * pNav;

protected slots:
    virtual void accept();
    void onBSSAdded(const QDBusObjectPath &path);
    void onPropertiesChanged(const QVariantMap &properties);
    void checkSettings();

private slots:
    void on_passwordRadio_toggled(bool checked);
    void on_list_currentItemChanged(QListWidgetItem *current);
    void on_vkeyboard_toggled(bool checked);
    void on_checkBox_stateChanged(int arg1);
};

#endif // WIFISETTINGSDIALOG_H
