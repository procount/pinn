#ifndef ADJUSTSIZES_H
#define ADJUSTSIZES_H

#include "osinfo.h"
#include "partitioninfo.h"
#include <QDialog>

namespace Ui {
class adjustSizes;
}

struct space_info_t
{
    int nominal_mb;
    int extra_mb;
    int total_mb;
};

class adjustSizes : public QDialog
{
    Q_OBJECT

public:
    explicit adjustSizes(uint provision, const QString & drive, QList<OsInfo *> _images, QWidget *parent = 0);
    ~adjustSizes();

private slots:
    void on_buttonBox1_accepted();
    void on_tableWidget_cellChanged(int row, int column);

private:
    Ui::adjustSizes *ui;
    QStringList m_TableHeader;
    QStringList m_RowHeader;
    const QString &_drive;
    uint _provision;
};

#endif // ADJUSTSIZES_H
