#include "fscheck.h"
#include "ui_fscheck.h"
#include "passwd.h"

#include <QDebug>
#include <QProcess>
#include <QRegExp>
#include <QString>
#include <QVariantMap>

QString readexec(int log, const QString &cmd, int &errorcode)
{
    QProcess proc;
    QString output;
    //int errorcode=0;

    proc.start(cmd);
    proc.waitForFinished(-1);
    errorcode = proc.exitCode();
    proc.setProcessChannelMode(proc.MergedChannels);
    output = proc.readAll();

    if (log)
        qDebug() << cmd << "\n" << output << "\n";
    return (output);
}


fscheck::fscheck(QListWidget * list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fscheck)
{
    int dummy;
    int part_err=0;
    int os_err=0;
    int overall_err=0;
    QListWidgetItem * item;

    ui->setupUi(this);

    if (list->count() ==0)
    {
        ui->plainTextEdit->appendPlainText("No OSes to check");
        return;
    }

    ui->plainTextEdit->appendPlainText("Checking filesystems...");
    show();
    QApplication::processEvents();

    for(int row = 0; row < list->count(); row++)
    {
        item = list->item(row);
        if (item->checkState() == Qt::Checked)
        {
            os_err=0;
            QVariantMap entry = item->data(Qt::UserRole).toMap();
            QString name = entry.value("name").toString();
            ui->plainTextEdit->appendPlainText("Checking "+name);
            QVariantList partitions = entry.value("partitions").toList();
            foreach (QVariant pv, partitions)
            {
                QString part = pv.toString();
                ui->plainTextEdit->appendPlainText(part+":");

                QString sedparam;
                sedparam = "'s_^\\([^:]*\\):.* TYPE=\\(.*\\) .*_\\2_p'";

                QString cmd;
                cmd = "sh -c \"blkid | grep \"" + part+ ":\" | sed -ne "+sedparam+" \"";
                QString type = readexec(0,cmd, dummy);
                type.remove(QChar(34)).remove(QChar(10)).remove(QChar(13));
                if (type=="vfat")
                    type="fat";

                QApplication::processEvents();

                QString result;
                if (!type.isEmpty())
                {
                    cmd = "sh -c \"fsck."+type+" -y "+part+"\"";
                    result = readexec(1,cmd, part_err);
                    os_err |= part_err;
                    overall_err |= os_err;
                }
                ui->plainTextEdit->appendPlainText(type + result);

                QApplication::processEvents();

            }
            if (os_err)
                ui->plainTextEdit->appendPlainText(name + " filessystem: error detected\n");
            else
                ui->plainTextEdit->appendPlainText(name + " filesystem checked ok\n");
        }
    }
    if (overall_err)
        ui->plainTextEdit->appendPlainText("DONE - some errors detected");
    else
        ui->plainTextEdit->appendPlainText("DONE - SUCCESSFUL filesystem check");

}

fscheck::~fscheck()
{
    delete ui;
}
