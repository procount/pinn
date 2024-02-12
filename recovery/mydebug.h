#ifndef MYDEBUG_H
#define MYDEBUG_H

#include <QDebug>

//Conditional GLOBAL flags
#define GLOBAL_ENABLE_DEBUG 1   //global debug enabler, enables MyDebug class

#define GLOBAL_DBG_ON       0   //Use the global data everywhere (overrides ths other globals. sets them to 0)

#if GLOBAL_DBG_ON
  #define GLOBAL_DBG_FUNC   1	//global Debug entry/exit of functions
  #define GLOBAL_DBG_OUT    1	//global Debug  output strings
  #define GLOBAL_DBG_MSG    1   //global Debug message boxes
#else
  #define GLOBAL_DBG_FUNC   0	//global Debug entry/exit of functions
  #define GLOBAL_DBG_OUT    0	//global Debug  output strings
  #define GLOBAL_DBG_MSG    0   //global Debug message boxes
#endif

#if LOCAL_DBG_ON
  #ifdef LOCAL_DBG_FUNC
    #define DBG_FUNC  LOCAL_DBG_FUNC
  #else
    #define DBG_FUNC    GLOBAL_DBG_FUNC
  #endif

  #ifdef LOCAL_DBG_OUT
    #define DBG_OUT   LOCAL_DBG_OUT
  #else
    #define DBG_OUT   GLOBAL_DBG_OUT
  #endif

  #ifdef LOCAL_DBG_MSG
    #define DBG_MSG   LOCAL_DBG_MSG
  #else
    #define DBG_MSG   GLOBAL_DBG_MSG
  #endif
#else
  #define DBG_FUNC    GLOBAL_DBG_FUNC
  #define DBG_OUT     GLOBAL_DBG_OUT
  #define DBG_MSG     GLOBAL_DBG_MSG
#endif


#if GLOBAL_ENABLE_DEBUG

 #define MYDEBUG(x) MyDebug dbg(__PRETTY_FUNCTION__, x);
 #define TRACE MYDEBUG(DBG_FUNC);
 #define TRACEIF(x) MYDEBUG(x);

 #define PARAMS if (dbg._display) qDebug() << dbg.header() << (const char *)"Params:"
 #define INDENT(n) dbg._local_level+=n
 #define OUTDENT(n) dbg._local_level-=n

 #if DBG_OUT
  #define DBG2 if (dbg._display==1) qDebug() << dbg.header() << ""
  #define DBG(x) DBG2 << x
 #else
  #define DBG2 QNoDebug()
  #define DBG(x)
 #endif

 #if DBG_MSG
  #include <QMessageBox>
  #define MSG() QMessageBox::warning(NULL, QString(__FILE__),QString(__PRETTY_FUNCTION__)+":#"+QString::number(__LINE__), QMessageBox::Close)
 #else
  #define MSG()
 #endif

class MyDebug
{
public:
    MyDebug(const char * funcname, int display=0);
    ~MyDebug();
    int _local_level;
    int _display;

    const char * header();
private:
    QString _name;
    static int _level;
};

#else //GLOBAL_ENABLE_DEBUG

 #define MYDEBUG
 #define TRACE
 #define DBG2 QNoDebug()
 #define PARAMS
 #define INDENT(n)
 #define OUTDENT(n)
 #define DBG(x)
 #define MSG(x)

#endif // GLOBAL_ENABLE_DEBUG
#endif // MYDEBUG_H

