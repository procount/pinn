#CC=$(PWD)/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-gcc

all: busyboc
	$(MAKE) $(AM_MAKEFLAGS) busyboc

busyboc: busyboc.c
	$(CC) -static busyboc.c -o busyboc



