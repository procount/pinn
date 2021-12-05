#include "chkperms.h"
#include "ui_chkperms.h"
#include "passwd.h"
#include "util.h"

#include <QDebug>
#include <QProcess>
#include <QRegExp>
#include <QString>
#include <QVariantMap>
#include <QDir>
#include <QFile>
#include <QFileInfoList>

chkperms::chkperms(QListWidget * list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::chkperms)
{
    //int dummy;
    //int part_err=0;
    int os_err=0;
    int overall_err=0;
    QListWidgetItem * item;

    ui->setupUi(this);

    if (list->count() ==0)
    {
        ui->plainTextEdit->appendPlainText("No OSes to check");
        return;
    }

    ui->plainTextEdit->appendPlainText("Checking file permissions");
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

                QProcess::execute("mount -o rw "+part+" /mnt2");

                QDir dir("/mnt2/media");
                QFileInfoList dirList = dir.entryInfoList(QDir::AllDirs);
                foreach (QFileInfo dirinfo, dirList)
                {
                    QString hexperms;

                    hexperms = QString::number(dirinfo.permissions(),16);
                    ui->plainTextEdit->appendPlainText( dirinfo.filePath() );
                    ui->plainTextEdit->appendPlainText( hexperms );

                    QDir d=QDir(dirinfo.filePath());
                    ui->plainTextEdit->appendPlainText( QString::number(d.entryList().size()) );
                }

                QProcess::execute("umount /mnt2");

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

chkperms::~chkperms()
{
    delete ui;
}
