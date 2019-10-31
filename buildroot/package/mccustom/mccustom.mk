################################################################################
#
# mccustom
#
################################################################################

MCCUSTOM_VERSION = 1.0
MCCUSTOM_SITE = $(TOPDIR)/../mccustom
MCCUSTOM_SITE_METHOD = local
MCCUSTOM_LICENSE = Proprietary
#CUSTOM_LICENSE_FILES = 

define MCCUSTOM_BUILD_CMDS
	cd $(@D) 
	$(TARGET_MAKE_ENV) $(MAKE)    \
		CC="$(TARGET_CC)"           \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)"   \
		EXTRA_LDFLAGS="$(TARGET_LDFLAGS)" \
		-C $(@D) all 
	$(TARGET_STRIP) $(@D)/custom
endef

define MCCUSTOM_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/custom $(TARGET_DIR)/usr/bin/custom
endef

define HOST_MCCUSTOM_BUILD_CMDS
	cd $(@D) 
	$(HOST_MAKE_ENV) $(MAKE) \
		CC="$(HOSTCC)" \
		EXTRA_CFLAGS="$(HOST_CFLAGS)"   \
		EXTRA_LDFLAGS="$(HOST_LDFLAGS)" \
		-C $(@D) all 
#	$(HOST_STRIP) $(@D)/custom
endef


define HOST_MCCUSTOM_INSTALL_CMDS
	$(INSTALL) -D -m 0755 $(@D)/custom $(HOST_DIR)/usr/bin/custom
endef

define HOST_MCCUSTOM_ADD_NEW_FONT
	$(INSTALL) -D -m 0755 $(@D)/dejavu_sans_24_50.qsf $(HOST_DIR)/tmp/qtembedded-1/fonts/dejavu_sans_24_50.qsf
endef

HOST_MCCUSTOM_POST_INSTALL_HOOKS += HOST_MCCUSTOM_ADD_NEW_FONT

$(eval $(generic-package))
$(eval $(host-generic-package))


