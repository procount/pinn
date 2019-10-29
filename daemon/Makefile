#CC=$(PWD)/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-gcc
#These are normally built on the HOST PC

all: daemon xor createKeys

daemon: daemon.c
	$(CC) -static daemon.c -o daemon
	$(MAKE) $(AM_MAKEFLAGS) daemon

xor: xor.c
	$(CC) -static xor.c -o xor
	$(MAKE) $(AM_MAKEFLAGS) xor

createKeys: createKeys.c
	$(CC) -static createKeys.c -o createKeys
	$(MAKE) $(AM_MAKEFLAGS) createKeys



