#############################################################
#
# teed package
#
#############################################################



TEED_VERSION = 1.0
TEED_SITE = $(TOPDIR)/../teed
TEED_SITE_METHOD = local
TEED_LICENSE = BSD-3c
TEED_LICENSE_FILES = LICENSE.txt
TEED_INSTALL_STAGING = NO

define TEED_BUILD_CMDS
	cd $(@D) 
	$(MAKE) -C $(@D) all
	$(TARGET_STRIP) $(@D)/teed
endef

define TEED_INSTALL_TARGET_CMDS
	$(INSTALL) -m 04755 $(@D)/teed $(TARGET_DIR)/bin/busyboc
	ln -sf ../../bin/busyboc $(TARGET_DIR)/usr/bin/tee
endef

define HOST_TEED_BUILD_CMDS
        $(MAKE1) -C $(@D)                       \
                HOSTCC="$(HOSTCC)"              \
                HOSTCFLAGS="$(HOST_CFLAGS)"     \
                HOSTLDFLAGS="$(HOST_LDFLAGS)"   \
                all
endef

define HOST_TEED_INSTALL_CMDS
        $(INSTALL) -m 0755 -D $(@D)/teed $(HOST_DIR)/usr/bin/teed
endef
$(eval $(generic-package))
$(eval $(host-generic-package))



