#include "json.h"
#include "adjustsizes.h"
#include "ui_adjustsizes.h"
#include "util.h"

#define LOCAL_DBG_ON   1
#define LOCAL_DBG_FUNC 1
#define LOCAL_DBG_OUT  1
#define LOCAL_DBG_MSG  0

#include "mydebug.h"

#include <QApplication>
#include <QDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>

adjustSizes::adjustSizes(uint provision, const QString &rootdrive, QList<OsInfo *> _images, QWidget *parent) :
    _provision(provision),  _drive(rootdrive), QDialog(parent),
    ui(new Ui::adjustSizes)
{
    TRACE

    _initialised=false;
    ui->setupUi(this);

    uint totalnominalsize = 0, totaluncompressedsize = 0;
    uint startSector = getFileContents(sysclassblock(_drive, 5)+"/start").trimmed().toUInt()
                    + getFileContents(sysclassblock(_drive, 5)+"/size").trimmed().toUInt();
    uint totalSectors = getFileContents(sysclassblock(_drive)+"/size").trimmed().toUInt();
    _availableMB = (totalSectors-startSector-_provision)/2048;
    uint nominalImageSize;
    _numparts = 0, _numexpandparts = 0;



    ui->tableWidget->setColumnCount(4);

    //ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    //ui->tableWidget->setShowGrid(false);
    //ui->tableWidget->setStyleSheet("QTableView {selection-background-color: red;}");
    //ui->tableWidget->setGeometry(QApplication::desktop()->screenGeometry());

    ui->tableWidget->setRowCount(_images.count());
    int i=0;
    foreach (OsInfo *image, _images)
    {
        SpaceInfo * os_space = new(SpaceInfo);

        QList<PartitionInfo *> *partitions = image->partitions();
        if (partitions->isEmpty())
        {
            //emit error(tr("partitions.json invalid"));
            return;
        }

        nominalImageSize=0;
        os_space->numexpandparts=0;
        foreach (PartitionInfo *partition, *partitions)
        {
            _numparts++;
            if ( partition->wantMaximised() )
                os_space->numexpandparts++;
            uint nominalsize = partition->partitionSizeNominal();

            nominalImageSize += nominalsize;

            totalnominalsize += nominalsize;
            totaluncompressedsize += partition->uncompressedTarballSize();

            if ( (partition->fsType() == "ext4") || (partition->fsType() == "ext3") )
            {
                totaluncompressedsize += nominalsize / 20 ; /* overhead for file system meta data */
            }
        }
        _numexpandparts += os_space->numexpandparts;

        QSize size(10,10);

        os_space->name = image->name();

        //set column 0 to OSname
        QTableWidgetItem * iTableItem= new QTableWidgetItem();
        iTableItem->setFlags(iTableItem->flags() & ~(Qt::ItemIsEditable|Qt::ItemIsSelectable));
        ui->tableWidget->setItem(i, 0, iTableItem);

        os_space->nominal_mb = nominalImageSize;

        //set column 1 to nominal size
        QTableWidgetItem * iTableItem1= new QTableWidgetItem();
        iTableItem1->setFlags(iTableItem->flags() & ~(Qt::ItemIsEditable|Qt::ItemIsSelectable));
        iTableItem1->setSizeHint(size);
        iTableItem1->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, 1, iTableItem1);

        os_space->extra_mb = 0;

        //set column 2 to Extra size
        QTableWidgetItem * iTableItem2= new QTableWidgetItem();
        //iTableItem2->setFlags(iTableItem->flags() & ~Qt::ItemIsEditable);
        iTableItem2->setTextAlignment(Qt::AlignCenter);
        iTableItem2->setSizeHint(size);
        ui->tableWidget->setItem(i, 2, iTableItem2);

        os_space->total_mb = os_space->nominal_mb + os_space->extra_mb;

        //set column 3 to Total size
        QTableWidgetItem * iTableItem3= new QTableWidgetItem();
        iTableItem3->setFlags(iTableItem->flags() & ~(Qt::ItemIsEditable|Qt::ItemIsSelectable));
        iTableItem3->setTextAlignment(Qt::AlignCenter);
        iTableItem3->setSizeHint(size);
        ui->tableWidget->setItem(i, 3, iTableItem3);

        _spaces.append(os_space);

        //ui->tableWidget->editItem(iTableItem1);

        i++;
    }
    displayTable();
    _initialised=true;


    //ui->tableWidget->setVerticalHeaderLabels(m_RowHeader);

}

void adjustSizes::displayTable()
{
    TRACE
    _usedMB = 0;
    for (int row=0; row <_spaces.count(); row++)
    {
        ui->tableWidget->item(row,0)->setText(_spaces.at(row)->name);
        ui->tableWidget->item(row,1)->setText(QString::number(_spaces.at(row)->nominal_mb));
        ui->tableWidget->item(row,2)->setText(QString::number(_spaces.at(row)->extra_mb));
        ui->tableWidget->item(row,3)->setText(QString::number(_spaces.at(row)->total_mb));
        _usedMB += _spaces.at(row)->total_mb;
    }
    _freeMB = _availableMB - _usedMB;

    ui->usedLbl->setText(QString("%1: %2 MB").arg(tr("Used"), QString::number(_usedMB)));
    ui->freeLbl->setText(QString("%1: %2 MB").arg(tr("Free"), QString::number(_freeMB)));
    ui->capacityLbl->setText(QString("%1: %2 MB").arg(tr("Capacity"), QString::number(_availableMB)));
    qApp->processEvents();
}

void adjustSizes::on_tableWidget_cellChanged(int row, int column)
{
    TRACE
    qDebug()<< "Table widget changed row,col="<<row<<","<<column;
    if ((_initialised) && (column==2))
    {
        int nominal = ui->tableWidget->item(row,1)->text().toInt();
        int extra   = ui->tableWidget->item(row,2)->text().toInt();
        int total = nominal + extra;
        qDebug()<< "TOtal = "<<total;
        ui->tableWidget->item(row,3)->setText(QString::number(total));
    }
}

adjustSizes::~adjustSizes()
{
    delete ui;
}

void adjustSizes::on_balancePb_clicked()
{
    uint extra = _freeMB / _numexpandparts;
    for (int row=0; row <_spaces.count(); row++)
    {
        _spaces.at(row)->extra_mb += extra * _spaces.at(row)->numexpandparts;
        _spaces.at(row)->total_mb  = _spaces.at(row)->nominal_mb + _spaces.at(row)->extra_mb;
    }
    displayTable();
}

void adjustSizes::on_clearPb_clicked()
{
    for (int row=0; row <_spaces.count(); row++)
    {
        _spaces.at(row)->extra_mb=0;
        _spaces.at(row)->total_mb  = _spaces.at(row)->nominal_mb + _spaces.at(row)->extra_mb;
    }
    displayTable();
}

void adjustSizes::on_buttonBox1_accepted()
{
    TRACE
    //qDebug() << "on_buttonBox_accepted";
}

void adjustSizes::on_buttonBox1_rejected()
{
    TRACE
    //qDebug() << "on_buttonBox_rejected";
}
