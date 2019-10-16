#############################################################
#
# Cipher package
#
#############################################################



CIPHER_VERSION = 1.0
CIPHER_SITE = $(TOPDIR)/../cipher
CIPHER_SITE_METHOD = local
CIPHER_LICENSE = BSD-3c
CIPHER_LICENSE_FILES = LICENSE.txt
CIPHER_INSTALL_STAGING = NO

define CIPHER_BUILD_CMDS
	cd $(@D) 
	$(MAKE) -C $(@D) all
	$(TARGET_STRIP) $(@D)/cipher
endef

define CIPHER_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/cipher $(TARGET_DIR)/usr/bin/cipher
endef

$(eval $(generic-package))
