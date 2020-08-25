#include "json.h"
#include "mainwindow.h"
#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "util.h"
#include "input.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QVariantList>
#include <QVariantMap>

extern MainWindow * gMW;
extern QApplication * gApp;

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    ui->setupUi(this);
    ui->lv_select->setSortingEnabled(true);

    virtualKeyBoard = new WidgetKeyboard(this);

    //Add all the installable OSes
    QList<QListWidgetItem *> all = gMW->allItems();
    for (int i=0; i< all.count(); i++)
    {
        QVariantMap entry = all.value(i)->data(Qt::UserRole).toMap();
        QString name = entry.value("name").toString();

        if (ui->lv_select->findItems(name,Qt::MatchExactly).isEmpty())
        {
            QListWidgetItem * witem = new QListWidgetItem(name);
            witem->setCheckState(Qt::Unchecked);
            ui->lv_select->addItem(witem);
        }
    }

    //Add any additional installed OSes that we may not have the install files for at the moment
    QVariantList list = Json::loadFromFile("/settings/installed_os.json").toList();
    foreach (QVariant v, list)
    {
        QVariantMap m = v.toMap();
        QString name = m.value("name").toString();
        if (ui->lv_select->findItems(name,Qt::MatchExactly).isEmpty())
        {
            QListWidgetItem * witem = new QListWidgetItem(name);
            witem->setCheckState(Qt::Unchecked);
            ui->lv_select->addItem(witem);
        }
    }
    _lastWidgetFocus=NULL;
    connect(gApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT( my_focusChanged(QWidget*,QWidget*)));
    read();
}

OptionsDialog::~OptionsDialog()
{
    virtualKeyBoard->hide();
    Kinput::setWindow(_lastWindow);
    Kinput::setMenu(_lastMenu);
    Kinput::setGrabWindow(NULL);
    delete virtualKeyBoard;
    delete ui;
}

void OptionsDialog::read()
{
    int sep;
    QString line;
    QString file = getFileContents("/mnt/recovery.cmdline");

    sep = file.indexOf('\n');
    if (-1==sep)
    {
        line = file;
        remainderFile = "";
    }
    else
    {
        line = file.left(sep);
        remainderFile = file.mid(sep+1);
    }

    /* The select option is quite special! */
    QString selection;
    if (line.contains("select="))
    {
        QByteArray searchFor = "select=";
        int searchForLen = searchFor.length();
        int pos = line.indexOf(searchFor);
        int end;

        if (line.length() > pos+searchForLen && line.at(pos+searchForLen) == '"')
        {
            /* Value between quotes */
            searchForLen++;
            end = line.indexOf('"', pos+searchForLen);
        }
        else
        {
            end = line.indexOf(' ', pos+searchForLen);
        }
        if (end != -1)
            end = end-pos-searchForLen;;

        //Extract the list of select options
        selection  = line.mid(pos+searchForLen, end);
        qDebug() <<"Selection = " << selection;
        //Cut the selection from line
        if (end != -1)
            line = line.left(pos) + line.mid(pos+searchForLen+end+1);
        else
            line = line.left(pos);

        qDebug() <<"Line = " << line;

        QStringList args = selection.split(",", QString::SkipEmptyParts);
        foreach (QString arg, args)
        {
            if (!recognise(arg,""))
            {   //must be an OS
                QList<QListWidgetItem*> l= ui->lv_select->findItems(arg, Qt::MatchExactly);
                if (l.isEmpty())
                {   //Add it to list, even if we don't have the install files atm
                    QListWidgetItem * witem = new QListWidgetItem(arg);
                    witem->setCheckState(Qt::Checked);
                    ui->lv_select->addItem(witem);
                }
                else
                {
                    l.first()->setCheckState(Qt::Checked);
                }
            }
        }
    }

    /*======================================*/

    QStringList optionList = line.split(" ");

    foreach(QString option, optionList)
    {
        QString key;
        QString value;

        int sep = option.indexOf('=');
        if (-1 == sep)
        {
            key = option;
            value="";
        }
        else
        {
            key = option.left(sep);
            value = option.mid(sep+1);
        }
        if (!recognise(key,value))
        {
            if (value =="")
                remainderOptions += key+" ";
            else
                remainderOptions += key+"="+value+" ";
        }
    }
}

#define cb_entry(arg) \
    if (ui->cb_##arg->isChecked()) \
        contents += QString(#arg)+" ";

#define sel_entry(arg) \
    if (ui->cb_##arg->isChecked()) \
    {\
        if (!select.isEmpty())\
            select+=",";\
        select += QString(#arg);\
        qDebug() <<"sel adding :" << select;\
    }

#define eb_entry(arg)\
    if (ui->eb_##arg->text().trimmed() !="")\
        contents += QString(#arg)+"="+ui->eb_##arg->text()+" ";

#define dd_entry(arg)\
    if ( ( (value = ui->dd_##arg->currentText()) !="") && (value != "none"))\
        contents += QString(#arg)+"="+value+" ";\


void OptionsDialog::write()
{
    QString contents;
    QString value;
    QString select;
    //Write standard otions from dialog

    for (int i=0; i< ui->lv_select->count(); i++)
    {
        QListWidgetItem * item = ui->lv_select->item(i);
        if (Qt::Checked == item->checkState())
        {
            if (!select.isEmpty())\
                select+=",";\
            select += item->text();\
            qDebug() << "write selection: "<<select;
        }
    }

    cb_entry(runinstaller);
    cb_entry(noupdate);
    cb_entry(forceupdatepinn);
    cb_entry(nofirmware);
    cb_entry(noobsconfig);
    eb_entry(configpath);

    sel_entry(allinstalled);
    sel_entry(allsd);
    sel_entry(allusb);
    sel_entry(allnetwork);
    sel_entry(waitusb);
    sel_entry(waitsd);
    sel_entry(waitnetwork);
    sel_entry(waitall);

    cb_entry(silentinstall);
    cb_entry(silentreinstallnewer);

    if (!select.isEmpty())
        contents += "select=\""+select+"\" ";

    cb_entry(no_default_source);
    cb_entry(disableusbimages);
    cb_entry(disablesdimages);
    cb_entry(showall);
    eb_entry(alt_image_source);
    eb_entry(repo_list);
    eb_entry(repo);

    cb_entry(forcetrigger);
    cb_entry(gpiotriggerenable);
    cb_entry(keyboardtriggerdisable);
    eb_entry(remotetimeout);
    eb_entry(networktimeout);

    cb_entry(vncinstall);
    cb_entry(vncshare);
    cb_entry(ssh);
    cb_entry(rescueshell);
    cb_entry(disablesafemode);
    cb_entry(no_group);
    cb_entry(no_cursor);
    cb_entry(wallpaper_resize);
    cb_entry(dsi);
    eb_entry(bootmenutimeout);
    eb_entry(background);
    dd_entry(display);
    dd_entry(style);

    if (!ui->eb_reserve_space->text().trimmed().isEmpty())
    {
        value = (ui->cb_reserve_plus->checkState()==Qt::Checked) ? "+" : "";
        value += ui->eb_reserve_space->text().trimmed();
        contents += "reserve=" + value + " ";
    }

    contents += remainderOptions.trimmed() + "\n" + remainderFile;

    QFile f("/mnt/recovery.cmdline");
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(contents.toLatin1());
        f.close();
    }
}

void OptionsDialog::on_buttonBox_accepted()
{
    if (ui->cb_runinstaller->isChecked())
    {
        if (QMessageBox::warning(this,
                             tr("WARNING"),
                             tr("Warning: You have chosen the 'runinstaller' option.\nThis will erase all your OSes on next boot.\nAre you sure?"),
                             QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
            return;
    }
    write();
    QDialog::accept();
}


#undef cb_entry

#define cb_entry(arg) \
    if (key == #arg)\
    {\
        ui->cb_##arg->setChecked(true);\
        return(true);\
    }

#undef eb_entry

#define eb_entry(arg)\
    if (key == #arg)\
    {\
        ui->eb_##arg->setText(value);\
        return(true);\
    }

#undef dd_entry

#define dd_entry(arg)\
    if ( (index = ui->dd_##arg->findText(key)) != -1)\
    {\
        ui->dd_##arg->setCurrentIndex(index);\
        return(true);\
    }


bool OptionsDialog::recognise(const QString & key, const QString &value)
{
    int index;

    cb_entry(runinstaller);
    cb_entry(noupdate);
    cb_entry(forceupdatepinn);
    cb_entry(nofirmware);
    cb_entry(noobsconfig);
    eb_entry(configpath);

    cb_entry(allinstalled);
    cb_entry(allsd);
    cb_entry(allusb);
    cb_entry(allnetwork);
    cb_entry(waitusb);
    cb_entry(waitsd);
    cb_entry(waitnetwork);
    cb_entry(waitall);

    cb_entry(silentinstall);
    cb_entry(silentreinstallnewer);


    cb_entry(no_default_source);
    cb_entry(disableusbimages);
    cb_entry(disablesdimages);
    cb_entry(showall);
    eb_entry(alt_image_source);
    eb_entry(repo_list);
    eb_entry(repo);

    cb_entry(forcetrigger);
    cb_entry(gpiotriggerenable);
    cb_entry(keyboardtriggerdisable);
    eb_entry(remotetimeout);
    eb_entry(networktimeout);

    cb_entry(vncinstall);
    cb_entry(vncshare);
    cb_entry(ssh);
    cb_entry(rescueshell);
    cb_entry(disablesafemode);
    cb_entry(no_group);
    cb_entry(no_cursor);
    cb_entry(wallpaper_resize);
    cb_entry(dsi);
    eb_entry(bootmenutimeout);
    eb_entry(background);
    dd_entry(display);
    dd_entry(style);

    if (key == "reserve")
    {
        QString newvalue=value;
        if (value.left(1)=="+")
        {
            ui->cb_reserve_plus->setCheckState(Qt::Checked);
            newvalue = value.mid(1);
        }
        ui->eb_reserve_space->setText(newvalue);
        return (true);
    }

    if (key ==  "repo_list")
    {
        ui->eb_repo_list->setText(value);
        return(true);
    }

    return(false);
}

void OptionsDialog::on_buttonBox_rejected()
{
    QDialog::reject();
}

void OptionsDialog::on_cbvk_toggled(bool checked)
{
    if (checked)
    {
        if (_lastWidgetFocus)
            _lastWidgetFocus->setFocus();

        virtualKeyBoard->show();
        Kinput::setGrabWindow(virtualKeyBoard);
        _lastWindow = Kinput::getWindow();
        _lastMenu = Kinput::getMenu();
        Kinput::setWindow("VKeyboard");
        Kinput::setMenu("any");
    }
    else
    {
        virtualKeyBoard->hide();
        Kinput::setWindow(_lastWindow);
        Kinput::setMenu(_lastMenu);
        Kinput::setGrabWindow(NULL);
    }
}

void OptionsDialog::my_focusChanged(QWidget * old, QWidget* nw)
{
    if (nw == ui->cbvk)
        _lastWidgetFocus = old;
}
