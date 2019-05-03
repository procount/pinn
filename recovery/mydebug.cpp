#include "mydebug.h"
#include <QString>

int MyDebug::_level = 0;

MyDebug::MyDebug(const char * funcname, int display) :
    _local_level(0), _name(funcname), _display(display)
{
#if DBG_FUNC
    if (_display)
        qDebug() << this->header() << ">" << _name;
#endif
    _level++;
    _local_level++;
}

MyDebug::~MyDebug()
{
    _level--;
    _local_level=0;
#if DBG_FUNC
    if (_display)
        qDebug() << this->header() << "<" << _name;
#endif
}

const char * MyDebug::header()
{
    QString out(" ");
    return out.repeated(_level+1+_local_level).toLatin1().data();
}
