#ifndef REPAIR_H
#define REPAIR_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class repair;
}

class repair : public QDialog
{
    Q_OBJECT

public:
    explicit repair(QListWidget * list,  const QString &rootdrive, QWidget *parent = 0);
    ~repair();

private slots:
    void on_buttonBox_accepted();

private:
    QListWidget * _listinstalled;
    QString _drive;
    Ui::repair *ui;
};

#endif // REPAIR_H
