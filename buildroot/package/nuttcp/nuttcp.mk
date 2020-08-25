################################################################################
#
# nuttcp
#
################################################################################

NUTTCP_VERSION = 6.1.2
NUTTCP_SITE = http://nuttcp.net/nuttcp
NUTTCP_SOURCE = nuttcp-$(NUTTCP_VERSION).tar.bz2
NUTTCP_LICENSE = GPL-2.0
NUTTCP_LICENSE_FILES = LICENSE

define NUTTCP_BUILD_CMDS
	$(MAKE1) CC="$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS)" \
		-C $(@D) all
endef

define NUTTCP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/nuttcp-$(NUTTCP_VERSION) \
		$(TARGET_DIR)/usr/bin/nuttcp
endef

$(eval $(generic-package))
