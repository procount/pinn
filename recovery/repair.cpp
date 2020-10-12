#include "repair.h"
#include "ui_repair.h"
#include "passwd.h"
#include "fscheck.h"
#include "util.h"
#include "rerunsetup.h"
#include "showlog.h"
#include "mainwindow.h"

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <QString>
#include <QVariantMap>

repair::repair(QListWidget * listinstalled, MainWindow * mw, const QString &rootdrive, QWidget *parent) :
    QDialog(parent),
    _drive(rootdrive),
    ui(new Ui::repair)
{
    QListWidgetItem * item;

    _listinstalled = listinstalled;
    _mw = mw;
    ui->setupUi(this);
    show();

    item = new QListWidgetItem("File System Check");
    item->setCheckState(Qt::Unchecked);
    QVariantMap mfs;
    mfs["action"]=QString("fscheck");
    item->setData(Qt::UserRole, mfs);
    ui->listWidget->addItem(item);

    item = new QListWidgetItem("Re-run partition_setup.sh");
    item->setCheckState(Qt::Unchecked);
    QVariantMap mps;
    mps["action"]=QString("setup");
    item->setData(Qt::UserRole, mps);
    ui->listWidget->addItem(item);

    item = new QListWidgetItem("Show Debug log");
    item->setCheckState(Qt::Unchecked);
    QVariantMap mfl;
    mfl["action"]=QString("showlog");
    item->setData(Qt::UserRole, mfl);
    ui->listWidget->addItem(item);

#if 0
    item = new QListWidgetItem("Upgrade firmware");
    item->setCheckState(Qt::Unchecked);
    QVariantMap mfw;
    mfw["action"]=QString("upgrade");
    item->setData(Qt::UserRole, mfw);
    ui->listWidget->addItem(item);
#endif
}

repair::~repair()
{
    delete ui;
}

void repair::on_buttonBox_accepted()
{
    QListWidgetItem *item = _listinstalled->currentItem();

    if (_listinstalled->count() && item)
    {   //Some OSes are selected

        for(int row = 0; row < ui->listWidget->count(); row++)
        {
            QListWidgetItem * actionitem = ui->listWidget->item(row);
            if (actionitem->checkState() == Qt::Checked)
            {
                QVariantMap m = actionitem->data(Qt::UserRole).toMap();

                if (m["action"] == "fscheck")
                {
                    fscheck dlg(_listinstalled);
                    dlg.exec();
                }
                if (m["action"] == "setup")
                {
                   rerunsetup dlg(_listinstalled,_mw,_drive);
                   dlg.exec();
                }
                if (m["action"] == "showlog")
                {
                    showlog dlg(_listinstalled);
                    dlg.exec();
                }

                if (m["action"] == "upgrade")
                {
                    /*xxx dlg(_listinstalled);
                    dlg.exec();*/
                }
            }
        }
    }
}


