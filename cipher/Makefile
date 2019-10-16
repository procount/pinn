CC=$(PWD)/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-gcc

all: cipher
	$(MAKE) $(AM_MAKEFLAGS) cipher

cipher: cipher.c
	$(CC) -static cipher.c -o cipher



