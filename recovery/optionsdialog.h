#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "WidgetKeyboard.h"

#include <QDialog>
#include <QStringList>
#include <QWidget>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();
    void read();
    void write();
    bool recognise(const QString & key, const QString &value);

    QString remainderFile;
    QString remainderOptions;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_cbvk_toggled(bool checked);
    void my_focusChanged(QWidget * old, QWidget* nw);

private:
    Ui::OptionsDialog *ui;
    WidgetKeyboard *virtualKeyBoard;
    QString _lastWindow;
    QString _lastMenu;
    QWidget * _lastWidgetFocus;
};

#endif // OPTIONSDIALOG_H
