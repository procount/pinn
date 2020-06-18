################################################################################
#
# xapp_xgc
#
################################################################################

XAPP_XGC_VERSION = 1.0.5
XAPP_XGC_SOURCE = xgc-$(XAPP_XGC_VERSION).tar.bz2
XAPP_XGC_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XGC_LICENSE = MIT
XAPP_XGC_LICENSE_FILES = COPYING
XAPP_XGC_DEPENDENCIES = xlib_libXaw

$(eval $(autotools-package))
