#include "json.h"
#include "adjustsizes.h"
#include "ui_adjustsizes.h"
#include "util.h"

#define LOCAL_DBG_ON   0
#define LOCAL_DBG_FUNC 0
#define LOCAL_DBG_OUT  0
#define LOCAL_DBG_MSG  0

#include "mydebug.h"

#include <QApplication>
#include <QDebug>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>

adjustSizes::adjustSizes(uint provision, const QString &bootdrive, const QString &rootdrive,  QList<OsInfo *> images, QWidget *parent) :
    _provision(provision),  _bootdrive(bootdrive), _drive(rootdrive), _images(images), QDialog(parent),
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



    ui->tableWidget->setColumnCount(3);

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
        QColor bgnd(200,200,200);

        os_space->name = image->name();
        m_RowHeader = m_RowHeader << QString(os_space->name);

        os_space->nominal_mb = nominalImageSize;

        //set column 0 to nominal size
        QTableWidgetItem * iTableItem1= new QTableWidgetItem();
        iTableItem1->setFlags(iTableItem1->flags() & ~(Qt::ItemIsEditable|Qt::ItemIsSelectable));
        iTableItem1->setSizeHint(size);
        iTableItem1->setTextAlignment(Qt::AlignCenter);
        iTableItem1->setBackgroundColor(bgnd);
        ui->tableWidget->setItem(i, 0, iTableItem1);

        os_space->extra_mb = 0;

        //set column 1 to Extra size
        QTableWidgetItem * iTableItem2= new QTableWidgetItem();
        //iTableItem2->setFlags(iTableItem2->flags() & ~Qt::ItemIsEditable);
        iTableItem2->setTextAlignment(Qt::AlignCenter);
        iTableItem2->setSizeHint(size);
        ui->tableWidget->setItem(i, 1, iTableItem2);

        os_space->total_mb = os_space->nominal_mb + os_space->extra_mb;

        //set column 2 to Total size
        QTableWidgetItem * iTableItem3= new QTableWidgetItem();
        iTableItem3->setFlags(iTableItem3->flags() & ~(Qt::ItemIsEditable|Qt::ItemIsSelectable));
        iTableItem3->setTextAlignment(Qt::AlignCenter);
        iTableItem3->setSizeHint(size);
        iTableItem3->setBackgroundColor(bgnd);
        ui->tableWidget->setItem(i, 2, iTableItem3);

        _spaces.append(os_space);

        //ui->tableWidget->editItem(iTableItem1);

        i++;
    }
    calcTable();
    on_balancePb_clicked();
    ui->tableWidget->setCurrentCell(0,2);
    _initialised=true;


    ui->tableWidget->setVerticalHeaderLabels(m_RowHeader);

}

void adjustSizes::displayTable()
{
    TRACE
    for (int row=0; row <_spaces.count(); row++)
    {   QString nominal = QString::number(_spaces.at(row)->nominal_mb);
        if (_spaces.at(row)->numexpandparts==0)
            nominal += " (*)";
        ui->tableWidget->item(row,0)->setText(nominal);
        ui->tableWidget->item(row,1)->setText(QString::number(_spaces.at(row)->extra_mb));
        ui->tableWidget->item(row,2)->setText(QString::number(_spaces.at(row)->total_mb));
    }

    ui->usedLbl->setText(QString("%1: %2 MB").arg(tr("Used"), QString::number(_usedMB)));
    ui->freeLbl->setText(QString("%1: %2 MB").arg(tr("Free"), QString::number(_freeMB)));
    ui->capacityLbl->setText(QString("%1: %2 MB").arg(tr("Capacity"), QString::number(_availableMB)));
    qApp->processEvents();
}

void adjustSizes::on_tableWidget_cellChanged(int row, int column)
{
    TRACE
    qDebug()<< "Table widget changed row,col="<<row<<","<<column;

    if ((_initialised) && (column==1))
    {
        _spaces.at(row)->extra_mb =  ui->tableWidget->item(row,1)->text().toInt();

        //_initialised=false;
        calcTable();

        if (_freeMB<0)
        {
            _spaces.at(row)->extra_mb += _freeMB ;
            calcTable();
        }
        displayTable();
        //_initialised=true;
    }
}

adjustSizes::~adjustSizes()
{
    delete ui;
}

void adjustSizes::calcTable()
{
    _usedMB=0;
    for (int row=0; row <_spaces.count(); row++)
    {
        _spaces.at(row)->total_mb  = _spaces.at(row)->nominal_mb + _spaces.at(row)->extra_mb;
        _usedMB += _spaces.at(row)->total_mb;
    }
    _freeMB = _availableMB - _usedMB;
}

void adjustSizes::on_balancePb_clicked()
{
    uint extra = _freeMB / _numexpandparts;
    for (int row=0; row <_spaces.count(); row++)
    {
        _spaces.at(row)->extra_mb += extra * _spaces.at(row)->numexpandparts;
    }
    calcTable();
    displayTable();
}

void adjustSizes::on_clearPb_clicked()
{
    for (int row=0; row <_spaces.count(); row++)
    {
        _spaces.at(row)->extra_mb=0;
    }
    calcTable();
    displayTable();
}

void adjustSizes::on_buttonBox1_accepted()
{
    TRACE
    //qDebug() << "on_buttonBox_accepted";
    int row=0;
    foreach (OsInfo *image, _images)
    {
        SpaceInfo * os_space = _spaces.at(row);
        int extra = os_space->extra_mb;
        int eachpart = 0;
        if (extra>0)
        {
            int numexpands = (os_space->numexpandparts==0) ? 1 : os_space->numexpandparts;
            eachpart = extra/numexpands;
        }

        QList<PartitionInfo *> *partitions = image->partitions();
        foreach (PartitionInfo *partition, *partitions)
        {
            partition->setPartitionSizeExtra(0);
            if ((os_space->numexpandparts==0) && (extra>0) && partition == partitions->last())
                partition->setPartitionSizeExtra(extra);
            if (partition->wantMaximised())
                partition->setPartitionSizeExtra(eachpart);
        }
        row++;
    }
    QDialog::accepted();
}

void adjustSizes::on_buttonBox1_rejected()
{
    TRACE
    //qDebug() << "on_buttonBox_rejected";
}


void adjustSizes::calcSpace()
{
    /* Calculate space requirements, and check special requirements */
    uint totalnominalsize = 0, totaluncompressedsize = 0, numparts = 0, numexpandparts = 0;
    uint startSector = getFileContents(sysclassblock(_drive, 5)+"/start").trimmed().toUInt()
                    + getFileContents(sysclassblock(_drive, 5)+"/size").trimmed().toUInt();
    uint totalSectors = getFileContents(sysclassblock(_drive)+"/size").trimmed().toUInt();
    uint availableMB = (totalSectors-startSector-_provision)/2048;

    /* key: partition number, value: partition information */
    QMap<int, PartitionInfo *> partitionMap, bootPartitionMap;

    foreach (OsInfo *image, _images)
    {
        QList<PartitionInfo *> *partitions = image->partitions();
        if (partitions->isEmpty())
        {
            emit error(tr("partitions.json invalid"));
            return;
        }

        /* If RISCOS is selected, it must be placed at partitions 6/7 */
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

        /* Check and assign any required partition numbers */
        /* And calculate any spare space to be distributed */
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
    }


    if (numexpandparts)
    {
        /* Extra spare space available for partitions that want to be expanded */
        _extraSpacePerPartition = (availableMB-totalnominalsize)/numexpandparts;

    }

    emit parsedImagesize(qint64(totaluncompressedsize)*1024*1024);

    if (totalnominalsize > availableMB)
    {
        emit error(tr("Not enough disk space. Need %1 MB, got %2 MB").arg(QString::number(totalnominalsize), QString::number(availableMB)));
        return;
    }

    /* Assign logical partition numbers to partitions that did not reserve a special number */
    int pnr, bootpnr;
    uint offset = 0;
    if (partitionMap.isEmpty())
        pnr = 6;
    else
        pnr = qMax(partitionMap.keys().last(), 5)+1;

    if (_multiDrives)
    {
        bootpnr = 6;
        offset = getFileContents(sysclassblock(_bootdrive, 5)+"/start").trimmed().toUInt()
               + getFileContents(sysclassblock(_bootdrive, 5)+"/size").trimmed().toUInt();
    }

    foreach (OsInfo *image, _images)
    {
        foreach (PartitionInfo *partition, *(image->partitions()))
        {
            if (!partition->requiresPartitionNumber())
            {
                if (_multiDrives && partition->bootable() && !partition->wantMaximised() )
                {
                    if (bootpnr >=63 )
                    {
                        emit error(tr("Cannot boot partitions > 62. Reduce the number of OSes"));
                        return;
                    }
                    bootPartitionMap.insert(bootpnr, partition);
                    partition->setPartitionDevice(partdev(_bootdrive, bootpnr));
                    bootpnr++;
                    offset += PARTITION_GAP;

                    /* Align at 4 MiB offset */
                    if (offset % PARTITION_ALIGNMENT != 0)
                    {
                            offset += PARTITION_ALIGNMENT-(offset % PARTITION_ALIGNMENT);
                    }
                    partition->setOffset(offset);
                    uint partsizeSectors = partition->partitionSizeNominal() * 2048;
                    partition->setPartitionSizeSectors(partsizeSectors);
                    offset += partsizeSectors;
                }
                else
                {
                    if (partition->bootable() && pnr >=63 )
                    {
                        emit error(tr("Cannot boot partitions > #62. Reduce the number of OSes"));
                        return;
                    }
                    partitionMap.insert(pnr, partition);
                    partition->setPartitionDevice(partdev(_drive, pnr));
                    pnr++;
                }
            }
        }
    }

    /* Set partition starting sectors and sizes.
     * First allocate space to all logical partitions, then to primary partitions */
    QList<PartitionInfo *> log_before_prim = partitionMap.values();
    if (!log_before_prim.isEmpty() && log_before_prim.first()->requiresPartitionNumber() == 2)
        log_before_prim.push_back(log_before_prim.takeFirst());
    if (!log_before_prim.isEmpty() && log_before_prim.first()->requiresPartitionNumber() == 3)
        log_before_prim.push_back(log_before_prim.takeFirst());
    if (!log_before_prim.isEmpty() && log_before_prim.first()->requiresPartitionNumber() == 4)
        log_before_prim.push_back(log_before_prim.takeFirst());

    offset = startSector;

    foreach (PartitionInfo *p, log_before_prim)
    {
        if (p->offset()) /* OS wants its partition at a fixed offset */
        {
            if (p->offset() <= offset)
            {
                emit error(tr("Fixed partition offset too low"));
                return;
            }

            offset = p->offset();
        }
        else
        {
            offset += PARTITION_GAP;
            /* Align at 4 MiB offset */
            if (offset % PARTITION_ALIGNMENT != 0)
            {
                    offset += PARTITION_ALIGNMENT-(offset % PARTITION_ALIGNMENT);
            }

            p->setOffset(offset);
        }

        uint partsizeMB = p->partitionSizeNominal();
#if 1
        if ( p->wantMaximised() )
            partsizeMB += _extraSpacePerPartition;
#else
        partsizeMB += p->partitionSizeExtra();
#endif
        //qDebug()<< "Partsize = "<< p->partitionSizeNominal() << " + " << partsizeMB - p->partitionSizeNominal() << " = " << partsizeMB;

        uint partsizeSectors = partsizeMB * 2048;

        if (p == log_before_prim.last())
        {
            /* Let last partition have any remaining space that we couldn't divide evenly */
            uint spaceleft = totalSectors - offset - partsizeSectors - _provision;

            if (spaceleft > 0 && p->wantMaximised())
            {
                partsizeSectors += spaceleft;
            }
        }
        else
        {
#ifdef SHRINK_PARTITIONS_TO_MINIMIZE_GAPS
            if (partsizeSectors % PARTITION_ALIGNMENT == 0 && p->fsType() != "raw")
            {
                /* Partition size is dividable by 4 MiB
                   Take off a couple sectors of the end of our partition to make room
                   for the EBR of the next partition, so the next partition can
                   align nicely without having a 4 MiB gap */
                partsizeSectors -= PARTITION_GAP;
            }
#endif
            if (p->wantMaximised() && (partsizeSectors+PARTITION_GAP) % PARTITION_ALIGNMENT != 0)
            {
                /* Enlarge partition to close gap to next partition */
                partsizeSectors += PARTITION_ALIGNMENT-((partsizeSectors+PARTITION_GAP) % PARTITION_ALIGNMENT);
            }
        }

        p->setPartitionSizeSectors(partsizeSectors);
        //qDebug()<<"partsizesectors: "<<partsizeSectors;
        offset += partsizeSectors;
    }
}
