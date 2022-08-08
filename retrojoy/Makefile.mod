#EXECS  = retrogame gamera
EXECS  = retrogame
CFLAGS = -Wall -static -Ofast -fomit-frame-pointer -funroll-loops -s \
 -I~/pinn-ub2004/buildroot/output/build/host-rpi-firmware-191360eaf2e5933eaa0ed76ac0d62722b6f9a58f/opt/vc/include \
 -I~/pinn-ub2004/buildroot/output/build/host-rpi-firmware-191360eaf2e5933eaa0ed76ac0d62722b6f9a58f/opt/vc/include/interface/vcos/pthreads \
 -I~/pinn-ub2004/buildroot/output/build/host-rpi-firmware-191360eaf2e5933eaa0ed76ac0d62722b6f9a58f/opt/vc/include/interface/vmcs_host \
 -I~/pinn-ub2004/buildroot/output/build/host-rpi-firmware-191360eaf2e5933eaa0ed76ac0d62722b6f9a58f/opt/vc/include/interface/vmcs_host/linux \
 -L~/pinn-ub2004/buildroot/output/build/host-rpi-firmware-191360eaf2e5933eaa0ed76ac0d62722b6f9a58f/opt/vc/lib
LIBS   = -lbcm_host
CC     = gcc $(CFLAGS)

all: $(EXECS)

retrogame: retrogame.c keyTable.h
	$(CC) $< $(LIBS) -o $@
	strip $@

# KEYFILE = /usr/include/linux/input.h
KEYFILE = /usr/include/linux/input-event-codes.h
keyTable.h: keyTableGen.sh $(KEYFILE)
	sh $^ >$@

gamera: gamera.c
	$(CC) $< -lncurses -lmenu -lexpat -o $@
	strip $@

install:
	mv $(EXECS) /usr/local/bin

clean:
	rm -f $(EXECS) keyTable.h
