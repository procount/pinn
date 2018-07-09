#ifndef backupdialog_H
#define backupdialog_H

#include <QDialog>
#include <QVariantMap>

namespace Ui {
class backupdialog;
}

class backupdialog : public QDialog
{
    Q_OBJECT

public:
    explicit backupdialog(QVariantMap &map, QWidget *parent = 0);
    ~backupdialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::backupdialog *ui;
    QVariantMap &_map;
};

#endif // backupdialog_H
