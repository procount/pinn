#include "json.h"
#include "replace.h"
#include "ui_replace.h"
#include "util.h"

#include <QDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>

replace::replace(QList<QListWidgetItem *> replacementList, QList<QListWidgetItem *> installedList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::replace)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(2);

    m_TableHeader << "Installed OS" << "New OS";
    ui->tableWidget->setHorizontalHeaderLabels(m_TableHeader);
    //ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    //ui->tableWidget->setShowGrid(false);
    //ui->tableWidget->setStyleSheet("QTableView {selection-background-color: red;}");
    //ui->tableWidget->setGeometry(QApplication::desktop()->screenGeometry());

    int numRows=0;
    {
        QListWidgetItem *item = installedList.at(0);
        if (!item)
            return; //check for NULL ptr
        QVariantMap installedMap = item->data(Qt::UserRole).toMap();
        if (installedMap.value("name").toString() =="PINN")
        {
            installedList.removeFirst();
        }
        numRows = installedList.count();
        ui->tableWidget->setRowCount(numRows);
    }

    for (int i=0; i<installedList.count(); i++)
    {
        QListWidgetItem *item;
        item = installedList.at(i);
        QVariantMap installedMap = item->data(Qt::UserRole).toMap();
        QString os_name = NICKNAME( installedMap.value("name").toString() );

        //set column 0 to installed OS
        QTableWidgetItem * iTableItem= new QTableWidgetItem(os_name);
        iTableItem->setData(Qt::UserRole, installedMap);
        ui->tableWidget->setItem(i, 0, iTableItem);

        //set column1 to replacement combos
        QComboBox* combo = new QComboBox();
        combo->addItem("<no change>");
        for (int j=0; j<replacementList.count(); j++)
        {
            QListWidgetItem *ritem = replacementList.at(j);
            QVariantMap rEntry = ritem->data(Qt::UserRole).toMap();
            combo->addItem(rEntry.value("name").toString(),rEntry);
        }
        ui->tableWidget->setCellWidget(i,1,combo);
    }
}

replace::~replace()
{
    delete ui;
}


void replace::on_buttonBox1_accepted()
{
    //qDebug() << "on_buttonBox_accepted";
}

QList<QVariantMap> replace::getMappedList()
{
    QList<QVariantMap> newMap;

    int numRows = ui->tableWidget->rowCount();
    for (int i=0; i<numRows; i++)
    {
        QTableWidgetItem * installedItem = ui->tableWidget->item(i,0);
        QComboBox * combo = (QComboBox *) ui->tableWidget->cellWidget(i,1);

        qDebug()<< installedItem->text() << " < " << combo->currentText();
        QVariantMap iMap = installedItem->data(Qt::UserRole).toMap();
        QVariantMap rMap = combo->itemData(combo->currentIndex(),Qt::UserRole).toMap();
        if (combo->currentText() != "<no change>")
        {
            rMap["existingOS"] = iMap;
            newMap.append(rMap);
        }
    }
    return(newMap);
}
