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

#include "joystick.h"
#include "json.h"
#include <QApplication>
#include <QRect>
#include <QDesktopWidget>
#include <QMargins>

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
#include "mydebug.h"

#define WANTED 0

#define sgn(x) ((x < 0) ? -1 : (x > 0))


#define TEST 0
#define MAXINPUTS 40

//Mapping of joystick events to a pseudo "string" key
struct joymap_str joymap[MAXINPUTS] =
{
    //String,          ID, value, type, number
    {"LAnalogLeft",     1,  -1,  2,  0,  0},
    {"LAnalogRight",    2,   1,  2,  0,  0},
    {"LAnalogUp",       3,  -1,  2,  1,  0},
    {"LAnalogDown",     4,   1,  2,  1,  0},
    {"LT",              5,   1,  2,  2,  0},
    {"RAnalogLeft",     6,  -1,  2,  3,  0},
    {"RAnalogRight",    7,   1,  2,  3,  0},
    {"RAnalogUp",       8,  -1,  2,  4,  0},
    {"RAnalogDown",     9,   1,  2,  4,  0},
    {"RT",             10,   1,  2,  5,  0},
    {"D-padLeft",      11,  -1,  2,  6,  0},
    {"D-padRight",     12,   1,  2,  6,  0},
    {"D-padUp",        13,  -1,  2,  7,  0},
    {"D-padDown",      14,   1,  2,  7,  0},
    {"A",              15,   1,  1,  0,  0},
    {"B",              16,   1,  1,  1,  0},
    {"X",              17,   1,  1,  2,  0},
    {"Y",              18,   1,  1,  3,  0},
    {"LB",             19,   1,  1,  4,  0},
    {"RB",             20,   1,  1,  5,  0},
    {"Back",           21,   1,  1,  6,  0},
    {"Start",          22,   1,  1,  7,  0},
    {"Mode",           23,   1,  1,  8,  0},
    {"LAnalog_click",  24,   1,  1,  9,  0},
    {"RAnalog_click",  25,   1,  1,  10, 0},
    {"",               -1,   0,  0,  0,  0}
};

joystick::joystick(QObject *parent) :
    Kinput(parent)
{
    keyPressed=0;
    fd=-1;
}


joystick::~joystick()
{
}

int joystick::findJoystick()
{
    int fl;
    if ((fl = open("/dev/input/js0", O_RDONLY)) >= 0)
    {
        fd=fl;
        loadMap("joy_keys.json");
    }
    return(fd);
}

int joystick::map_button(QVariant joy)
{
    TRACE
    int result=0;
    if (joy.type() == QVariant::String)
    {
        QString str = joy.toString();
        struct joymap_str * map = joymap;
        while ((*map->string) && (!result))
        {
            if (map->string == str)
                result = map->id;
            map++;
        }
    }
    else
        result=joy.toInt();
    //DBG2 << joy << "==>" << result;
    return (result);
}


int joystick::convert_event2joy(struct js_event *jse)
{
    TRACE
    int result=0;

    //DBG2 ("Searching for Type " << jse.type << " Number "<<jse.number << " Value " << jse.value;)
    //qDebug() << "Searching for Type " << jse.type << " Number "<<jse.number << " Value " << jse.value;
#if 1 //WANTED
    if ((jse->type & JS_EVENT_INIT)==0)
    {
        QString dbgmsg;
        dbgmsg = "Joystick " + ((jse->type==1) ? QString("btn "):QString("axis")) + " #"+ QString::number(jse->number)+" Value: " + QString::number(jse->value);
        emit joyDebug(dbgmsg);
        //emit joyEvent(jse->type, jse->number, jse->value);
         //qDebug() << "Joy type: " <<jse->type<< " No: "<<jse->number<<" Value: " << jse->value;
    }
#endif
    struct joymap_str *map = joymap;
    while ((*map->string) && (!result))
    {
        int value=jse->value;
        if (jse->type==2)
        {
            value = sgn(value);
        }
        if ((map->type == (jse->type & ~JS_EVENT_INIT)) && ((map->value == value) || (!value)) && (map->number == jse->number))
        {
            result = map->id;
            if (map->deadzone>0)
            {
                if ((jse->value < map->deadzone) && (jse->value > -map->deadzone))
                    jse->value=0;
            }
            if (map->deadzone<0)
            {   //Triggers default to -32768
                if (jse->value < map->deadzone)
                    jse->value=0;
                else
                    jse->value=1;
            }

            //DBG2 << "Found "<< map->id << ":" <<map->string;
            //qDebug() << "Found "<< map->id << ":" <<map->string;
        }
        map++;
    }

    return (result);
}


void joystick::print_device_info()
{
    TRACE
    int axes=0, buttons=0;
    char name[128];
    if (fd != -1)
    {
        ioctl(fd, JSIOCGAXES, &axes);
        ioctl(fd, JSIOCGBUTTONS, &buttons);
        ioctl(fd, JSIOCGNAME(sizeof(name)), &name);

        qDebug() << name << "\n" << axes << " Axes, " << buttons << " buttons";
    }
}

QString joystick::getMapName()
{
    char name[128];
    int axes=0, buttons=0;
    TRACE

    QString filename;
    if (fd != -1)
    {
        ioctl(fd, JSIOCGAXES, &axes);
        ioctl(fd, JSIOCGBUTTONS, &buttons);
        ioctl(fd, JSIOCGNAME(sizeof(name)), &name);

        filename=QString(name)+"_"+QString::number(axes)+"A_"+QString::number(buttons)+"B.json";

        filename.replace(' ','_');
        filename.replace('(','_');
        filename.replace(')','_');
    }
    return(filename);
}

void joystick::loadMap(QString def_filename)
{
    TRACE
    QString filename = getMapName();
    //emit joyDebug(filename);
    Kinput::loadMap(filename, def_filename);

    //display_mapping();
}

int joystick::map_input(QVariant joy)
{
    TRACE
    int result=-1;
    if (joy.type() == QVariant::String)
    {
        QString str = joy.toString();
        struct joymap_str * map = joymap;
        while ((map->type) && (result == -1))
        {
            if (map->string == str)
            {
                result = map->id;
                qDebug() << "map_input found "<<str<< " at "<< result;
            }
            map++;
        }
    }
    else
        result=joy.toInt();
    return (result);
}

void joystick::parse_inputs(QVariantMap &map)
{
    int i;
    int numinputs=0;
    TRACE

    //qDebug()<<"Processing Inputs";

    for (i=0; i< MAXINPUTS; i++)
    {
        joymap[i].id=-1;
        joymap[i].value=-1;
        joymap[i].number=-1;
        joymap[i].type=0;
        joymap[i].deadzone=0;
        strcpy(joymap[i].string,"");
    }

    for(QVariantMap::const_iterator input = map.begin(); input != map.end(); ++input)
    {   //For each key
        QString str = input.key();
        int type=0;
        if (str.endsWith("_btn"))
            type=1;
        else if (str.endsWith("_axis"))
            type=2;

        int joy_code = map_input(str);  //Convert the Joy input name
        if ((type) && (numinputs < MAXINPUTS-1))
        {
            if (joy_code == -1)
            {
                joy_code = numinputs++;
            }
            const char *ca;
            QByteArray ba;
            int elems = input.value().toList().size();
            ba = str.toLatin1();
            ca = ba.data();
            strncpy (joymap[joy_code].string, ca,31);
            joymap[joy_code].id=joy_code;
            joymap[joy_code].type = type;
            if (elems>0)
                joymap[joy_code].number = input.value().toList().at(0).toInt();
            if (elems>1)
                joymap[joy_code].value = input.value().toList().at(1).toInt();
            if (elems>2)
                joymap[joy_code].deadzone = input.value().toList().at(2).toInt();

            //qDebug()<<"Mapped to "<< joymap[joy_code].number << " " << joymap[joy_code].value;
        }
    }

#if 0
    for (i=0; i< MAXINPUTS; i++)
    {
        qDebug() << joymap[i].id <<"\t"<<joymap[i].type <<"\t"<< joymap[i].value <<"\t"<< joymap[i].number <<"\t"<< joymap[i].string;
    }
#endif
}


void joystick::process_event(struct js_event jse)
{
    TRACE
    int key1=-1;
    int key2=-1;

    key1 = convert_event2joy(&jse);

    if ((jse.value==0) && (jse.type==2))
    {   //If a directional axis goes to zero, we should release both directions.
        jse.value=1;
        key1=convert_event2joy(&jse);
        jse.value=-1;
        key2=convert_event2joy(&jse);
        jse.value=0;
    }

    if ( !(jse.type & JS_EVENT_INIT))
    {
        if (jse.value)
            keyPressed=1;
        emit joyPress(key1,jse.value);
        if (key2!=-1)
            emit joyPress(key2,jse.value);
    }
}

void joystick::run()
{
    TRACE
    struct js_event jse;

    while (1)
    {
        if (fd==-1)
        {
            while (findJoystick() ==-1)
                sleep(1);
        }
        while (read(fd, &jse, sizeof(struct js_event)) == sizeof(struct js_event))
                process_event(jse);
        qDebug()<<"Error reading joystick";
        close(fd);
        fd=-1;
        reset();
    }
}

const char * joystick::decode_joy( int code)
{
    TRACE
    struct joymap_str *map = joymap;
    while (*map->string)
    {
        if (map->id == code)
            return(map->string);
        map++;
    }
    return("Unknown");
}


//Only required for analog joysticks to give some sort of proportional mouse speed
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

void joystick::display_mapping()
{
    for(mapwnd_t::const_iterator wnd =_map.begin(); wnd != _map.end(); ++wnd)
    {
        QString wname = wnd.key();
        qDebug()<<wname;
        mapmenu_t menu = wnd.value();
        for(mapmenu_t::const_iterator m =menu.begin(); m != menu.end(); ++m)
        {
            QString mname = m.key();
            qDebug()<<mname;
            mapkeys_t keys = m.value();
            for(mapkeys_t::const_iterator k =keys.begin(); k != keys.end(); ++k)
            {
                int joy,key;
                joy = k.key();
                key = k.value();
                qDebug() << "Joy="<<joy<<"("<<decode_joy(joy)<<")" <<"Key="<<key<<"("<<decode_key(key_map, key)<<")";
            }
        }

    }
}

void joystick::process_joy(int joy_code, int value)
{
    TRACE
    int found=0;
    int key=-1;
    QString menu;
    QString wnd;

    int done=0;
    wnd=_wnd;
    do
    {
        if (_map.contains(wnd))
        {
            mapmenu_t m = _map.value(wnd);

            menu = _menu;
            do {
                if (m.contains(menu))
                {
                    mapkeys_t k = m.value(menu);
                    if (k.contains(joy_code))
                    {
                        key = k.value(joy_code);
                        qDebug() << "found joy " << joy_code << " ("<< decode_joy(joy_code) <<  ") in "<<wnd<<" : "<<menu<<" as "<<key<< " ("<<decode_key(key_map,key)<<")";
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

    if (key&joy_any)
    {
        value /= 3000;
    }

/*    if (key & mouse_any)
    {
        if (key != mouse_lclick)
        {
            if (value != 0)
                value=-100;
        }
    }*/
    inject_key(key,value);
}
