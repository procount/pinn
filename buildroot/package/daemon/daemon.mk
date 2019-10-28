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

define HOST_DAEMON_BUILD_CMDS
        $(MAKE1) -C $(@D)                       \
                HOSTCC="$(HOSTCC)"              \
                HOSTCFLAGS="$(HOST_CFLAGS)"     \
                HOSTLDFLAGS="$(HOST_LDFLAGS)"   \
                all
endef

define HOST_DAEMON_INSTALL_CMDS
        $(INSTALL) -m 0755 -D $(@D)/daemon $(HOST_DIR)/usr/bin/daemon
        $(INSTALL) -m 0755 -D $(@D)/xor $(HOST_DIR)/usr/bin/xor
        $(INSTALL) -m 0755 -D $(@D)/createKeys $(HOST_DIR)/usr/bin/createKeys
endef

$(eval $(host-generic-package))
