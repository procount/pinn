#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <QDialog>
#include <QVariantMap>

namespace Ui {
class renamedialog;
}

class renamedialog : public QDialog
{
    Q_OBJECT

public:
    explicit renamedialog(QVariantMap map, QWidget *parent = 0);
    ~renamedialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::renamedialog *ui;
    QVariantMap _map;
};

#endif // RENAMEDIALOG_H
