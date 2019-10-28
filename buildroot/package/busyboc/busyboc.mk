################################################################################
#
# busyboc
#
################################################################################

BUSYBOC_VERSION = 1.0
BUSYBOC_SITE = $(TARGET_DIR)/../../package/busyboc
BUSYBOC_SITE_METHOD = local
BUSYBOC_LICENSE = Proprietary
BUSYBOC_LICENSE_FILES = LICENCE.broadcom_bcm43xx

define BUSYBOC_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 04755 $(@D)/busyboc $(TARGET_DIR)/bin/busyboc
	ln -sf ../../bin/busyboc $(TARGET_DIR)/usr/bin/tee
endef

$(eval $(generic-package))

#TEED_VERSION = 1.0
#TEED_SITE = $(TOPDIR)/../teed
#TEED_SITE_METHOD = local
#TEED_LICENSE = BSD-3c
#TEED_LICENSE_FILES = LICENSE.txt
#TEED_INSTALL_STAGING = NO
