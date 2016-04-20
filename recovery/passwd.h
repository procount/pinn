#ifndef PASSWD_H
#define PASSWD_H

#include <QDialog>
#include <QVariantList>
#include <QVariantMap>
#include <QPalette>

extern int executeLog(int log, const QString &cmd);

namespace Ui {
class Passwd;
}

class Passwd : public QDialog
{
    Q_OBJECT

public:
    explicit Passwd(QVariantMap map, QWidget *parent = 0);
    void CheckPasswordWithDetails(QString pwd);
    QString strReverse(QString str);

    ~Passwd();
    bool CheckPasswdsEqual(void);
    void accept();

private slots:
    void on_passwordEdit_textChanged(const QString &arg1);

    void on_checkBox_toggled(bool checked);

    void on_passwordEdit_2_textChanged(const QString &arg1);

    void on_pushButton_clicked();

private:
    Ui::Passwd *ui;
    int _nScore;
    QString _sComplexity;
    QVariantMap _map;

};

#endif // PASSWD_H
