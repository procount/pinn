#include "passwd.h"
#include "ui_passwd.h"
#include "json.h"
#include <algorithm>
#include <QHash>
#include <QProcess>
#include <QDebug>

Passwd::Passwd(QVariantMap Map, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Passwd)
{
    ui->setupUi(this);
    _map = Map;
    ui->checkBox->setChecked(false);
    _nScore = 0;
    _sComplexity = "Weak";
}

Passwd::~Passwd()
{
    delete ui;
}

void Passwd::on_passwordEdit_textChanged(const QString &pass)
{
    CheckPasswordWithDetails(pass);
    ui->strength->setText(_sComplexity);
    CheckPasswdsEqual();
}

QString Passwd::strReverse(QString str)
{
    QByteArray ba = str.toLatin1();
    char *d = ba.data();
    std::reverse(d, d+str.length());
    return QString(d);
}

void Passwd::CheckPasswordWithDetails(QString pwd)
{
    // Init Vars
    int nScore = 0;
    QString sComplexity = "";
    int iUpperCase = 0;
    int iLowerCase = 0;
    int iDigit = 0;
    int iSymbol = 0;
    int iRepeated = 1;
    QHash<QChar,int> htRepeated;
    int iMiddle = 0;
    int iMiddleEx = 1;
    int ConsecutiveMode = 0;
    int iConsecutiveUpper = 0;
    int iConsecutiveLower = 0;
    int iConsecutiveDigit = 0;
    QString sAlphas = "abcdefghijklmnopqrstuvwxyz";
    QString sNumerics = "01234567890";
    int nSeqAlpha = 0;
    int nSeqChar = 0;
    int nSeqNumber = 0;


    // Scan password
    for (int i=0; i< pwd.length(); i++)
    {
        QChar ch = pwd.at(i);
        // Count digits
        if (ch.isDigit())
        {
            iDigit++;

            if (ConsecutiveMode == 3)
                iConsecutiveDigit++;
            ConsecutiveMode = 3;
        }

        // Count uppercase characters
        if (ch.isUpper())
        {
            iUpperCase++;
            if (ConsecutiveMode == 1)
                iConsecutiveUpper++;
            ConsecutiveMode = 1;
        }

        // Count lowercase characters
        if (ch.isLower())
        {
            iLowerCase++;
            if (ConsecutiveMode == 2)
                iConsecutiveLower++;
            ConsecutiveMode = 2;
        }

        // Count symbols
        if (ch.isSymbol() || ch.isPunct())
        {
            iSymbol++;
            ConsecutiveMode = 0;
        }

        // Count repeated letters
        if (ch.isLetter())
        {
            if (htRepeated.contains(ch.toLower()))
                iRepeated++;
            else
                htRepeated.insert(ch.toLower(), 0);

            if (iMiddleEx > 1)
                iMiddle = iMiddleEx - 1;
        }

        if (iUpperCase > 0 || iLowerCase > 0)
        {
            if (ch.isDigit() || ch.isSymbol())
                iMiddleEx++;
        }
    }

    // Check for sequential alpha string patterns (forward and reverse)
    for (int s = 0; s < 23; s++)
    {
        QString sFwd = sAlphas.mid(s, 3);
        QString sRev = strReverse(sFwd);
        if (pwd.toLower().indexOf(sFwd) != -1 || pwd.toLower().indexOf(sRev) != -1)
        {
            nSeqAlpha++;
            nSeqChar++;
        }
    }

    // Check for sequential numeric string patterns (forward and reverse)
    for (int s = 0; s < 8; s++)
    {
        QString sFwd = sNumerics.mid(s, 3);
        QString sRev = strReverse(sFwd);
        if (pwd.toLower().indexOf(sFwd) != -1 || pwd.toLower().indexOf(sRev) != -1)
        {
            nSeqNumber++;
            nSeqChar++;
        }
    }

    // Calcuate score

    // Score += 4 * Password Length
    nScore = 4 * pwd.length();

    // if we have uppercase letters Score +=(number of uppercase letters *2)
    if (iUpperCase > 0)
    {
        nScore += ((pwd.length() - iUpperCase) * 2);
    }

    // if we have lowercase letetrs Score +=(number of lowercase letters *2)
    if (iLowerCase > 0)
    {
        nScore += ((pwd.length() - iLowerCase) * 2);
    }

    // Score += (Number of digits *4)
    nScore += (iDigit * 4);

    // Score += (Number of Symbols * 6)
    nScore += (iSymbol * 6);

    // Score += (Number of digits or symbols in middle of password *2)
    nScore += (iMiddle * 2);

    //requirements
    int requirements = 0;
    if (pwd.length() >= 8) requirements++;     // Min password length
    if (iUpperCase > 0) requirements++;      // Uppercase letters
    if (iLowerCase > 0) requirements++;      // Lowercase letters
    if (iDigit > 0) requirements++;          // Digits
    if (iSymbol > 0) requirements++;         // Symbols

    // If we have more than 3 requirements then
    if (requirements > 3)
    {
        // Score += (requirements *2)
        nScore += (requirements * 2);
    }

    //
    // Deductions
    //

    // If only letters then score -=  password length
    if (iDigit == 0 && iSymbol == 0)
    {
        nScore -= pwd.length();
    }

    // If only digits then score -=  password length
    if (iDigit == pwd.length())
    {
        nScore -= pwd.length();
    }

    // If repeated letters used then score -= (iRepeated * (iRepeated - 1));
    if (iRepeated > 1)
    {
        nScore -= (iRepeated * (iRepeated - 1));
    }

    // If Consecutive uppercase letters then score -= (iConsecutiveUpper * 2);
    nScore -= (iConsecutiveUpper * 2);

    // If Consecutive lowercase letters then score -= (iConsecutiveUpper * 2);
    nScore -= (iConsecutiveLower * 2);

    // If Consecutive digits used then score -= (iConsecutiveDigits* 2);
    nScore -= (iConsecutiveDigit * 2);

    // If password contains sequence of letters then score -= (nSeqAlpha * 3)
    nScore -= (nSeqAlpha * 3);

    // If password contains sequence of digits then score -= (nSeqNumber * 3)
    nScore -= (nSeqNumber * 3);

    /* Determine complexity based on overall score */
    if (nScore > 100) { nScore = 100; } else if (nScore < 0) { nScore = 0; }
    if (nScore >= 0 && nScore < 20) { sComplexity = tr("Very Weak"); }
    else if (nScore >= 20 && nScore < 40) { sComplexity = tr("Weak"); }
    else if (nScore >= 40 && nScore < 60) { sComplexity = tr("Good"); }
    else if (nScore >= 60 && nScore < 80) { sComplexity = tr("Strong"); }
    else if (nScore >= 80 && nScore <= 100) { sComplexity = tr("Very Strong"); }

    // Store score and complexity in dataset
    _nScore = nScore;
    _sComplexity = sComplexity;
}


void Passwd::on_checkBox_toggled(bool checked)
{
    if (checked)
    {
        ui->passwordEdit->setEchoMode(QLineEdit::Normal);
        ui->passwordEdit_2->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->passwordEdit->setEchoMode(QLineEdit::Password);
        ui->passwordEdit_2->setEchoMode(QLineEdit::Password);
    }
}

void Passwd::on_passwordEdit_2_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    CheckPasswdsEqual();
}

bool Passwd::CheckPasswdsEqual(void)
{
    QString pass1 = ui->passwordEdit->text();
    QString pass2 = ui->passwordEdit_2->text();
    QPalette palette;
    bool equal;
    if (equal=(pass1==pass2))
    {
        palette.setColor(QPalette::Base,Qt::white);
        palette.setColor(QPalette::Text,Qt::black);
    }
    else
    {
        palette.setColor(QPalette::Base,Qt::red);
        palette.setColor(QPalette::Text,Qt::white);
    }
    ui->passwordEdit_2->setPalette(palette);
    return(equal);
}

int executeLog(int log, const QString &cmd)
{
    QProcess proc;
    int errorcode=0;

    proc.setProcessChannelMode(proc.MergedChannels);
    proc.start(cmd);
    proc.waitForFinished(-1);
    errorcode = proc.exitCode();
    if (log)
        qDebug() << cmd << "\n" << proc.readAllStandardOutput() << "\n"  << proc.readAllStandardError();
    return (errorcode);
}


void Passwd::accept()
{
    if (CheckPasswdsEqual())
    {
        //if (QProcess::execute("mount -t ext4 " SETTINGS_PARTITION " /settings") != 0)
        QVariantList l = _map.value("partitions").toList();
        if (l.length()>1)
        {
            QString p = l.at(1).toString();
            executeLog(1,QString("sh -c \"mkdir /tmp/part\""));
            executeLog(1,QString("sh -c \"mount "+ p+" /tmp/part\""));
            executeLog(1,QString("sh -c \"mount -o bind /tmp/part/etc /etc\""));
            //get username
            QString username = ui->username->text();
            //get pass1
            QString password = ui->passwordEdit->text();

            //Change the password
            executeLog(1,QString("sh -c \"echo " +username +":" +password +" | chpasswd\""));

/* Shadow format:

smithj:Ep6mckrOLChF.:10063:0:99999:7:::
As with the passwd file, each field in the shadow file is also separated with ":" colon characters, and are as follows:

1. Username, up to 8 characters. Case-sensitive, usually all lowercase. A direct match to the username in the /etc/passwd file.
2. Password, 13 character encrypted. A blank entry (eg. ::) indicates a password is not required to log in (usually a bad idea), and a ``*'' entry (eg. :*:) indicates the account has been disabled.
3. The number of days (since January 1, 1970) since the password was last changed.
4. The number of days before password may be changed (0 indicates it may be changed at any time)
5. The number of days after which password must be changed (99999 indicates user can keep his or her password unchanged for many, many years)
6. The number of days to warn user of an expiring password (7 for a full week)
7. The number of days after password expires that account is disabled
8. The number of days since January 1, 1970 that an account has been disabled
9. A reserved field for possible future use
*/
            //set field 3 of /etc/shadow to 16858?
            //set field 5 of /etc/shadow to 99999 so it does nto expire straight-away (in fact ever!)
            executeLog(1,QString("sh -c \"sed -i.bak -e \"s/\(" +username+ "\):\(.*\):\(.*\):\(.*\):\(.*\):\(.*\):\(.*\):\(.*\):\(.*\)/\1:\2:16850:\4:99999:\6\:\7:\8:\9/\" /etc/shadow\""));

            executeLog(1,QString("sh -c \"umount /etc\""));
            executeLog(1,QString("sh -c \"umount /tmp/part\""));

            done(QDialog::Accepted);
        }
    }
}

void Passwd::on_pushButton_clicked()
{
    if (_map.contains("name"))
    {
        //Get name of selected OS
        QString name = _map.value("name").toString();
        //Find OS name on installed list
        QVariantList i = Json::loadFromFile("/settings/installed_os.json").toList();
        foreach (QVariant v, i)
        {
            QVariantMap m = v.toMap();
            QString n = m.value("name").toString();
            if (name==n)
            {
                //Found it!
                if (m.contains("username"))
                    ui->username->setText(m.value("username").toString());
                else
                    ui->username->setText("pi");

                if (m.contains("password"))
                {
                    ui->passwordEdit->setText(m.value("password").toString());
                    ui->passwordEdit_2->setText(m.value("password").toString());
                }
                else
                {
                    ui->passwordEdit->setText("");
                    ui->passwordEdit_2->setText("");
                }
            }
        }
    }
}
