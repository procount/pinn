#ifndef MYDEBUG_H
#define MYDEBUG_H

#include <QDebug>

//Conditional flags
#define DO_DBG 0	//global debug enabler
#define DBG_FUNC 0	//Debug entry/exit of functions
#define DBG_OUT 0	//Debug  output strings
#define DBG_MSG 0   //Debug message boxes

#if DO_DBG
//macros when debugging is on
#define MYDEBUG MyDebug dbg(__PRETTY_FUNCTION__);
#define DBG(x) dbg.outstring(x);
#if DBG_MSG
#define MSG() QMessageBox::warning(NULL, QString(__FILE__),QString(__PRETTY_FUNCTION__)+":#"+QString::number(__LINE__), QMessageBox::Close)
#else
#define MSG()
#endif

#else
//macros when debugging is off
#define DBG(x)
#define MYDEBUG
#define MSG()
#endif

#define TRACEFN MYDEBUG
#define TRACE TRACEFN

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
