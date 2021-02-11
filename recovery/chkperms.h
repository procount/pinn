#ifndef CHKPERMS_H
#define CHKPERMS_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class chkperms;
}

class chkperms : public QDialog
{
    Q_OBJECT

public:
    explicit chkperms(QListWidget * list, QWidget *parent = 0);
    ~chkperms();

private:
    Ui::chkperms *ui;
};


#endif // CHKPERMS_H
