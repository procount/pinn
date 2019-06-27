CC=$(PWD)/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-gcc

all: rebootp
	$(MAKE) $(AM_MAKEFLAGS) rebootp

rebootp: rebootp.c
	$(CC) -static rebootp.c -o rebootp



