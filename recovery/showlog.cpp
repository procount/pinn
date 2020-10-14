#include "showlog.h"
#include "ui_showlog.h"
#include "passwd.h"
#include "util.h"


#include <QDebug>
#include <QDialog>
#include <QProcess>
#include <QRegExp>
#include <QString>
#include <QVariantMap>



showlog::showlog(QListWidget * list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::showlog)
{
    ui->setupUi(this);

    show();
    QApplication::processEvents();
    on_refresh_clicked();
}

showlog::~showlog()
{
    delete ui;
}

void showlog::on_refresh_clicked()
{
    QByteArray log = getFileContents("/tmp/debug");
    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText(log);
}
