#include "datetimedialog.h"
#include "ui_datetimedialog.h"

#include <sys/types.h>
#include <sys/time.h>

#include <QDateTime>

DateTimeDialog::DateTimeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DateTimeDialog)
{
    ui->setupUi(this);
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
}

DateTimeDialog::~DateTimeDialog()
{
    delete ui;
}

void DateTimeDialog::on_buttonBox_accepted()
{
    QDateTime parsedDate = ui->dateTimeEdit->dateTime();
    struct timeval tv;
    tv.tv_sec = parsedDate.toTime_t();
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
}
