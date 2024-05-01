#ifndef ADJUSTSIZES_H
#define ADJUSTSIZES_H

#include "config.h"
#include "osinfo.h"
#include "partitioninfo.h"
#include <QDialog>

namespace Ui {
class adjustSizes;
}

class SpaceInfo
{

public:
    QString name;
    uint nominal_mb;
    uint extra_mb;
    uint total_mb;
    int numexpandparts;
};

class adjustSizes : public QDialog
{
    Q_OBJECT

public:
    explicit adjustSizes(uint provision, const QString & bootdrive, const QString & drive, QList<OsInfo *> _images, QWidget *parent = 0);
    ~adjustSizes();
    void displayTable();
    void calcTable();
    void calcSpace();

private slots:
    void on_buttonBox1_accepted();
    void on_buttonBox1_rejected();

    void on_tableWidget_cellChanged(int row, int column);
    void on_clearPb_clicked();
    void on_balancePb_clicked();

signals:
    void error(const QString &msg);
    void parsedImagesize(qint64 size);

private:
    bool _initialised;
    int _availableMB;
    int _freeMB;
    int _usedMB;
    Ui::adjustSizes *ui;
    QStringList m_TableHeader;
    QStringList m_RowHeader;
    uint _provision;
    const QString &_drive;
    const QString &_bootdrive;
    QList<SpaceInfo *> _spaces;
    uint _numparts;
    uint _numexpandparts;
    QList<OsInfo *> _images;
    int _extraSpacePerPartition; //delete!
    bool _multiDrives;
};

#endif // ADJUSTSIZES_H
