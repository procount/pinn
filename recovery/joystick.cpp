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

joystick::joystick(QObject *parent) :
    Kinput(parent)
{
    keyPressed=0;
}


joystick::~joystick()
{
}


int joystick::map_button(QVariant joy)
{
    //TRACE
    int result=0;
    if (joy.type() == QVariant::String)
    {
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


int joystick::convert_event2joy(struct js_event jse)
{
    //TRACE
    int result=0;

    //DBG2 ("Searching for Type " << jse.type << " Number "<<jse.number << " Value " << jse.value;)
    //qDebug() << "Searching for Type " << jse.type << " Number "<<jse.number << " Value " << jse.value;

    struct joymap_str *map = joymap;
    while ((map->string) && (!result))
    {
        int value=jse.value;
        if (jse.type==2)
        {
            value = sgn(value);
        }
        if ((map->type == (jse.type & ~JS_EVENT_INIT)) && ((map->value == value) || (!value)) && (map->number == jse.number))
        {
            result = map->id;
            //DBG2 << "Found "<< map->id << ":" <<map->string;
            //qDebug() << "Found "<< map->id << ":" <<map->string;
        }
        map++;
    }

    return (result);
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

    //TRACE
    int key=convert_event2joy(jse);
    keyPressed=1;
    emit joyPress(key,jse.value);
    if ((jse.value==0) && (jse.type==2))
    {
        if (key==1)
            emit joyPress(2,jse.value);
        if (key==3)
            emit joyPress(4,jse.value);
    }
}

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
        clearKeyPressed();
    }
}


const char * joystick::decode_joy( int code)
{
    //TRACE
    struct joymap_str *map = joymap;
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

    int step = scale_joystick(value);
    int sign = sgn(step);
    int negstep = step*sign*-1;
    value = negstep;

    inject_key(key,value);
}
