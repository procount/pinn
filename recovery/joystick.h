#ifndef JOYSTICK_H
#define JOYSTICK_H

//#include <QObject>
#include <QThread>
#include <QFile>
//#include <QWaitCondition>
//#include <QMutex>
//#include <stdint.h>
#include <QVariantMap>

typedef QMap<int,int> mapkeys_t;
typedef QMap<QString, mapkeys_t> mapmenu_t;
typedef QMap<QString, mapmenu_t> mapwnd_t;


struct joymap_str {
    const char * string;
    int id;                 // Pseudo key
    int value;              // value
    unsigned char type;     // event type
    unsigned char number;   // axis/button number
};


int scale_joystick(int value);

class joystick : public QThread
{
    Q_OBJECT
public:
    explicit joystick(QObject *parent = 0);
    virtual ~joystick();

    void print_device_info();
    void process_event(struct js_event jse);
    static int hasKeyPressed() {return(keyPressed);}
    static void clearKeyPressed() {keyPressed=0;}
    void setWindow(const QString &wnd)  { _wnd = wnd; }
    void setMenu(const QString &menu)   { _menu = menu; }
    int map_joy(QVariant joy);
    int convert_event2joy(struct js_event jse);
    void loadJOYmap(QString);
    int map_key(QString key);
    void process_joy(int joy_code, int value);

protected:
    virtual void run();

    int fd;
    static int keyPressed;
    mapwnd_t _JOYmap;
    QString _wnd;
    QString _menu;

signals:
    void joyPress(int key, int value);

public slots:


};

#endif // JOYSTICK_H
