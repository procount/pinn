#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "input.h"

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

class joystick : public Kinput
{
    Q_OBJECT
public:
    explicit joystick(QObject *parent = 0);
    virtual ~joystick();

    void print_device_info();
    void process_event(struct js_event jse);
    int map_joy(QVariant joy);
    int convert_event2joy(struct js_event jse);
    void loadMap(QString filename) { Kinput::loadMap(filename, ":/joy_keys.json"); }
    void process_joy(int joy_code, int value);
    const char * decode_joy(int code);

protected:
    virtual void run();

    int fd;

signals:
    void joyPress(int key, int value);

public slots:


};

#endif // JOYSTICK_H
