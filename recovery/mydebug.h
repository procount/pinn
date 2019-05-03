#ifndef MYDEBUG_H
#define MYDEBUG_H

#include <QDebug>

//Conditional GLOBAL flags
#define DO_DBG 0	//global debug enabler
#define DBG_FUNC 0	//global Debug entry/exit of functions
#define DBG_OUT 0	//global Debug  output strings
#define DBG_MSG 0   //global Debug message boxes


#ifndef LOCAL_DO_DBG        //If the local debug does not exist
#define LOCAL_DO_DBG DO_DBG //Use the global version
#endif

#ifndef LOCAL_DBG_FUNC
#define LOCAL_DBG_FUNC DBG_FUNC
#endif

#ifndef LOCAL_DBG_OUT
#define LOCAL_DBG_OUT DBG_OUT
#endif

#ifndef LOCAL_DBG_MSG
#define LOCAL_DBG_MSG DBG_MSG
#endif


#if DO_DBG && DBG_LOCAL                 //Use local debug where it is given
  #define FILE_DO_DBG   LOCAL_DO_DBG
  #define FILE_DBG_FUNC LOCAL_DBG_FUNC
  #define FILE_DBG_OUT  LOCAL_DBG_OUT
  #define FILE_DBG_MSG  LOCAL_DBG_MSG
#else                                   //Else use the global file
  #define FILE_DO_DBG   DO_DBG
  #define FILE_DBG_FUNC DBG_FUNC
  #define FILE_DBG_OUT  DBG_OUT
  #define FILE_DBG_MSG  DBG_MSG
#endif


#if FILE_DO_DBG

 #define MYDEBUG MyDebug dbg(__PRETTY_FUNCTION__, FILE_DBG_FUNC)
 #define TRACE MYDEBUG;
 #define PARAMS qDebug() << dbg.header() << (const char *)"Params:"
 #define INDENT(n) dbg._local_level+=n
 #define OUTDENT(n) dbg._local_level-=n

 #if FILE_DBG_OUT
  #define DBG2 qDebug() << dbg.header() <<""
  #define DBG(x) DBG2 << x
 #else
  #define DBG2
  #define DBG
 #endif

 #if FILE_DBG_MSG
  #define MSG() QMessageBox::warning(NULL, QString(__FILE__),QString(__PRETTY_FUNCTION__)+":#"+QString::number(__LINE__), QMessageBox::Close)
 #else
  #define MSG()
 #endif

#else

 #define MYDEBUG
 #define TRACE
 #define DBG2
 #define PARAMS
 #define INDENT(n)
 #define OUTDENT(n)
 #define DBG(x)
 #define MSG(x)

#endif

class MyDebug
{
public:
    MyDebug(const char * funcname, int display=0);
    ~MyDebug();
    int _local_level;

    const char * header();
private:
    QString _name;
    static int _level;
    int _display;
};


#endif // MYDEBUG_H
