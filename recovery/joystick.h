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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "input.h"

#include <QThread>
#include <QFile>
#include <QVariantMap>

#define MAXJOYTRIES 10

typedef QMap<int,int> mapkeys_t;
typedef QMap<QString, mapkeys_t> mapmenu_t;
typedef QMap<QString, mapmenu_t> mapwnd_t;


struct joymap_str {
    char string[32];        // Name of button/axis
    int id;                 // Pseudo key
    int value;              // value
    int type;               // event type
    int number;             // axis/button number
    int deadzone;           // +ve means a +/- deadzone around 0
                            // -ve means a trigger input (default is -32768) goes to +32768 when > deadzone.
};


int scale_joystick(int value);

class joystick : public Kinput
{
    Q_OBJECT
public:
    explicit joystick(QObject *parent = 0, QString device = "/dev/js0");
    virtual ~joystick();

    int findJoystick();
    void print_device_info();
    void process_event(struct js_event jse);
    virtual int map_button(QVariant joy);
    int convert_event2joy(struct js_event * jse);
    void loadMap(QString filename);
    void process_joy(int joy_code, int value);
    const char * decode_joy(int code);
    int get_fd() { return fd; }
    int map_input(QVariant joy);
    QString getMapName();

protected:
    virtual void run();
    void virtual parse_inputs(QVariantMap &map);
    void display_mapping();

    int fd;
    QString _device;

signals:
    void joyPress(int key, int value);
    void joyEvent(int type, int number, int value);
    void joyDebug(QString dbgmsg);

public slots:


};

#endif // JOYSTICK_H
