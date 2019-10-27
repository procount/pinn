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
    virtual int map_button(QVariant joy);
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
