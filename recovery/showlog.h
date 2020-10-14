#ifndef SHOWLOG_H
#define SHOWLOG_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class showlog;
}

class showlog : public QDialog
{
    Q_OBJECT

public:
    explicit showlog(QListWidget * list, QWidget *parent = 0);
    ~showlog();

private slots:
    void on_refresh_clicked();

private:
    Ui::showlog *ui;
};

#endif // SHOWLOG_H
