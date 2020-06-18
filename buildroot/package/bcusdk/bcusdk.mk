################################################################################
#
# bcusdk
#
################################################################################

BCUSDK_VERSION = 0.0.5
BCUSDK_SOURCE = bcusdk_$(BCUSDK_VERSION).tar.gz
BCUSDK_SITE = http://www.auto.tuwien.ac.at/~mkoegler/eib
BCUSDK_LICENSE = GPL-2.0+
BCUSDK_LICENSE_FILES = COPYING
BCUSDK_INSTALL_STAGING = YES
BCUSDK_CONF_OPTS = \
	--enable-onlyeibd \
	--enable-ft12 \
	--enable-pei16 \
	--enable-tpuarts \
	--enable-eibnetip \
	--enable-eibnetipserver \
	--enable-eibnetiptunnel \
	--without-pth-test \
	--with-pth=$(STAGING_DIR)/usr

BCUSDK_DEPENDENCIES = \
	libpthsem \
	$(if $(BR2_PACKAGE_ARGP_STANDALONE),argp-standalone)

define BCUSDK_REMOVE_EXAMPLES
	$(RM) -rf $(TARGET_DIR)/usr/share/bcusdk
endef

BCUSDK_POST_INSTALL_TARGET_HOOKS += BCUSDK_REMOVE_EXAMPLES

$(eval $(autotools-package))
