################################################################################
#
# custom
#
################################################################################

CUSTOM_VERSION = 1.0
CUSTOM_SITE = $(TOPDIR)/../custom
CUSTOM_SITE_METHOD = local
CUSTOM_LICENSE = Proprietary
#CUSTOM_LICENSE_FILES = 

define CUSTOM_BUILD_CMDS
        cd $(@D)
        $(TARGET_MAKE_ENV) $(MAKE)    \
                CC="$(TARGET_CC)"           \
                EXTRA_CFLAGS="$(TARGET_CFLAGS)"   \
                EXTRA_LDFLAGS="$(TARGET_LDFLAGS)" \
        		-C $(@D) all \
        $(TARGET_STRIP) $(@D)/custom
endef

define CUSTOM_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/custom $(TARGET_DIR)/usr/bin/custom
endef

$(eval $(generic-package))

