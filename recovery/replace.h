#ifndef REPLACE_H
#define REPLACE_H

#include "osgroup.h"

#include <QDialog>

namespace Ui {
class replace;
}

class replace : public QDialog
{
    Q_OBJECT

public:
    explicit replace(QList<QListWidgetItem *> replacementList, QList<QListWidgetItem *> installedList, QWidget *parent = 0);
    ~replace();

    QList<QVariantMap> getMappedList();

private slots:
    void on_buttonBox1_accepted();

private:
    Ui::replace *ui;
    QStringList m_TableHeader;
};

#endif // REPLACE_H
