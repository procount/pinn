#ifndef TERMSDIALOG_H
#define TERMSDIALOG_H

#include <QDialog>
#include <QFile>

namespace Ui {
class TermsDialog;
}

class TermsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TermsDialog(const QString &osname, const QString & fTerms, QWidget *parent = 0);
    ~TermsDialog();


private:
    Ui::TermsDialog *ui;
};

#endif // TERMSDIALOG_H
