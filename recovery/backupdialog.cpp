#include "json.h"
#include "backupdialog.h"
#include "util.h"
#include "ui_backupdialog.h"

#include <QDebug>

backupdialog::backupdialog(QVariantMap &Map, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::backupdialog), _map(Map)
{
    ui->setupUi(this);

    QString description;
    QString name;
    QString datetime;

    QString fullname;
    if (_map.contains("backupName"))
        fullname = _map.value("backupName").toString();
    else if (_map.contains("name"))
        fullname = _map.value("name").toString();

    name = getNameParts(fullname, eCORE);
    datetime = getNameParts(fullname, eDATE|eSPLIT);

    if (_map.contains("description"))
        description = _map.value("description").toString();

    ui->nameEdit->setText(name);
    ui->datetimeEdit->setText(datetime);
    ui->descriptionEdit->setText(description);
}

backupdialog::~backupdialog()
{
    delete ui;
}


void backupdialog::on_buttonBox_accepted()
{
    QString core = ui->nameEdit->text();
    QString datetime = ui->datetimeEdit->text();
    QString description = ui->descriptionEdit->text();

    datetime.remove(QChar('#'));
    datetime.remove(QChar('='));
    datetime.remove(QChar('@'));

    //Set backupName to datetime
    QString name = _map.value("backupName").toString();
    QStringList parts = splitNameParts(name);
    setNameParts(parts, eDATE, datetime);
    name = joinNameParts(parts);

    //Update map with new name & Description
    _map["backupName"] = name.replace(" ","_");
    _map["description"] = description;
}
