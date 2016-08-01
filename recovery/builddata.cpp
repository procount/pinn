#include "builddata.h"
#include <QProcess>
#include <QRegExp>

BuildData::BuildData()
{
}

void BuildData::read(const char *filename)
{
    QFile f(filename);
    QString contents;

    if (f.exists())
    {
        f.open(QIODevice::ReadOnly);
        contents = f.readAll();
        f.close();
    }
    else
        contents="";

    _bdl     = contents.split("\n");
    foreach (const QString& str, _bdl)
    {
        if (str.contains("Version"))
            _version = str.split(":").at(1).trimmed();
        if (str.contains("Build-date"))
            _date = str.split(":").at(1).trimmed();
    }
}

bool BuildData::isEmpty()
{
    bool result = _version.isEmpty() && _date.isEmpty();
    return (result);
}


bool operator<(const BuildData& l, const BuildData& r)
{
    //Remove the initial letter.
    QString lver = l._version.mid(1);
    QString rver = r._version.mid(1);

    //split on any non-digit
    QRegExp sep("(\\D)");
    QStringList lparts = lver.split(sep);
    QStringList rparts = rver.split(sep);
    int llen=lparts.count();
    int rlen=rparts.count();

    bool result=false;
    qDebug() << "Comparing " << lver << rver;
    for (int i=0; i< rparts.count(); i++)
    {
        int l,r;
        if (i>=lparts.count())
        {
            //R has more parts
            result=true;
            break;
        }
        l=lparts.at(i).toInt();
        r=rparts.at(i).toInt();
        if (l < r)
        {
            //L < R
            result = true;
            break;
        }
        if (l > r)
        {
            //L > R
            result = false;
            break;
        }
    }
    return (result);
}
