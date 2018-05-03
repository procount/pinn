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
    int rerunPostInstallScript(QListWidget * list);

private slots:
    void on_buttonBox_accepted();

signals:
    void error(const QString &msg);
    void statusUpdate(const QString &msg);

private:
    Ui::repair *ui;
    QListWidget * _listinstalled;
    QString _drive;
};

#endif // REPAIR_H
