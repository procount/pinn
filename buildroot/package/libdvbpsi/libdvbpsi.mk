################################################################################
#
# libdvbpsi
#
################################################################################

LIBDVBPSI_VERSION = 1.3.3
LIBDVBPSI_SITE = http://download.videolan.org/pub/libdvbpsi/$(LIBDVBPSI_VERSION)
LIBDVBPSI_SOURCE = libdvbpsi-$(LIBDVBPSI_VERSION).tar.bz2
LIBDVBPSI_LICENSE = LGPL-2.1+
LIBDVBPSI_LICENSE_FILES = COPYING
LIBDVBPSI_INSTALL_STAGING = YES

$(eval $(autotools-package))
