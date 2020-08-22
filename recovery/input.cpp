/*
 * Copyright (c) 2019, @procount
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "input.h"
#include <QDebug>

#include <linux/input.h>
#include <linux/uinput.h>

// #include <QWSServer>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QCoreApplication>

QString Kinput::_wnd="";
QString Kinput::_menu="";
QObject * Kinput::_grab;

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
    _grab=NULL;
    keyPressed=0;
    keyState=0;
    currentKey=0;
    mouse_input=0;
    for (int i=0; i<6; i++)
        mouse_state[i]=0;

    mousetimer.setSingleShot(true);
    connect (&mousetimer, SIGNAL(timeout()), this, SLOT(mouse_repeat()) );
    keytimer.setSingleShot(true);
    connect(&keytimer, SIGNAL(timeout()), this, SLOT(key_repeat()));
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

int Kinput::map_button(QVariant joy)
{
    Q_UNUSED(joy);
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
        qDebug() << "Loading Mappings mappings from " << fname;
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
                        int joy_code = map_button(iKey.value());   //Convert the CEC code (string or int)
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
void Kinput::inject_key(int key, int value)
{
    if (key & mouse_any)
        mouse_simulate(key,value);
    else
        key_simulate(key,value);

}

void Kinput::mouse_simulate(int key, int value)
{
//    qDebug() << "Inject Mouse Code: "<<key<<" Value: " <<value;

    mouse_state[key & 0x07]= (value ? 1 : 0);

    if (value)
    {
        if (!mouse_input)
        {
            mouse_input=1;
            step=1;
            count=0;
            mouse_repeat(); //Do it now
        }
    }
    else
    {
        int down=0;
        for (int i=0; i<6; i++)
        {
            if (mouse_state[i])
                down++;
        }
        if (!down)
            mouse_input=0;
    }


//    qDebug()<<"Mouse input=" << mouse_input;
//    for (int i=0; i<6; i++)
//        qDebug() << mouse_state[i];

}

void Kinput::mouse_repeat()
{
    extern simulate * sim;
    int down=0;
    QPoint p = QCursor::pos();
    QSize screen = QApplication::desktop()->screenGeometry(-1).size();

    for (int i=0; i<6; i++)
    {

        switch (i | mouse_any)
        {
        /* MOUSE SIMULATION */
        case mouse_left:
            if (mouse_state[i])
            {
                //qDebug()<<"MouseLeft";
                down++;
                if (p.x()>0)
                    sim->inject(EV_REL, REL_X, -step);
                else
                    mouse_state[i]=0;
            }
            break;
        case mouse_right:
            if (mouse_state[i])
            {
                //qDebug()<<"MouseRight";
                down++;
                //p.rx()+=10;
                //QCursor::setPos(p);
                if (p.x()<screen.width())
                    sim->inject(EV_REL, REL_X, step);
                else
                    mouse_state[i]=0;
            }
            break;
        case mouse_up:
            if (mouse_state[i])
            {
                //qDebug()<<"MouseUp";
                down++;
                //p.ry()-=10;
                //QCursor::setPos(p);
                if (p.y()>0)
                    sim->inject(EV_REL, REL_Y, -step);
                else
                    mouse_state[i]=0;
            }
            break;
        case mouse_down:
            if (mouse_state[i])
            {
                //qDebug()<<"MouseDown";
                down++;
                //p.ry()+=10;
                //QCursor::setPos(p);
                if (p.y()<screen.height())
                   sim->inject(EV_REL, REL_Y, step);
                else
                    mouse_state[i]=0;
            }
            break;
        case mouse_lclick:
            if (mouse_state[i])
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
        }
    }
    sim->inject(EV_SYN, SYN_REPORT, 0);
    if (down==0) {
        count=0;
        step=1;
        mouse_input=0;
    }
    else {
        count++;
        if (count>5)
            step=5;
        if (count>25)
            step=8;
        if (count>50)
            step=10;
//        QTimer::singleShot(40, this, SLOT(mouse_repeat()));
        mousetimer.start( 40 );

    }
}

#if 0
void Kinput::key_simulate(int key, int value)
{
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    //QPoint p = QCursor::pos();
//    extern simulate * sim;

    qDebug() << "Inject Key Code: "<<key<<" Value: " <<value;

    if (_grab) {
        enum QEvent::Type type = (value ? QEvent::KeyPress : QEvent::KeyRelease);
        QKeyEvent *event=new QKeyEvent(type, key, modifiers);
        QCoreApplication::sendEvent(_grab, event);
    }
    else
    {
        // key press
        //sim->inject(EV_KEY, key, 1);
        //sim->inject(EV_SYN, SYN_REPORT, 0);
        QWSServer::sendKeyEvent(0, key, modifiers, true, false);
        // key release
        //sim->inject(EV_KEY, key, 0);
        //sim->inject(EV_SYN, SYN_REPORT, 0);
        QWSServer::sendKeyEvent(0, key, modifiers, false, false);
    }
}
#endif

void Kinput::key_simulate(int key, int value)
{
    if (value)
    {
        if ((keyState==0) && currentKey==0)
        {
            keyState=1;
            currentKey=key;
            key_repeat(); //Do it now
        }
    }
    else
    {
        keyState=0;
        keytimer.start( 20 );
    }
}

void Kinput::key_repeat()
{
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;

    if (keyState)
    {
        if (_grab) {
            QKeyEvent *event=new QKeyEvent(QEvent::KeyPress, currentKey, modifiers);
            QCoreApplication::sendEvent(_grab, event);
        }
        else
        {
            // key press
            //sim->inject(EV_KEY, key, 1);
            //sim->inject(EV_SYN, SYN_REPORT, 0);
            // QWSServer::sendKeyEvent(0, currentKey, modifiers, true, false);
        }
        //QTimer::singleShot( (keyState==1) ? 400 : 100, this, SLOT(key_repeat()));
        keytimer.start( (keyState==1) ? 400:100);
        keyState=2;
    }
    else
    {
        if (currentKey)
        {
            //send release event
            if (_grab) {
                QKeyEvent *event=new QKeyEvent( QEvent::KeyRelease, currentKey, modifiers);
                QCoreApplication::sendEvent(_grab, event);
            }
            else
            {
                // key release
                //sim->inject(EV_KEY, key, 0);
                //sim->inject(EV_SYN, SYN_REPORT, 0);
                // QWSServer::sendKeyEvent(0, currentKey, modifiers, false, false);
            }
            currentKey=0;
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




