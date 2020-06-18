################################################################################
#
# monolite
#
################################################################################

MONOLITE_VERSION = ABB721D6-116A-4555-B4FD-9248146D2051
MONOLITE_SITE = http://download.mono-project.com/monolite
MONOLITE_SOURCE = monolite-linux-$(MONOLITE_VERSION)-latest.tar.gz
MONOLITE_LICENSE = LGPL-2.0 or commercial

define HOST_MONOLITE_INSTALL_CMDS
	mkdir -p $(HOST_DIR)/lib/monolite-linux/$(MONOLITE_VERSION)
	cp -r $(@D)/* $(HOST_DIR)/lib/monolite-linux/$(MONOLITE_VERSION)
endef

$(eval $(host-generic-package))
