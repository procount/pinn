################################################################################
#
# mrouted
#
################################################################################

MROUTED_VERSION = 3.9.8
MROUTED_SITE = $(call github,troglobit,mrouted,$(MROUTED_VERSION))
MROUTED_DEPENDENCIES = host-bison
MROUTED_LICENSE = BSD-3-Clause
MROUTED_LICENSE_FILES = LICENSE

define MROUTED_CONFIGURE_CMDS
	(cd $(@D); \
		$(TARGET_MAKE_ENV) $(@D)/configure --enable-rsrr \
	)
endef

define MROUTED_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define MROUTED_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) prefix=/usr DESTDIR=$(TARGET_DIR) -C $(@D) install
endef

$(eval $(generic-package))
