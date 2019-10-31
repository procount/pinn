#These are normally built on the HOST PC

all: daemon xor createkeys makeblock
#all: createkeys 

createkeys: createkeys.c
	$(CC) -static createkeys.c -o createkeys
	$(MAKE) $(AM_MAKEFLAGS) createkeys
	
daemon: daemon.c daemon.h
	$(CC) -static daemon.c -o daemon
	$(MAKE) $(AM_MAKEFLAGS) daemon

xor: xor.c
	$(CC) -static xor.c -o xor
	$(MAKE) $(AM_MAKEFLAGS) xor
