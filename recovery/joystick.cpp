#include "joystick.h"
#include "json.h"
#include "ceclistener.h"

#include <QApplication>
#include <QRect>
#include <QDesktopWidget>
#include <QMargins>
#include <QDebug>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <linux/joystick.h>

#define DBG_LOCAL 0
#define LOCAL_DO_DBG 0
#define LOCAL_DBG_FUNC 0
#define LOCAL_DBG_OUT 0
#define LOCAL_DBG_MSG 0


#define sgn(x) ((x < 0) ? -1 : (x > 0))

struct scale_t
{
    int threshold;
    int step;
};

#define MAXSTEPS 10
struct scale_t scale_map[MAXSTEPS] = {-1,-1};

#define TEST 0

//Mapping of joystick events to a pseudo "string" key
struct joymap_str joymap[] =
{
    //String,          ID, value, type, number
    {"LAnalogLeft",     1,  -1,  2,  0},
    {"LAnalogRight",    2,   1,  2,  0},
    {"LAnalogUp",       3,  -1,  2,  1},
    {"LAnalogDown",     4,   1,  2,  1},
    {"LT",              5,   1,  2,  2},
    {"RAnalogLeft",     6,  -1,  2,  3},
    {"RAnalogRight",    7,   1,  2,  3},
    {"RAnalogUp",       8,  -1,  2,  4},
    {"RAnalogDown",     9,   1,  2,  4},
    {"RT",             10,   1,  2,  5},
    {"D-padLeft",      11,  -1,  2,  6},
    {"D-padRight",     12,   1,  2,  6},
    {"D-padUp",        13,  -1,  2,  7},
    {"D-padDown",      14,   1,  2,  7},
    {"A",              15,   1,  1,  0},
    {"B",              16,   1,  1,  1},
    {"X",              17,   1,  1,  2},
    {"Y",              18,   1,  1,  3},
    {"LB",             19,   1,  1,  4},
    {"RB",             20,   1,  1,  5},
    {"Back",           21,   1,  1,  6},
    {"Start",          22,   1,  1,  7},
    {"Mode",           23,   1,  1,  8},
    {"LAnalog_click",  24,   1,  1,  9},
    {"RAnalog_click ", 25,   1,  1,  10}
};

int joystick::keyPressed = 0;

joystick::joystick(QObject *parent) :
    QThread(parent)
{
    keyPressed=0;
}


joystick::~joystick()
{
}


int joystick::map_joy(QVariant joy)
{
    //TRACE
    int result=0;
    if (joy.type() == QVariant::String)
    {
        //result = map_joy(cec_map, cec.toString());
        QString str = joy.toString();
        struct joymap_str * map = joymap;
        while ((map->string) && (!result))
        {
            if (map->string == str)
                result = map->id;
            map++;
        }
    }
    else
        result=joy.toInt();
    return (result);
}


int joystick::map_key(QString key)
{
    //TRACE
    struct keymap_str *map = key_map;
    while (map->string)
    {
        if (map->string == key)
            return(map->code);
        map++;
    }
    return(0);
}

int joystick::convert_event2joy(struct js_event jse)
{
    //TRACE
    int result=0;

    //DBG2 ("Searching for Type " << jse.type << " Number "<<jse.number << " Value " << jse.value;)
    qDebug() << "Searching for Type " << jse.type << " Number "<<jse.number << " Value " << jse.value;

    struct joymap_str *map = joymap;
    while ((map->string) && (!result))
    {
        int value=jse.value;
        if (jse.type==2)
        {
            value = sgn(value);
        }
        if ((map->type == (jse.type & ~JS_EVENT_INIT)) && (map->value == value) && (map->number == jse.number))
        {
            result = map->id;
            //DBG2 << "Found "<< map->id << ":" <<map->string;
            qDebug() << "Found "<< map->id << ":" <<map->string;
        }
        map++;
    }

    return (result);
}

#if TEST
struct js_event find_joy_event(QString name)
{
    TRACE
    int result=0;
    struct js_event jse;


    struct joymap_str *map = joymap;
    while ((map->string) && (!result))
    {
        if (map->string == name)
        {
            jse.number = map->number;
            jse.type   = map->type;
            jse.value  = map->value;
            result=1;
        }
        map++;
    }

    return (jse);
}
#endif

void joystick::loadJOYmap(QString filename)
{
    //TRACE
    QString fname = filename;
    if (!QFile::exists(fname))
        fname=":/joy_keys.json"; //Use default mapping

    if (QFile::exists(fname))
    {
        qDebug() << "Loading Joystick mappings from " << fname;
        _JOYmap.clear();

        //Get the map of windows (& calibration)
        QVariantMap mapWnd = Json::loadFromFile(fname).toMap();

        for(QVariantMap::const_iterator iWnd = mapWnd.begin(); iWnd != mapWnd.end(); ++iWnd)
        {   //For each window, get the map of menus
            QVariantMap mapMenu = iWnd.value().toMap();

            if (iWnd.key() == "calibration")
            {
                mapkeys_t k; //My own map of keys
                QVariantList cal_list = iWnd.value().toList();
                int i=0;
                foreach (QVariant v, cal_list)
                {
                    QVariantMap  step = v.toMap();
                    if (i<MAXSTEPS-1)
                    {
                        scale_map[i].threshold=step.value("threshold").toInt();
                        scale_map[i].step=step.value("step").toInt();
                        //DBG2 << "Step "<<i<< " threshold: "<<scale_map[i].threshold<<" step: "<<scale_map[i].step;
                        i++;
                        //Mark end of table
                        scale_map[i].threshold=-1;
                        scale_map[i].step=-1;
                    }
                }
            }

            else
            {
                mapmenu_t m; //My own map of menus
                for(QVariantMap::const_iterator iMenu = mapMenu.begin(); iMenu != mapMenu.end(); ++iMenu)
                {   //For each menu, get the map of keys
                    QVariantMap mapKeys = iMenu.value().toMap();

                    mapkeys_t k; //My own map of keys
                    for(QVariantMap::const_iterator iKey = mapKeys.begin(); iKey != mapKeys.end(); ++iKey)
                    {   //For each key
                        int joy_code = map_joy(iKey.value());   //Convert the CEC code (string or int)
                        int key_code = map_key(iKey.key());     //Convert the KEY code to press (string)
                        if (joy_code !=-1)                      //Use CEC code of -1 to ignore that key
                            k[joy_code] = key_code;             //Map the CEC code to the key to be pressed
                    }
                    //Add key mapping to my menu
                    m[iMenu.key()] = k;
                }
                //Add my menu to my window map
                _JOYmap[iWnd.key()] = m;
            }
        }
    }
}



void joystick::print_device_info()
{
    //TRACE
    int axes=0, buttons=0;
    char name[128];

    ioctl(fd, JSIOCGAXES, &axes);
    ioctl(fd, JSIOCGBUTTONS, &buttons);
    ioctl(fd, JSIOCGNAME(sizeof(name)), &name);

    qDebug() << name << "\n" << axes << " Axes, " << buttons << " buttons";
}

void joystick::process_event(struct js_event jse)
{

/*
 * 	struct js_event {
        __u32 time;     // event timestamp in milliseconds
        __s16 value;    // value
        __u8 type;      // event type
        __u8 number;    // axis/button number
    };

The possible values of ``type'' are

    #define JS_EVENT_BUTTON         0x01    // button pressed/released
    #define JS_EVENT_AXIS           0x02    // joystick moved
    #define JS_EVENT_INIT           0x80    // initial state of device

The values of ``number'' correspond to the axis or button that
generated the event. Note that they carry separate numeration (that
is, you have both an axis 0 and a button 0). Generally,

            number
    1st Axis X	0
    1st Axis Y	1
    2nd Axis X	2
    2nd Axis Y	3
    ...and so on

For an axis, ``value'' is a signed integer between -32767 and +32767
representing the position of the joystick along that axis. If you
don't read a 0 when the joystick is `dead', or if it doesn't span the
full range, you should recalibrate it (with, for example, jscal).

For a button, ``value'' for a press button event is 1 and for a release
button event is 0.

Implementation:

    if ((js_event.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON) {
        if (js_event.value)
            buttons_state |= (1 << js_event.number);
        else
            buttons_state &= ~(1 << js_event.number);
    }


 */
    //TRACE
    int key=convert_event2joy(jse);
    emit joyPress(key,jse.value);
}

#if TEST
void joystick::run()
{
    //TRACE
    int i;
    struct js_event jse;

    while (1)
    {
        sleep (30);

        for (i=0; i<3; i++)
        {
            jse = find_joy_event("RAnalogLeft");
            jse.value *= scale_map[i].threshold;
            process_event(jse);
            sleep(1);
        }
        sleep (5);

        for (i=0; i<3; i++)
        {
            jse = find_joy_event("RAnalogUp");
            jse.value *= scale_map[i].threshold;
            process_event(jse);
            sleep(1);
        }
        sleep (5);

        for (i=0; i<3; i++)
        {
            jse = find_joy_event("RAnalogRight");
            jse.value *= scale_map[i].threshold;
            process_event(jse);
            sleep(1);
        }
        sleep (5);

        for (i=0; i<3; i++)
        {
            jse = find_joy_event("RAnalogDown");
            jse.value *= scale_map[i].threshold;
            process_event(jse);
            sleep(1);
        }
        sleep (5);
    }
}
#else
void joystick::run()
{
    //TRACE

    struct js_event jse;

    if ((fd = open("/dev/input/js0", O_RDONLY)) < 0)
    {
        qDebug() << "No Joystick found";
    }
    else
    {
        print_device_info();

        while (1)
        {
            if (read(fd, &jse, sizeof(struct js_event)) != sizeof(struct js_event))
                //qDebug() << "error reading joystick";
                ;
            else
                process_event(jse);
        }
    }
}
#endif


const char * decode_joy(struct joymap_str *map, int code)
{
    //TRACE
    while (map->string)
    {
        if (map->id == code)
            return(map->string);
        map++;
    }
    return("Unknown");
}

int scale_joystick(int value)
{
    int step=0;
    int i;
    int sign = sgn(value);
    value *= sign;
    for (i=0; i<MAXSTEPS; i++)
    {
        if (scale_map[i].threshold == -1)
        {
            break;
        }
        else
        {
            if (value >= scale_map[i].threshold)
                step = scale_map[i].step;
         }
    }
    step *= sign;
    return(step);
}

void joystick::process_joy(int joy_code, int value)
{
    //TRACE
    int found=0;
    int key=-1;
    QString menu;
    QString wnd;

    int done=0;
    wnd=_wnd;
    do
    {
        if (_JOYmap.contains(wnd))
        {
            mapmenu_t m = _JOYmap.value(wnd);

            menu = _menu;
            do {
                if (m.contains(menu))
                {
                    mapkeys_t k = m.value(menu);
                    if (k.contains(joy_code))
                    {
                        key = k.value(joy_code);
                        qDebug() << "found joy " << joy_code << " ("<< decode_joy(joymap, joy_code) <<  ") in "<<wnd<<" : "<<menu<<" as "<<key<< " ("<<decode_key(key_map,key)<<")";
                        found = 1;
                        done = 1;
                    }
                }
                if (!found)
                {
                    done=0;
                    if (menu !="any")
                        menu="any";
                    else
                        done=1;
                }
            } while (!done);
        }
        if (!found)
        {
            done=0;
            if (wnd != "any")
                wnd="any";
            else
                done=1;
        }
    } while (!done);

    if (key == -1)
        return;

    int step = scale_joystick(value);
    int sign = sgn(step);
    int negstep = step*sign*-1;

    QRect s = QApplication::desktop()->screenGeometry();
    QPoint p = QCursor::pos();

    switch (key)
    {
    /* JOYSTICK SCALED SIMULATION */
    case joy_left:
        //DBG2 <<"joy_left";
        p.rx()+=step;
        if (s.contains(p,true))
        {
            QCursor::setPos(p);
        }
        break;
    case joy_right:
        //DBG2 <<"joy_right";
        p.rx()+=step;
        if (s.contains(p,true))
        {
            QCursor::setPos(p);
        }
        break;
    case joy_up:
        //DBG2 <<"joy_up";
        p.ry()+=step;
        if (s.contains(p,true))
        {
            QCursor::setPos(p);
        }
        break;
    case joy_down:
        //DBG2 <<"joy_down";
        p.ry()+=step;
        if (s.contains(p,true))
        {
            QCursor::setPos(p);
        }
        break;
    default:
        //DBG2 <<"Inject_key";
        inject_key(key);
        break;
    }
}
