FlashDialog::FlashDialog(QWidget *parent):QDialog(parent)
{
setupUi(this);
connect(okButton,SIGNAL(clicked()),this,SLOT(close()));
connect(&proc,SIGNAL(readyReadStandardOutput()),this,SLOT(readFromStdout()));
connect(&proc,SIGNAL(readyReadStandardError()),this,SLOT(readFromStdErr()));
connect(&proc,SIGNAL(error(QProcess::ProcessError)),this,SLOT(processError(QProcess::ProcessError)));
connect(&proc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(processFinished(int,QProcess::ExitStatus)));

}
void FlashDialog::setFilePath(QString filePath)
{
proc.start(filePath);
}

void FlashDialog::readFromStdout()
{
QByteArray newData=proc.readAllStandardOutput();
QString text=statusView->toPlainText() + QString::fromLocal8Bit(newData);
statusView->setPlainText(text);

}
void FlashDialog::readFromStdErr()
{
QByteArray newData=proc.readAllStandardError();
QString text=statusView->toPlainText() + QString::fromLocal8Bit(newData);
statusView->setPlainText(text);

}

void FlashDialog::processFinished(int exitCode,QProcess::ExitStatus exitStatus)
{
if(exitStatus==QProcess::CrashExit)
{
statusView->append("Program Crashed";);
}
}

void FlashDialog::enableOkButton()
{

}
void FlashDialog::processError(QProcess::ProcessError error)
{
if(error==QProcess::FailedToStart)
{
statusView->append("Failed to start";);
}
}
