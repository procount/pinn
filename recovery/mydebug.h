#ifndef MYDEBUG_H
#define MYDEBUG_H

#include <QDebug>

#define DO_DBG 1
#define DBG_FUNC 1

#define DBG_OUT(x) dbg.outstring(x);
#define NO_DBG(x)


#if DO_DBG
#define DBG(x) DBG_OUT(x)
#define MYDEBUG MyDebug dbg(__PRETTY_FUNCTION__);
#else
#define DBG(x) NO_DBG(x)
#define MYDEBUG
#endif

class MyDebug
{
public:

    explicit MyDebug(const char * func);
    ~MyDebug();
    void outstring(QString out);

private:
    QString name;
    static int level;
};

#endif // MYDEBUG_H
