#ifndef DATETIMEDIALOG_H
#define DATETIMEDIALOG_H

#include <QDialog>

#include <sys/types.h>
#include <sys/time.h>

namespace Ui {
class DateTimeDialog;
}

class DateTimeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DateTimeDialog(QWidget *parent = 0);
    ~DateTimeDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DateTimeDialog *ui;
};

#endif // DateTimeDIALOG_H
