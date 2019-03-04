build:
	dtc -I dts -O dtb -o pimhyp3.dtbo pimhyp3.dts
	dtc -I dts -O dtb -o pimhyp4.dtbo pimhyp4.dts
	dtc -I dts -O dtb -o cypress.dtbo cypress.dts

install:
	mkdir -p /tmp/1
	sudo mount /dev/mmcblk0p1 /tmp/1
	sudo cp pimhyp3.dtbo /tmp/1/overlays/
	sudo cp pimhyp4.dtbo /tmp/1/overlays/
	sudo cp cypress.dtbo /tmp/1/overlays/

test:
	dtoverlay pimhyp4.dtbo

init:
	gcc -o hyperpixel4-init -static hyperpixel4-init.c -lbcm2835
