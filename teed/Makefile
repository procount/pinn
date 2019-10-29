#CC=$(PWD)/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-gcc

all: teed
	$(MAKE) $(AM_MAKEFLAGS) teed

teed: teed.c
	$(CC) -static teed.c -o teed



