################################################################################
#
# liblog4c-localtime
#
################################################################################

LIBLOG4C_LOCALTIME_VERSION = 1.0
LIBLOG4C_LOCALTIME_SITE = $(call github,rcmadruga,log4c-localtime,v$(LIBLOG4C_LOCALTIME_VERSION))
LIBLOG4C_LOCALTIME_INSTALL_STAGING = YES
LIBLOG4C_LOCALTIME_CONF_OPTS = --disable-expattest
LIBLOG4C_LOCALTIME_DEPENDENCIES = expat
LIBLOG4C_LOCALTIME_CONFIG_SCRIPTS = log4c-config
LIBLOG4C_LOCALTIME_LICENSE = LGPL-2.1
LIBLOG4C_LOCALTIME_LICENSE_FILES = COPYING
LIBLOG4C_LOCALTIME_AUTORECONF = YES

define LIBLOG4C_LOCALTIME_FIX_CONFIGURE_PERMS
	chmod +x $(@D)/configure
endef

LIBLOG4C_LOCALTIME_PRE_CONFIGURE_HOOKS += LIBLOG4C_LOCALTIME_FIX_CONFIGURE_PERMS

$(eval $(autotools-package))
