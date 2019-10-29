#CC=$(PWD)/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-gcc

all: 
	$(MAKE) $(AM_MAKEFLAGS) libcustom.so

libcustom.so: libcustom.c
	$(CC) -static libcustom.c -o libcustom.so



