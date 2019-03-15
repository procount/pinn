#include "termsdialog.h"
#include "ui_termsdialog.h"
#include "util.h"

#include <QFile>
#include <QDesktopWidget>
#include <QScreen>

TermsDialog::TermsDialog(const QString &osname, const QString & sTerms, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TermsDialog)
{
    ui->setupUi(this);

    QRect s = QApplication::desktop()->screenGeometry();
    resize(s.width()*8/10, s.height()*8/10);

    this->setWindowTitle(osname);
    QByteArray contents = getFileContents(sTerms);
    ui->textEdit->setHtml(contents);
}

TermsDialog::~TermsDialog()
{
    delete ui;
}


