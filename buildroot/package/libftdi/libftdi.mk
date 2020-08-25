################################################################################
#
# libftdi
#
################################################################################

LIBFTDI_VERSION = 0.20
LIBFTDI_SITE = http://www.intra2net.com/en/developer/libftdi/download
LIBFTDI_LICENSE = LGPL-2.1 (libftdi)
LIBFTDI_LICENSE_FILES = LICENSE COPYING.LIB
LIBFTDI_DEPENDENCIES = libusb-compat libusb
HOST_LIBFTDI_DEPENDENCIES = host-libusb-compat host-libusb
LIBFTDI_INSTALL_STAGING = YES
LIBFTDI_CONFIG_SCRIPTS = libftdi-config
LIBFTDI_AUTORECONF = YES

LIBFTDI_CONF_OPTS = --without-examples

# configure detect it automaticaly so we need to force it
ifeq ($(BR2_PACKAGE_LIBFTDI_CPP),y)
LIBFTDI_LICENSE += , GPL-2.0 with exception (ftdipp)
LIBFTDI_LICENSE_FILES += COPYING.GPL
LIBFTDI_DEPENDENCIES += boost
LIBFTDI_CONF_OPTS += --enable-libftdipp
else
LIBFTDI_CONF_OPTS += --disable-libftdipp
endif

$(eval $(autotools-package))
$(eval $(host-autotools-package))
