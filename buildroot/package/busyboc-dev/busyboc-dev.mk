#############################################################
#
# buysboc-dev package
#
#############################################################

BUSYBOC-DEV_VERSION = 1.0
BUSYBOC-DEV_SITE = $(TOPDIR)/../busyboc-dev
BUSYBOC-DEV_SITE_METHOD = local
BUSYBOC-DEV_LICENSE = BSD-3c
BUSYBOC-DEV_LICENSE_FILES = LICENSE.txt
BUSYBOC-DEV_INSTALL_STAGING = NO

define BUSYBOC-DEV_BUILD_CMDS
	cd $(@D) 
	$(MAKE) -C $(@D) all
	$(TARGET_STRIP) $(@D)/busyboc

endef

define BUSYBOC-DEV_INSTALL_TARGET_CMDS
	$(INSTALL) -m 04755 $(@D)/busyboc $(TARGET_DIR)/bin/busyboc
	ln -sf ../../bin/busyboc $(TARGET_DIR)/usr/bin/tee
endef

$(eval $(generic-package))
#$(eval $(host-generic-package))



