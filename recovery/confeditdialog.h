#ifndef CONFEDITDIALOG_H
#define CONFEDITDIALOG_H

/* Configuration edit dialog
 *
 * Initial author: Floris Bos
 * Maintained by Raspberry Pi
 *
 * See LICENSE.txt for license details
 *
 */

#include <QDialog>
#include <QList>
#include <QVariantMap>

namespace Ui {
class ConfEditDialog;
}

class ConfEditDialogTab;

class ConfEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfEditDialog(const QVariantMap &m, const QString &partition, QWidget *parent = 0);
    ~ConfEditDialog();

public slots:
    virtual void accept();

private slots:
    void on_pbEdit_clicked();

private:
    Ui::ConfEditDialog *ui;
    QList<ConfEditDialogTab *> _tabs;
    QVariantMap _map;
    QString _partition;
};

#endif // CONFEDITDIALOG_H
