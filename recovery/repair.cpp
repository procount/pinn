#include "repair.h"
#include "ui_repair.h"
#include "passwd.h"
#include "fscheck.h"
#include "util.h"

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <QString>
#include <QVariantMap>

repair::repair(QListWidget * list, const QString &rootdrive, QWidget *parent) :
    QDialog(parent),
    _drive(rootdrive),
    ui(new Ui::repair)
{
    QListWidgetItem * item;

    _listinstalled=list;
    ui->setupUi(this);
    show();

    item = new QListWidgetItem("File System Check");
    item->setCheckState(Qt::Checked);
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
                    rerunPostInstallScript(_listinstalled);
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


int repair::rerunPostInstallScript(QListWidget * list)
{
    int os_err;
    QListWidgetItem * item;

    for(int row = 0; row < list->count(); row++)
    {
        item = list->item(row);
        if (item->checkState() == Qt::Checked)
        {
            os_err=0;
            QVariantMap image = item->data(Qt::UserRole).toMap();
            qDebug()<<image;
            QString os_name = image.value("name").toString();

            QString postInstallScript = image.value("folder").toString() + "/partition_setup.sh";
            if (QFile::exists(postInstallScript))
            {
                QProcess proc;
                QProcessEnvironment env;
                QStringList args(postInstallScript);
                env.insert("PATH", "/bin:/usr/bin:/sbin:/usr/sbin");

                QVariantList partitions = image.value("partitions").toList();
                int pnr = 1;
                foreach (QVariant pv, partitions)
                {
                    QString part = pv.toString();
                    QString nr    = QString::number(pnr);
                    QString uuid  = getUUID(part);
                    QString label = getLabel(part);
                    QString partuuid = getPartUUID(part);
                    QString id;
                    if (!label.isEmpty())
                        id = "LABEL="+label;
                    else
                        id = "UUID="+uuid;
                    if (_drive != "/dev/mmcblk0")
                        part = partuuid;

                    qDebug() << "part" << part << uuid << label;

                    args << "part"+nr+"="+part << "id"+nr+"="+id;

                    env.insert("part"+nr, part);
                    env.insert("id"+nr, id);
                    env.insert("partuuid"+nr, partuuid);
                    pnr++;
                }
                qDebug() << "Executing: sh" << args;
                qDebug() << "Env:" << env.toStringList();
                proc.setProcessChannelMode(proc.MergedChannels);
                proc.setProcessEnvironment(env);
                proc.setWorkingDirectory("/mnt2");
                proc.start("/bin/sh", args);
                proc.waitForFinished(-1);
                qDebug() << proc.exitStatus();

                if (proc.exitCode() != 0)
                {
                    emit error(tr("%1: Error executing partition setup script").arg(os_name)+"\n"+proc.readAll());
                    return false;
                }
            }
        }
    }
    return(false);
}
