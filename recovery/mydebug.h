#ifndef MYDEBUG_H
#define MYDEBUG_H

#include <QDebug>

//Conditional flags
#define DO_DBG 0	//global debug enabler
#define DBG_FUNC 1	//Debug entry/exit of functions
#define DBG_OUT 1	//Debug  output strings

#if DO_DBG
//macros when debugging is on
#define MYDEBUG MyDebug dbg(__PRETTY_FUNCTION__);
#define DBG(x) dbg.outstring(x);
#else
//macros when debugging is off
#define DBG(x) 
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
