#include "mydebug.h"
#include <QString>


int MyDebug::level = 0;

MyDebug::MyDebug(const char * func) :
    name(func)
{
#if DBG_FUNC
    QString output;
    for (int i=0; i< level ; i++)
        output += " ";
    output += ">";
    output += name;
    qDebug() << output;
#endif
    level++;
}

MyDebug::~MyDebug()
{
    level--;
#if DBG_FUNC
    QString output;
    for (int i=0; i< level ; i++)
        output += " ";
    output += "<";
    output += name;
    qDebug() << output;
#endif
}

void MyDebug::outstring(QString out)
{
#if DBG_OUT
    QString output;
    for (int i=0; i< level ; i++)
        output += " ";
    output += out;
    qDebug() << output.toUtf8().constData();
#endif
}
