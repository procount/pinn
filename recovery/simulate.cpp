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

#if 0
#define errexit(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

int main(void)
{
    int fd;
    struct uinput_user_dev uidev;

    fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
    if(fd < 0)
        fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    if(fd < 0)
        errexit("error: open \"/dev/input/uinput\" and \"/dev/uinput\"");

    if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
        errexit("error: ioctl(fd, UI_SET_EVBIT, EV_KEY)");
    if(ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0)
        errexit("error: ioctl(fd, UI_SET_KEYBIT, BTN_LEFT)");

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

    sleep(1);

    struct input_event ev;
    int i, dx, dy;
    for (i = 0; i < 20; i++)
    {
        switch(i % 4) {
        case 0:
            dx = 50;
            dy = -50;
            break;
        case 1:
            dx = 50;
            dy = 50;
            break;
        case 2:
            dx = -50;
            dy = 50;
            break;
        case 3:
            dx = -50;
            dy = -50;
            break;
        }

        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_REL;
        ev.code = REL_X;
        ev.value = dx;
        if(write(fd, &ev, sizeof(struct input_event)) < 0)
            errexit("error: write");

        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_REL;
        ev.code = REL_Y;
        ev.value = dy;
        if(write(fd, &ev, sizeof(struct input_event)) < 0)
            errexit("error: write");

        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_SYN;
        if(write(fd, &ev, sizeof(struct input_event)) < 0)
            errexit("error: write");

        usleep(800000);
    }

    sleep(2);

    if(ioctl(fd, UI_DEV_DESTROY) < 0)
        errexit("error: ioctl UI_DEV_DESTROY");

    close(fd);

    return 0;
}
#endif

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

simulate::~simulate()
{
    if(ioctl(fd, UI_DEV_DESTROY) < 0)
        errexit("error: ioctl UI_DEV_DESTROY");

    close(fd);
    ok=false;
}

void simulate::test()
{
    if (!ok)
        return;

    sleep(1);

    struct input_event ev;
    int i, dx, dy;
    for (i = 0; i < 20; i++)
    {
        switch(i % 4) {
        case 0:
            dx = 50;
            dy = -50;
            break;
        case 1:
            dx = 50;
            dy = 50;
            break;
        case 2:
            dx = -50;
            dy = 50;
            break;
        case 3:
            dx = -50;
            dy = -50;
            break;
        }

        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_REL;
        ev.code = REL_X;
        ev.value = dx;
        if(write(fd, &ev, sizeof(struct input_event)) < 0)
            errexit("error: write");

        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_REL;
        ev.code = REL_Y;
        ev.value = dy;
        if(write(fd, &ev, sizeof(struct input_event)) < 0)
            errexit("error: write");

        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_SYN;
        if(write(fd, &ev, sizeof(struct input_event)) < 0)
            errexit("error: write");

        QApplication::processEvents();
        usleep(800000);
    }

    sleep(2);

    return;
}
