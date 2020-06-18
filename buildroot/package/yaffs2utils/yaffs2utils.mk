################################################################################
#
# yaffs2utils
#
################################################################################

YAFFS2UTILS_VERSION = 0.2.9
YAFFS2UTILS_SOURCE = $(YAFFS2UTILS_VERSION).tar.gz
YAFFS2UTILS_SITE = https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/yaffs2utils
YAFFS2UTILS_LICENSE = GPL-2.0
YAFFS2UTILS_LICENSE_FILES = COPYING

define HOST_YAFFS2UTILS_BUILD_CMDS
	$(HOST_MAKE_ENV) $(MAKE) -C $(@D)
endef

define HOST_YAFFS2UTILS_INSTALL_CMDS
	mkdir -p $(HOST_DIR)/bin
	$(HOST_MAKE_ENV) $(MAKE) -C $(@D) INSTALLDIR=$(HOST_DIR)/bin install
endef

$(eval $(host-generic-package))
