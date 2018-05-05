#ifndef RERUNSETUP_H
#define RERUNSETUP_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class rerunsetup;
}

class rerunsetup : public QDialog
{
    Q_OBJECT

public:
    explicit rerunsetup(QListWidget * list, const QString &rootdrive, QWidget *parent = 0);
    ~rerunsetup();

private:
    QString _drive;
    Ui::rerunsetup *ui;
};

#endif // RERUNSETUP_H
