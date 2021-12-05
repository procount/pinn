#ifndef DLGINSTALL_H
#define DLGINSTALL_H

#include <QDialog>
#include "input.h"
#include "WidgetKeyboard.h"

namespace Ui {
class dlgInstall;
}

class dlgInstall : public QDialog
{
    Q_OBJECT

public:
    explicit dlgInstall(QWidget *parent = NULL);
    ~dlgInstall();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_cbvk_toggled(bool checked);
    void my_focusChanged(QWidget * old, QWidget* nw);

private:
    Ui::dlgInstall *ui;
    WidgetKeyboard *virtualKeyBoard;
    QWidget * _lastWidgetFocus;
    navigate _nav;
    navigate * pNav;

};

#endif // DLGINSTALL_H
