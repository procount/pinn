#include "json.h"
#include "adjustsizes.h"
#include "ui_adjustsizes.h"
#include "util.h"

#define LOCAL_DBG_ON   1
#define LOCAL_DBG_FUNC 1
#define LOCAL_DBG_OUT  1
#define LOCAL_DBG_MSG  0

#include "mydebug.h"

#include <QDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>

adjustSizes::adjustSizes(QList<OsInfo *> _images, ulong totalSize,  ulong availableMB, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::adjustSizes)
{
    TRACE
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(5);

    m_TableHeader << "OS Name" << "Default Size" << "Extra" << "Spare" << "Total";
    ui->tableWidget->setHorizontalHeaderLabels(m_TableHeader);
    //ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    //ui->tableWidget->setShowGrid(false);
    //ui->tableWidget->setStyleSheet("QTableView {selection-background-color: red;}");
    //ui->tableWidget->setGeometry(QApplication::desktop()->screenGeometry());

    ui->tableWidget->setRowCount(_images.count());
    int i=0;
    foreach (OsInfo *image, _images)
    {

        //set column 0 to OS name
        QTableWidgetItem * iTableItem= new QTableWidgetItem(image->name());
        ui->tableWidget->setItem(i, 0, iTableItem);

        QList<PartitionInfo *> *partitions = image->partitions();
        if (partitions->isEmpty())
        {
            //emit error(tr("partitions.json invalid"));
            return;
        }
#if 0
        if (nameMatchesRiscOS( image->folder() ))
        {
            /* Check the riscos_offset in os.json matches what we're expecting.
               In theory we shouldn't hit either of these errors because the invalid RISC_OS
               should have been filtered out already (not added to OS-list) in mainwindow.cpp */
            if (image->riscosOffset())
            {
                if (image->riscosOffset() != RISCOS_OFFSET)
                {
                    emit error(tr("RISCOS cannot be installed. RISCOS offset value mismatch."));
                    return;
                }
            }
            else
            {
                emit error(tr("RISCOS cannot be installed. RISCOS offset value missing."));
                return;
            }
            if (startSector > RISCOS_SECTOR_OFFSET-2048)
            {
                emit error(tr("RISCOS cannot be installed. Size of recovery partition too large."));
                return;
            }

            totalnominalsize += (RISCOS_SECTOR_OFFSET - startSector)/2048;

            partitions->first()->setRequiresPartitionNumber(6);
            partitions->first()->setOffset(RISCOS_SECTOR_OFFSET);
            partitions->last()->setRequiresPartitionNumber(7);
        }

        foreach (PartitionInfo *partition, *partitions)
        {
            numparts++;
            if ( partition->wantMaximised() )
                numexpandparts++;
            uint nominalsize = partition->partitionSizeNominal();
            totalnominalsize += nominalsize;
            totaluncompressedsize += partition->uncompressedTarballSize();

            if ( (partition->fsType() == "ext4") || (partition->fsType() == "ext3") )
            {
                totaluncompressedsize += nominalsize / 20 ; /* overhead for file system meta data */
            }
            int reqPart = partition->requiresPartitionNumber();
            if (reqPart)
            {
                if (partitionMap.contains(reqPart))
                {
                    emit error(tr("More than one operating system requires partition number %1").arg(reqPart));
                    return;
                }
                if (reqPart == 1 || reqPart == 5)
                {
                    emit error(tr("Operating system cannot require a system partition (1,5)"));
                    return;
                }
                if ((reqPart == 2 && partitionMap.contains(4)) || (reqPart == 4 && partitionMap.contains(2)))
                {
                    emit error(tr("Operating system cannot claim both primary partitions 2 and 4"));
                    return;
                }

                partition->setPartitionDevice(partdev(_drive, reqPart));
                partitionMap.insert(reqPart, partition);
            }

            /* Maximum overhead per partition for alignment */
#ifdef SHRINK_PARTITIONS_TO_MINIMIZE_GAPS
            if (partition->wantMaximised() || (partition->partitionSizeNominal()*2048) % PARTITION_ALIGNMENT != 0)
                totalnominalsize += PARTITION_ALIGNMENT/2048;
#else
            totalnominalsize += PARTITION_ALIGNMENT/2048;
#endif
        }
#endif
    i++;
    }

#if 0
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
#endif
}

adjustSizes::~adjustSizes()
{
    delete ui;
}


void adjustSizes::on_buttonBox1_accepted()
{
    //qDebug() << "on_buttonBox_accepted";
}

