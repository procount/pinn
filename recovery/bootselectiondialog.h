#ifndef BOOTSELECTIONDIALOG_H
#define BOOTSELECTIONDIALOG_H

/*
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

#include "countdownfilter.h"

#include <QAbstractButton>
#include <QListWidgetItem>
#include <QModelIndex>
#include <QDialog>
#include <QVariantList>
#include <QVariantMap>
#include <QModelIndex>
#include <QTimer>
#include <QListWidgetItem>


namespace Ui {
class BootSelectionDialog;
}

class BootSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BootSelectionDialog(const QString &drive, const QString &defaultPartition, bool stickyboot, bool dsi=false, QWidget *parent = 0);
    ~BootSelectionDialog();
    virtual void accept();
    void setDisplayMode();

protected slots:
    void countdown(int);
    void bootPartition();
    void onKeyPress(int,int);
    void countdownExpired();
    void onJoyPress(int cec_code,int value);

private slots:
    void on_list_itemChanged(QListWidgetItem *item);
    void on_pushButton_clicked();

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_pb_Up_clicked();

    void on_pb_Down_clicked();

protected:
    QTimer _timer;
    int _countdown;
    bool _altered;
    QVariantList installed_os;
    void updateConfig4dsi(QByteArray partition);
    bool _dsi;
    //int extractPartition(QVariantMap m);

private:
    Ui::BootSelectionDialog *ui;
    bool _inSelection;
    QByteArray _drive;
    CountdownFilter _counter;
};

#endif // BOOTSELECTIONDIALOG_H
