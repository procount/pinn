#ifndef MYDEBUG_H
#define MYDEBUG_H

#include <QDebug>

//Conditional GLOBAL flags
#define DO_DBG 0	//global debug enabler
#define DBG_FUNC 0	//Debug entry/exit of functions
#define DBG_OUT 0	//Debug  output strings
#define DBG_MSG 0   //Debug message boxes


#ifndef LOCAL_DO_DBG
#define LOCAL_DO_DBG DO_DBG
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


#if DO_DBG && DBG_LOCAL
  #define FILE_DO_DBG   LOCAL_DO_DBG
  #define FILE_DBG_FUNC LOCAL_DBG_FUNC
  #define FILE_DBG_OUT  LOCAL_DBG_OUT
  #define FILE_DBG_MSG  LOCAL_DBG_MSG
#else
  #define FILE_DO_DBG   DO_DBG
  #define FILE_DBG_FUNC DBG_FUNC
  #define FILE_DBG_OUT  DBG_OUT
  #define FILE_DBG_MSG  DBG_MSG
#endif


#if FILE_DO_DBG
  //macros when debugging is on
  #define MYDEBUG MyDebug dbg(__PRETTY_FUNCTION__);

  #if FILE_DBG_OUT
    #define DBG(x) dbg.outstring(x);
  #else
    #define DBG(x)
  #endif

  #if FILE_DBG_MSG
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
