#include "dlginstall.h"
#include "ui_dlginstall.h"
#include <QDesktopWidget>

extern QApplication * gApp;

dlgInstall::dlgInstall(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgInstall)
{
    //setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    ui->setupUi(this);

    pNav=NULL;

    virtualKeyBoard = new WidgetKeyboard(this);

    _nav.setContext("deleteInstall", "any");

    _lastWidgetFocus=NULL;
    connect(gApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT( my_focusChanged(QWidget*,QWidget*)));

}

dlgInstall::~dlgInstall()
{
    virtualKeyBoard->hide();
    if (pNav)
    {
        delete pNav;
        pNav=NULL;
    }
    Kinput::setGrabWindow(NULL);
    delete virtualKeyBoard;

    delete ui;
}

void dlgInstall::on_buttonBox_accepted()
{
    if (ui->deleteWord->text().trimmed().toUpper() == "DELETE")
        QDialog::accept();
    else {
        QDialog::reject();
    }
}

void dlgInstall::on_buttonBox_rejected()
{
    QDialog::reject();
}

void dlgInstall::on_cbvk_toggled(bool checked)
{
    if (checked)
    {
        if (_lastWidgetFocus)
            _lastWidgetFocus->setFocus();

        virtualKeyBoard->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignHCenter | Qt::AlignBottom, QSize(480,160), gApp->desktop()->availableGeometry()));
        virtualKeyBoard->show();
        if (pNav)
            delete pNav;
        pNav = new navigate("VKeyboard", "any", virtualKeyBoard);
    }
    else
    {
        virtualKeyBoard->hide();
        if (pNav)
        {
            delete pNav;
            pNav=NULL;
        }
    }

}

void dlgInstall::my_focusChanged(QWidget * old, QWidget* nw)
{
    if (nw == ui->cbvk)
        _lastWidgetFocus = old;
}
