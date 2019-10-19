#include "input.h"

#include <linux/input.h>
#include <linux/uinput.h>

#include <QWSServer>

struct scale_t scale_map[MAXSTEPS] = {-1,-1};

keymap_str key_map[]={
    {"Key_Escape",      Qt::Key_Escape},
    {"Key_Space",       Qt::Key_Space},
    {"Key_Enter",       Qt::Key_Enter},
    {"Key_PageUp",      Qt::Key_PageUp},
    {"Key_PageDown",    Qt::Key_PageDown},
    {"Key_Up",          Qt::Key_Up},
    {"Key_Down",        Qt::Key_Down},
    {"Key_Left",        Qt::Key_Left},
    {"Key_Right",       Qt::Key_Right},
    {"Key_A",           Qt::Key_A},
    {"Key_B",           Qt::Key_B},
    {"Key_C",           Qt::Key_C},
    {"Key_D",           Qt::Key_D},
    {"Key_E",           Qt::Key_E},
    {"Key_F",           Qt::Key_F},
    {"Key_G",           Qt::Key_G},
    {"Key_H",           Qt::Key_H},
    {"Key_I",           Qt::Key_I},
    {"Key_J",           Qt::Key_J},
    {"Key_K",           Qt::Key_K},
    {"Key_L",           Qt::Key_L},
    {"Key_M",           Qt::Key_M},
    {"Key_N",           Qt::Key_N},
    {"Key_O",           Qt::Key_O},
    {"Key_P",           Qt::Key_P},
    {"Key_Q",           Qt::Key_Q},
    {"Key_R",           Qt::Key_R},
    {"Key_S",           Qt::Key_S},
    {"Key_T",           Qt::Key_T},
    {"Key_U",           Qt::Key_U},
    {"Key_V",           Qt::Key_V},
    {"Key_W",           Qt::Key_W},
    {"Key_X",           Qt::Key_X},
    {"Key_Y",           Qt::Key_Y},
    {"Key_Z",           Qt::Key_Z},
    {"mouse_left",      mouse_left},
    {"mouse_right",     mouse_right},
    {"mouse_up",        mouse_up},
    {"mouse_down",      mouse_down},
    {"mouse_lclick",    mouse_lclick},
    {"joy_left",        joy_left},
    {"joy_right",       joy_right},
    {"joy_up",          joy_up},
    {"joy_down",        joy_down},
    {"joy_lclick",      mouse_lclick},
    {NULL,                0}
};

Kinput::Kinput(QObject *parent) :
    QThread(parent)
{
    keyPressed=0;
}

int Kinput::map_string(struct keymap_str *map, QString str)
{
    while (map->string)
    {
        if (map->string == str)
            return(map->code);
        map++;
    }
    return(0);
}

int Kinput::map_key(QString key)
{
    return ( map_string(key_map, key) );
}

int Kinput::map_joy(QVariant joy)
{
    return(0);
}

void Kinput::loadMap(QString filename, QString defName)
{
    //TRACE
    QString fname = filename;
    if (!QFile::exists(fname))
        fname=defName; //Use default mapping

    if (QFile::exists(fname))
    {
        qDebug() << "Loading Joystick mappings from " << fname;
        _map.clear();

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
                _map[iWnd.key()] = m;
            }
        }
    }
}



const char * decode_key(struct keymap_str *map, int code)
{
    while (map->string)
    {
        if (map->code == code)
            return(map->string);
        map++;
    }
    return("Unknown");
}

void inject_key(int key)
{
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    //QPoint p = QCursor::pos();
    extern simulate * sim;

    switch (key)
    {
    /* MOUSE SIMULATION */
    case mouse_left:
        //p.rx()-=10;
        //QCursor::setPos(p);
        sim->inject(EV_REL, REL_X, -10);
        sim->inject(EV_SYN, SYN_REPORT, 0);
        break;
    case mouse_right:
        //p.rx()+=10;
        //QCursor::setPos(p);
        sim->inject(EV_REL, REL_X, 10);
        sim->inject(EV_SYN, SYN_REPORT, 0);
        break;
    case mouse_up:
        //p.ry()-=10;
        //QCursor::setPos(p);
        sim->inject(EV_REL, REL_Y, -10);
        sim->inject(EV_SYN, SYN_REPORT, 0);
        break;
    case mouse_down:
        //p.ry()+=10;
        //QCursor::setPos(p);
        sim->inject(EV_REL, REL_Y, 10);
        sim->inject(EV_SYN, SYN_REPORT, 0);
        break;
    case mouse_lclick:
        { //Click!
/*
 *             QWidget* widget = dynamic_cast<QWidget*>(QApplication::widgetAt(QCursor::pos()));
            if (widget)
            {
                QPoint pos = QCursor::pos();
                QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress,widget->mapFromGlobal(pos), Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
                QCoreApplication::sendEvent(widget,event);
                QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonRelease,widget->mapFromGlobal(pos), Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
                QCoreApplication::sendEvent(widget,event1);
                qApp->processEvents();
            }
*/
            sim->inject(EV_KEY, BTN_LEFT, 1);
            sim->inject(EV_SYN, SYN_REPORT, 0);
            sim->inject(EV_KEY, BTN_LEFT, 0);
            sim->inject(EV_SYN, SYN_REPORT, 0);
        }
        break;
    default:
        // key press
        //sim->inject(EV_KEY, key, 1);
        //sim->inject(EV_SYN, SYN_REPORT, 0);
        QWSServer::sendKeyEvent(0, key, modifiers, true, false);
        // key release
        //sim->inject(EV_KEY, key, 0);
        //sim->inject(EV_SYN, SYN_REPORT, 0);
        QWSServer::sendKeyEvent(0, key, modifiers, false, false);
        break;
    }
}


