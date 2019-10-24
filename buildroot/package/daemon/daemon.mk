#############################################################
#
# daemon package
#
#############################################################



DAEMON_VERSION = 1.0
DAEMON_SITE = $(TOPDIR)/../daemon
DAEMON_SITE_METHOD = local
DAEMON_LICENSE = BSD-3c
DAEMON_LICENSE_FILES = LICENSE.txt
DAEMON_INSTALL_STAGING = NO

define DAEMON_BUILD_CMDS
	cd $(@D) 
	$(MAKE) -C $(@D) all
	$(TARGET_STRIP) $(@D)/daemon
endef

define DAEMON_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/daemon $(TARGET_DIR)/usr/bin/daemon
endef

$(eval $(generic-package))
