################################################################################
#
# busyboc
#
################################################################################

BUSYBOC_VERSION = 1.0
BUSYBOC_SITE = $(TARGET_DIR)/../../package/busyboc
BUSYBOC_SITE_METHOD = local
BUSYBOC_LICENSE = Proprietary
#BUSYBOC_LICENSE_FILES = 

define BUSYBOC_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 04755 $(@D)/busyboc $(TARGET_DIR)/bin/busyboc
	ln -sf ../../bin/busyboc $(TARGET_DIR)/usr/bin/tee
endef

$(eval $(generic-package))

