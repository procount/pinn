################################################################################
#
# libserialport
#
################################################################################

LIBSERIALPORT_VERSION = 0.1.1
LIBSERIALPORT_SITE = http://sigrok.org/download/source/libserialport
LIBSERIALPORT_LICENSE = LGPL-3.0+
LIBSERIALPORT_LICENSE_FILES = COPYING
LIBSERIALPORT_INSTALL_STAGING = YES
LIBSERIALPORT_DEPENDENCIES = host-pkgconf
LIBSERIALPORT_AUTORECONF = YES

$(eval $(autotools-package))
