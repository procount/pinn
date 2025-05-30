################################################################################
#
# rpi-firmware
#
################################################################################

#2b41f509710d99758a5b8efa88d95dd0e9169c0a
#fcf8d2f7639ad8d0330db9c8db9b71bd33eaaa28
#RPI_FIRMWARE_VERSION = 1eb8921ae7158249894f89e8bb6c4bfb04ec7377
#RPI_FIRMWARE_VERSION = cfdbadea5f74c16b7ed5d3b4866092a054e3c3bf
#RPI_FIRMWARE_VERSION = 191360eaf2e5933eaa0ed76ac0d62722b6f9a58f
RPI_FIRMWARE_VERSION = b9dbcb006701e55e660656204afd4ebd6adecdf8
RPI_FIRMWARE_SITE = $(call github,raspberrypi,firmware,$(RPI_FIRMWARE_VERSION))
RPI_FIRMWARE_LICENSE = BSD-3c
RPI_FIRMWARE_LICENSE_FILES = boot/LICENCE.broadcom
RPI_FIRMWARE_INSTALL_TARGET = NO
RPI_FIRMWARE_INSTALL_IMAGES = YES

RPI_FIRMWARE_DEPENDENCIES += host-rpi-firmware

ifeq ($(BR2_PACKAGE_RPI_FIRMWARE_INSTALL_DTBS),y)
define RPI_FIRMWARE_INSTALL_DTB
	$(INSTALL) -D -m 0644 $(@D)/boot/bcm2708-rpi-zero-w.dtb $(BINARIES_DIR)/rpi-firmware/bcm2708-rpi-zero-w.dtb
	$(INSTALL) -D -m 0644 $(@D)/boot/bcm2708-rpi-b.dtb $(BINARIES_DIR)/rpi-firmware/bcm2708-rpi-b.dtb
	$(INSTALL) -D -m 0644 $(@D)/boot/bcm2708-rpi-b-plus.dtb $(BINARIES_DIR)/rpi-firmware/bcm2708-rpi-b-plus.dtb
	$(INSTALL) -D -m 0644 $(@D)/boot/bcm2708-rpi-cm.dtb $(BINARIES_DIR)/rpi-firmware/bcm2708-rpi-cm.dtb
	$(INSTALL) -D -m 0644 $(@D)/boot/bcm2709-rpi-2-b.dtb $(BINARIES_DIR)/rpi-firmware/bcm2709-rpi-2-b.dtb
	$(INSTALL) -D -m 0644 $(@D)/boot/bcm2710-rpi-3-b.dtb $(BINARIES_DIR)/rpi-firmware/bcm2710-rpi-3-b.dtb
        $(INSTALL) -D -m 0644 $(@D)/boot/bcm2710-rpi-3-b-plus.dtb $(BINARIES_DIR)/rpi-firmware/bcm2710-rpi-3-b-plus.dtb
	$(INSTALL) -D -m 0644 $(@D)/boot/bcm2710-rpi-cm3.dtb $(BINARIES_DIR)/rpi-firmware/bcm2710-rpi-cm3.dtb
	$(INSTALL) -D -m 0644 $(@D)/boot/bcm2711-rpi-4-b.dtb $(BINARIES_DIR)/rpi-firmware/bcm2711-rpi-4-b.dtb
	$(INSTALL) -D -m 0644 $(@D)/boot/bcm2711-rpi-cm4.dtb $(BINARIES_DIR)/rpi-firmware/bcm2711-rpi-cm4.dtb
endef
endif

ifeq ($(BR2_PACKAGE_RPI_FIRMWARE_INSTALL_DTB_OVERLAYS),y)
define RPI_FIRMWARE_INSTALL_DTB_OVERLAYS
	for ovldtb in  $(@D)/boot/overlays/*.dtbo; do \
		$(INSTALL) -D -m 0644 $${ovldtb} $(BINARIES_DIR)/rpi-firmware/overlays/$${ovldtb##*/} || exit 1; \
	done
endef
endif

ifeq ($(BR2_PACKAGE_RPI_FIRMWARE_INSTALL_VCDBG),y)
define RPI_FIRMWARE_INSTALL_TARGET_CMDS
	$(INSTALL) -d -m 0700 $(@D)/$(if BR2_ARM_EABIHF,hardfp/)opt/vc/bin/vcdbg \
		$(TARGET_DIR)/usr/sbin/vcdbg
endef
endif # INSTALL_VCDBG

define RPI_FIRMWARE_INSTALL_IMAGES_CMDS
	$(INSTALL) -D -m 0644 $(@D)/boot/bootcode.bin $(BINARIES_DIR)/rpi-firmware/bootcode.bin
	$(INSTALL) -D -m 0644 $(@D)/boot/start$(BR2_PACKAGE_RPI_FIRMWARE_BOOT).elf $(BINARIES_DIR)/rpi-firmware/start.elf
	$(INSTALL) -D -m 0644 $(@D)/boot/start4cd.elf $(BINARIES_DIR)/rpi-firmware/start4.elf
	$(INSTALL) -D -m 0644 $(@D)/boot/fixup$(BR2_PACKAGE_RPI_FIRMWARE_BOOT).dat $(BINARIES_DIR)/rpi-firmware/fixup.dat
	$(INSTALL) -D -m 0644 $(@D)/boot/fixup4cd.dat $(BINARIES_DIR)/rpi-firmware/fixup4.dat
	$(INSTALL) -D -m 0644 package/rpi-firmware/config.txt $(BINARIES_DIR)/rpi-firmware/config.txt
	$(INSTALL) -D -m 0644 package/rpi-firmware/cmdline.txt $(BINARIES_DIR)/rpi-firmware/cmdline.txt
	$(RPI_FIRMWARE_INSTALL_DTB)
	$(RPI_FIRMWARE_INSTALL_DTB_OVERLAYS)
endef

# We have no host sources to get, since we already
# bundle the script we want to install.
HOST_RPI_FIRMWARE_SOURCE =
HOST_RPI_FIRMWARE_DEPENDENCIES =

$(eval $(generic-package))
$(eval $(host-generic-package))
