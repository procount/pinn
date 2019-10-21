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

#ifndef INPUT_H
#define INPUT_H

#include <QThread>
#include <QMap>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QTimer>

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


const char * decode_key(struct keymap_str *map, int code);

class Kinput : public QThread
{
    Q_OBJECT
public:
    explicit Kinput(QObject *parent = 0);
    int hasKeyPressed() {return(keyPressed);}
    void clearKeyPressed() {keyPressed=0;}
    static void setWindow(const QString &wnd)  { _wnd = wnd; qDebug() << "setWindow "<<wnd;}
    static void setMenu(const QString &menu)   { _menu = menu; qDebug() << "setMenu "<<menu;}
    static void setGrabWindow(QObject * window) {_grab = window; }
    static QObject *  grabWindow() {return (_grab); }
    static QString getWindow() { return _wnd; }
    static QString getMenu() { return _menu; }
    void loadMap(QString filename, QString defName);

protected:
    int keyPressed;
    static QString _wnd;
    static QString _menu;
    int map_string(struct keymap_str *map, QString str);
    int map_key(QString cec);
    mapwnd_t _map;
    virtual int map_button(QVariant joy);
    void inject_key(int key,int value);
    void key_simulate(int key, int value);
    void mouse_simulate(int key, int value);

signals:

public slots:
    void mouse_repeat();
    void key_repeat();

private:
    int mouse_state[6];
    int mouse_input;
    int step;
    int count;
    static QObject * _grab;
    int currentKey;
    int keyState;
    QTimer keytimer;
    QTimer mousetimer;
};

#endif // INPUT_H
