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
		-C $(@D) all \
        $(TARGET_STRIP) $(@D)/custom
endef


#define HOST_MCCUSTOM_BUILD_CMDS
        cd $(@D)
        $(HOST_MAKE_ENV) $(MAKE) \
                CC="$(HOSTCC)" \
                EXTRA_CFLAGS="$(HOST_CFLAGS)"   \
                EXTRA_LDFLAGS="$(HOST_LDFLAGS)" \
                -C $(@D) all \
        $(HOST_STRIP) $(@D)/custom
#endef

define MCCUSTOM_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/custom $(TARGET_DIR)/usr/bin/custom
endef

define HOST_MCCUSTOM_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 0755 $(@D)/custom $(HOST_DIR)/usr/bin/custom
endef


$(eval $(generic-package))

