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

#include "WidgetKeyboard.h"
#include "input.h"
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
    void on_cbvk_toggled(bool checked);
    void my_focusChanged(QWidget * old, QWidget* nw);
    void options_finished(int result);

private:
    Ui::ConfEditDialog *ui;
    QList<ConfEditDialogTab *> _tabs;
    QVariantMap _map;
    QString _partition;
    WidgetKeyboard *virtualKeyBoard;
    QWidget * _lastWidgetFocus;
    //navigate _nav;
    navigate * pNav;
};

#endif // CONFEDITDIALOG_H
