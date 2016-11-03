#include "piclonedialog.h"
#include "ui_piclonedialog.h"

#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QDialog>
#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QMessageBox>

piclonedialog::piclonedialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::piclonedialog)
{
    ui->setupUi(this);
    src_count=0;
    dst_count=0;
    on_drives_changed();
}

piclonedialog::~piclonedialog()
{
    delete ui;
}

/* Parse the partition table to get a device name */
int piclonedialog::get_dev_name (const char *dev, char *name)
{
    char buffer[256];
    FILE *fp;

    sprintf (buffer, "parted -l | grep -B 1 \"%s\" | head -n 1 | cut -d \":\" -f 2 | cut -d \"(\" -f 1", dev);
    fp = popen (buffer, "r");
    if (fp == NULL || fgets (buffer, sizeof (buffer) - 1, fp) == NULL) return 0;
    buffer[strlen (buffer) - 2] = 0;
    strcpy (name, buffer + 1);
    pclose(fp);
    return 1;
}


void piclonedialog::on_drives_changed(void)
{
    char buffer[256], name[128], device[32];
	FILE *fp;
    QStringList drives;

	// empty the comboboxes
    ui->from_cb->clear();
    src_count=0;        
    
    ui->to_cb->clear();
    dst_count=0;        

	// populate the comboboxes
    ui->from_cb->insertItem(0,"Internal SD card  (/dev/mmcblk0)");
	src_count++;

    fp = popen ("lsblk -pl| grep 'disk' | cut -d ' ' -f 1", "r");
    if (fp != NULL)
    {
	    while (1)
	    {
            if (fgets (device, sizeof (device) - 1, fp) == NULL)
                break;

	        if (!strncmp (device + 5, "sd", 2) || !strncmp (device + 5, "mmcblk1", 7) )
	        {
	            device[strlen (device) - 1] = 0;
                drives << device;
                qDebug() <<device;
	        }
	    }
        pclose(fp);
	}

    foreach (QString drive, drives)
    {
        get_dev_name (drive.toUtf8().constData(), name);
        sprintf (buffer, "%s  (%s)", name, drive.toUtf8().constData());
        qDebug() <<buffer;
        ui->from_cb->insertItem(src_count++, buffer);
        ui->to_cb->insertItem(dst_count++, buffer);
    }

    if (dst_count == 0)
	{
        ui->to_cb->insertItem(dst_count, "No devices available");
        ui->to_cb->setEnabled(false);
	}
	else 
    {
        ui->to_cb->setEnabled(true);
    }   

    on_cb_Changed("");
}


void piclonedialog::on_cb_Changed(const QString &arg1)
{
    Q_UNUSED(arg1)
    if ( (dst_count==0) ||
         (ui->from_cb->currentText() == "") ||
         (ui->to_cb->currentText() =="") ||
         (ui->from_cb->currentText() == ui->to_cb->currentText()))
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( false );
    }
    else
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( true );
    }
}

void piclonedialog::on_from_cb_currentIndexChanged(const QString &arg1)
{
    on_cb_Changed(arg1);
}

void piclonedialog::on_to_cb_currentIndexChanged(const QString &arg1)
{
    on_cb_Changed(arg1);
}

void piclonedialog::on_buttonBox_accepted()
{
    _src = ui->from_cb->currentText();
    int start = _src.lastIndexOf('(');
    int end   = _src.lastIndexOf(')');
    src_dev = _src.mid(start+1,end-start-1);
    qDebug() << "Src: " << src_dev;

    _dst = ui->to_cb->currentText();
    start = _dst.lastIndexOf('(');
    end   = _dst.lastIndexOf(')');
    dst_dev = _dst.mid(start+1,end-start-1);
    qDebug() << "Dst: " << dst_dev;
}
