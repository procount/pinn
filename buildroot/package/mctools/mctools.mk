#############################################################
#
# mctools package
#
#############################################################

MCTOOLS_VERSION = 1.0
MCTOOLS_SITE = $(TOPDIR)/../mctools
MCTOOLS_SITE_METHOD = local
MCTOOLS_LICENSE = BSD-3c
MCTOOLS_LICENSE_FILES = LICENSE.txt
MCTOOLS_INSTALL_STAGING = NO

define HOST_MCTOOLS_BUILD_CMDS
        $(MAKE1) -C $(@D)                       \
                HOSTCC="$(HOSTCC)"              \
                HOSTCFLAGS="$(HOST_CFLAGS)"     \
                HOSTLDFLAGS="$(HOST_LDFLAGS)"   \
                all
endef

define HOST_MCTOOLS_INSTALL_CMDS
        $(INSTALL) -m 0755 -D $(@D)/createkeys $(HOST_DIR)/usr/bin/createkeys
        $(INSTALL) -m 0755 -D $(@D)/makeblock $(HOST_DIR)/usr/bin/makeblock
        $(INSTALL) -m 0755 -D $(@D)/daemon $(HOST_DIR)/usr/bin/daemon
        $(INSTALL) -m 0755 -D $(@D)/xor $(HOST_DIR)/usr/bin/xor
endef

$(eval $(host-generic-package))
