#ifndef FSCHECK_H
#define FSCHECK_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class fscheck;
}

class fscheck : public QDialog
{
    Q_OBJECT

public:
    explicit fscheck(QListWidget * list, QWidget *parent = 0);
    ~fscheck();

private:
    Ui::fscheck *ui;
};
#endif // FSCHECK_H
