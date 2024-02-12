#ifndef ADJUSTSIZES_H
#define ADJUSTSIZES_H

#include "osinfo.h"
#include "partitioninfo.h"
#include <QDialog>

namespace Ui {
class adjustSizes;
}

class adjustSizes : public QDialog
{
    Q_OBJECT

public:
    explicit adjustSizes(QList<OsInfo *> _images, ulong totalSize, ulong availableMB, QWidget *parent = 0);
    ~adjustSizes();

private slots:
    void on_buttonBox1_accepted();

private:
    Ui::adjustSizes *ui;
    QStringList m_TableHeader;
};

#endif // ADJUSTSIZES_H
