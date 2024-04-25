#ifndef ADJUSTSIZES_H
#define ADJUSTSIZES_H

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
    explicit adjustSizes(uint provision, const QString & drive, QList<OsInfo *> _images, QWidget *parent = 0);
    ~adjustSizes();
    void displayTable();

private slots:
    void on_buttonBox1_accepted();
    void on_buttonBox1_rejected();

    void on_tableWidget_cellChanged(int row, int column);
    void on_clearPb_clicked();
    void on_balancePb_clicked();

private:
    bool _initialised;
    uint _availableMB;
    uint _freeMB;
    uint _usedMB;
    Ui::adjustSizes *ui;
    QStringList m_TableHeader;
    QStringList m_RowHeader;
    uint _provision;
    const QString &_drive;
    QList<SpaceInfo *> _spaces;
    uint _numparts;
    uint _numexpandparts;

};

#endif // ADJUSTSIZES_H
