#include "rerunsetup.h"
#include "ui_rerunsetup.h"
#include "passwd.h"
#include "util.h"
#include "mainwindow.h"
#include <QDebug>
#include <QProcess>
#include <QRegExp>
#include <QString>
#include <QVariantMap>

rerunsetup::rerunsetup(QListWidget * listinstalled, MainWindow * mw, const QString &rootdrive, QWidget *parent) :
    QDialog(parent),
    _drive(rootdrive),
    ui(new Ui::rerunsetup)
{
    //int dummy;
    //int part_err=0;
    int os_err=0;
    int overall_err=0;
    QListWidgetItem * item;

    ui->setupUi(this);

    if (listinstalled->count() ==0)
    {
        ui->plainTextEdit->appendPlainText(tr("No OSes to check"));
        return;
    }

    ui->plainTextEdit->appendPlainText(tr("Checking for partition_setup.sh scripts..."));
    show();
    QApplication::processEvents();


    for(int row = 0; row < listinstalled->count(); row++)
    {
        item = listinstalled->item(row);
        if (item->checkState() == Qt::Checked)
        {
            os_err=0;
            QVariantMap image = item->data(Qt::UserRole).toMap();
            QString os_name = CORE(image.value("name").toString());
            if (os_name != "PINN")
            {
                QString folder = "/settings/os/"+os_name;

                QString postInstallScript = folder + "/partition_setup.sh";
                postInstallScript.replace(' ','_');
                if (!QFile::exists(postInstallScript))
                {
                    postInstallScript = image.value("folder").toString() + "/partition_setup.sh";
                    postInstallScript.replace(' ','_');
                }
                if (!QFile::exists(postInstallScript))
                {
                    ui->plainTextEdit->appendPlainText(tr("Cannot find setup script for ")+os_name);
                    ui->plainTextEdit->appendPlainText(tr("Maybe insert source USB stick"));
                    os_err =1;
                }
                if (image.value("supports_backup", "false").toBool() == false)
                {
                    ui->plainTextEdit->appendPlainText(os_name+tr("does not support re-running the setup script."));
                    os_err =1;
                }
                if (image.value("supports_backup", "false").toString() == "update")
                {
                    QListWidgetItem *witem = mw->findItemByName(os_name);

                    if (!updatePartitionScript(image, witem))
                    {
                        ui->plainTextEdit->appendPlainText(os_name+tr(" setup script needs to be updated."));
                        os_err =1;
                    }
                }

                if (QFile::exists(postInstallScript))
                {
                    QString csumType = getCsumType(image);
                    QString csum     = getCsum(image,csumType);

                    if ((csum != "") && (csumType != ""))
                    {
                        int errorcode;
                        QString filecsum = readexec(1,csumType+" "+postInstallScript, errorcode).split(" ").first();
                        if (filecsum != csum)
                        {
                            qDebug()<< "Expected partition_setup.sh csum= "<<csum<<" Calculated= "<<filecsum;
                            ui->plainTextEdit->appendPlainText(os_name+tr("Error in checksum for partition_setup.sh"));
                            ui->plainTextEdit->appendPlainText(os_name+QString(tr("Expected csum= %1 Calculated= %2")).arg(csum,filecsum));
                            os_err=1;
                        }
                    }
                }

                if (!os_err)
                {
                    QProcess proc;
                    QProcessEnvironment env;
                    QStringList args(postInstallScript);
                    env.insert("PATH", "/bin:/usr/bin:/sbin:/usr/sbin");

                    QVariantList partitions = image.value("partitions").toList();
                    int pnr = 1;
                    foreach (QVariant pv, partitions)
                    {
                        QString partuuid;
                        QString id;
                        QString nr = QString::number(pnr);
                        QString uuid;
                        QString label;
                        QString part = pv.toString();
                        if (part.left(8)=="PARTUUID")
                        { //for PARTUUID=xxxx-yy format
                            partuuid = part;
                            part = getDevice(partuuid);
                        }
                        else
                        { // for /dev/sdX format
                            partuuid = getPartUUID(part);
                        }
                        uuid  = getUUID(part);
                        label = getLabel(part);
                        if (!label.isEmpty())
                            id = "LABEL="+label;
                        else
                            id = "UUID="+uuid;
                        //if (_drive != "/dev/mmcblk0")
                        //    part = partuuid;

                        args << "part"+nr+"="+part << "id"+nr+"="+id;

                        env.insert("part"+nr, part);
                        env.insert("id"+nr, id);
                        env.insert("partuuid"+nr, partuuid);
                        pnr++;
                    }

                    env.insert("restore", "true");

                    ui->plainTextEdit->appendPlainText("Executing "+args.join(" "));
                    qDebug() << "Executing: sh" << args;
                    qDebug() << "Env:" << env.toStringList();
                    proc.setProcessChannelMode(proc.MergedChannels);
                    proc.setProcessEnvironment(env);
                    proc.setWorkingDirectory("/mnt2");
                    proc.start("/bin/sh", args);
                    proc.waitForFinished(-1);

                    QString output(proc.readAllStandardOutput());
                    qDebug() << output;

                    if (proc.exitCode() != 0)
                    {
                        ui->plainTextEdit->appendPlainText(tr("%1: Error executing partition setup script").arg(os_name)+"\n"+proc.readAll());
                        os_err =1;
                    }
                }
                if (os_err)
                    ui->plainTextEdit->appendPlainText(os_name + tr(" partions_setup.sh script error\n"));
                else
                    ui->plainTextEdit->appendPlainText(os_name + tr(" partition_setup.sh script OK\n"));
                overall_err |= os_err;
            }
        }
    }

    if (overall_err)
        ui->plainTextEdit->appendPlainText(tr("DONE - some setup script errors detected"));
    else
        ui->plainTextEdit->appendPlainText(tr("DONE - SUCCESSFUL setup scripts exeuted"));

}

rerunsetup::~rerunsetup()
{
    delete ui;
}

