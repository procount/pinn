CC=$(PWD)/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-gcc

all: daemon
	$(MAKE) $(AM_MAKEFLAGS) daemon

daemon: daemon.c
	$(CC) -static daemon.c -o daemon



