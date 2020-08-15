#############################################################
#
# Arora 
#
#############################################################

ARORA_VERSION = 9f2fbe2cc98a6d14995d66af2ebfe8f5dacfe23f
ARORA_SITE = $(call github,AaronDewes,arora-unofficial-updates,$(ARORA_VERSION))
ARORA_LICENSE = GPLv2
ARORA_LICENSE_FILES = LICENSE.GPL2
ARORA_INSTALL_STAGING = NO
ARORA_DEPENDENCIES = qt5base qt5webkit qt5tools qt5script

define ARORA_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/arora $(TARGET_DIR)/usr/bin/arora
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/.qws/share/data/Arora/locale
	$(INSTALL) -D -m 0755 $(@D)/src/*.qm $(TARGET_DIR)/.qws/share/data/Arora/locale
endef

$(eval $(cmake-package))
