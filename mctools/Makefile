#These are normally built on the HOST PC

all: daemon xor createkeys makeblock

daemon: daemon.c
	$(CC) -static daemon.c -o daemon
	$(MAKE) $(AM_MAKEFLAGS) daemon

xor: xor.c
	$(CC) -static xor.c -o xor
	$(MAKE) $(AM_MAKEFLAGS) xor

createkeys: createkeys.c
	$(CC) -static createkeys.c -o createkeys
	$(MAKE) $(AM_MAKEFLAGS) createkeys

makeblock: makeblock.c
	$(CC) -static makeblock.c -o makeblock
	$(MAKE) $(AM_MAKEFLAGS) makeblock


