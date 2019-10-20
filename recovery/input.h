#ifndef INPUT_H
#define INPUT_H

#include <QThread>
#include <QMap>
#include <QFile>
#include <QDebug>
#include "json.h"
#include "simulate.h"

#define MAXSTEPS 10
struct scale_t
{
    int threshold;
    int step;
};

extern struct scale_t scale_map[MAXSTEPS];
#define mouse_any       0x1000
#define mouse_left      0x1001
#define mouse_right     0x1002
#define mouse_up        0x1003
#define mouse_down      0x1004
#define mouse_lclick    0x1005

#define joy_any         0x2000
#define joy_left        0x2001
#define joy_right       0x2002
#define joy_up          0x2003
#define joy_down        0x2004

typedef QMap<int,int> mapkeys_t;
typedef QMap<QString, mapkeys_t> mapmenu_t;
typedef QMap<QString, mapmenu_t> mapwnd_t;

struct keymap_str {
    const char * string;
    int code;
};


extern keymap_str key_map[];

extern void inject_key(int key,int value);
const char * decode_key(struct keymap_str *map, int code);

class Kinput : public QThread
{
    Q_OBJECT
public:
    explicit Kinput(QObject *parent = 0);
    int hasKeyPressed() {return(keyPressed);}
    void clearKeyPressed() {keyPressed=0;}
    void setWindow(const QString &wnd)  { _wnd = wnd; }
    void setMenu(const QString &menu)   { _menu = menu; }
    QString getWindow() { return _wnd; }
    QString getMenu() { return _menu; }
    void loadMap(QString filename, QString defName);

protected:
    int keyPressed;
    QString _wnd;
    QString _menu;
    int map_string(struct keymap_str *map, QString str);
    int map_key(QString cec);
    mapwnd_t _map;
    virtual int map_button(QVariant joy);

signals:

public slots:

private:

};

#endif // INPUT_H
