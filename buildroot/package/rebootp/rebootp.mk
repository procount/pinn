#############################################################
#
# Reboot system
#
#############################################################


REBOOTP_VERSION = 1.0
REBOOTP_SITE = $(TOPDIR)/../rebootp
REBOOTP_SITE_METHOD = local
REBOOTP_LICENSE = BSD-3c
REBOOTP_LICENSE_FILES = LICENSE.txt
REBOOTP_INSTALL_STAGING = NO

define REBOOTP_BUILD_CMDS
	cd $(@D) 
	$(MAKE) -C $(@D) all
	$(TARGET_STRIP) $(@D)/rebootp
endef

define REBOOTP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/rebootp $(TARGET_DIR)/usr/bin/rebootp
	$(INSTALL) -m 0755 package/rebootp/rebootp.sh $(TARGET_DIR)/usr/bin/rebootp.sh
endef

$(eval $(generic-package))
