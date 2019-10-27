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

#include "simulate.h"

#include <QDebug>
#include <QApplication>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>


#define errexit(str, args...) do { \
        perror(str); \
        qDebug() << str; \
        return; \
    } while(0)


simulate::simulate()
{
    struct uinput_user_dev uidev;
    ok=false;
    fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
    if(fd < 0)
        fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    if(fd < 0)
        errexit("error: open \"/dev/input/uinput\" and \"/dev/uinput\"");

    if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
        errexit("error: ioctl(fd, UI_SET_EVBIT, EV_KEY)");
    if(ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0)
        errexit("error: ioctl(fd, UI_SET_KEYBIT, BTN_LEFT)");
    for (int i=0; i<256; i++)
    {
        if(ioctl(fd, UI_SET_KEYBIT, i) < 0)
            errexit("error: ioctl(fd, UI_SET_KEYBIT, i)");
    }

    if(ioctl(fd, UI_SET_EVBIT, EV_REL) < 0)
        errexit("error: ioctl(fd, UI_SET_EVBIT, EV_REL)");
    if(ioctl(fd, UI_SET_RELBIT, REL_X) < 0)
        errexit("error: ioctl(fd, UI_SET_RELBIT, REL_X)");
    if(ioctl(fd, UI_SET_RELBIT, REL_Y) < 0)
        errexit("error: ioctl(fd, UI_SET_RELBIT, REL_Y)");

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if(write(fd, &uidev, sizeof(uidev)) < 0)
        errexit("error: write &uidev");

    if(ioctl(fd, UI_DEV_CREATE) < 0)
        errexit("error: ioctl UI_DEV_CREATE");
    ok=true;
    qDebug()<< "Simulate OK";
}

void simulate::inject(int type, int code, int value)
{
    struct input_event key_input_event;

    memset(&key_input_event, 0, sizeof(input_event));

    // key release event for 'a'
    key_input_event.type = type;
    key_input_event.code = code;
    key_input_event.value = value;

    // now write to the file descriptor
    if(write(fd, &key_input_event, sizeof(input_event)) < 0)
        errexit("error: write evt");
}

simulate::~simulate()
{
    if(ioctl(fd, UI_DEV_DESTROY) < 0)
        errexit("error: ioctl UI_DEV_DESTROY");

    close(fd);
    ok=false;
}

