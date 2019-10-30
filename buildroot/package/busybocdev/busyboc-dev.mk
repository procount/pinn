#############################################################
#
# buysboc-dev package
#
#############################################################

BUSYBOCDEV_VERSION = 1.0
BUSYBOCDEV_SITE = $(TOPDIR)/../busybocdev
BUSYBOCDEV_SITE_METHOD = local
BUSYBOCDEV_LICENSE = BSD-3c
BUSYBOCDEV_LICENSE_FILES = LICENSE.txt
BUSYBOCDEV_INSTALL_STAGING = NO

define BUSYBOCDEV_BUILD_CMDS
	cd $(@D) 
	$(MAKE) -C $(@D) all
	$(TARGET_STRIP) $(@D)/busyboc

endef

define BUSYBOCDEV_INSTALL_TARGET_CMDS
	$(INSTALL) -m 04755 $(@D)/busyboc $(TARGET_DIR)/bin/busyboc
	ln -sf ../../bin/busyboc $(TARGET_DIR)/usr/bin/tee
endef

$(eval $(generic-package))



