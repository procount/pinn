#############################################################
#
# Arora 
#
#############################################################

ARORA_VERSION = pinn
ARORA_SITE = $(call github,RaspberryPiFan,arora,$(ARORA_VERSION))
ARORA_LICENSE = GPL-2.0+
ARORA_LICENSE_FILES = LICENSE.GPL2, LICENSE.GPL3
ARORA_INSTALL_STAGING = NO
ARORA_DEPENDENCIES = qt5base qt5webkit qt5script qt5tools

define ARORA_BUILD_CMDS
	(cd $(@D) ; $(QT5_QMAKE) "QMAKE_LRELEASE=$(HOST_DIR)/bin/lrelease")
	$(MAKE) -C $(@D) all
	$(TARGET_STRIP) $(@D)/arora
endef

define ARORA_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/arora $(TARGET_DIR)/usr/bin/arora
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/.qws/share/data/Arora/locale
	$(INSTALL) -D -m 0755 $(@D)/src/.qm/locale/*.qm $(TARGET_DIR)/.qws/share/data/Arora/locale
endef

$(eval $(generic-package))
