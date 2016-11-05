#include "mydebug.h"
#include <QString>


int MyDebug::level = 0;

MyDebug::MyDebug(const char * func) :
    name(func)
{
    QString output;
    for (int i=0; i< level ; i++)
        output += " ";
    output += ">";
    output += name;
#if DBG_FUNC
    qDebug() << output;
#endif
    level++;
}

MyDebug::~MyDebug()
{
    QString output;
    level--;
    for (int i=0; i< level ; i++)
        output += " ";
    output += "<";
    output += name;
#if DBG_FUNC
    qDebug() << output;
#endif
}

void MyDebug::outstring(QString out)
{
    QString output;
    for (int i=0; i< level ; i++)
        output += " ";
    output += out;
#if DBG_FUNC
    qDebug() << output.toUtf8().constData();
#endif
}
