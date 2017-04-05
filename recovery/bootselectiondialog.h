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

#include <QListWidgetItem>
#include <QModelIndex>
#include <QDialog>
#include <QVariantList>
#include <QVariantMap>
#include <QModelIndex>
#include <QTimer>

namespace Ui {
class BootSelectionDialog;
}

class BootSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BootSelectionDialog(const QString &drive, const QString &defaultPartition, bool stickyboot, QWidget *parent = 0);
    ~BootSelectionDialog();
    virtual void accept();
    void setDisplayMode();
    virtual bool eventFilter(QObject *obj, QEvent *event);

protected slots:
    void countdown();
    void bootPartition();

private slots:
    void on_list_activated(const QModelIndex &index);
    void on_list_itemChanged(QListWidgetItem *item);

protected:
    QTimer _timer;
    int _countdown;
    void stopCountdown();
    int extractPartition(QVariantMap m);

private:
    Ui::BootSelectionDialog *ui;
    bool _inSelection;
};

#endif // BOOTSELECTIONDIALOG_H
