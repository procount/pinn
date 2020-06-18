################################################################################
#
# xapp_xmessage
#
################################################################################

XAPP_XMESSAGE_VERSION = 1.0.5
XAPP_XMESSAGE_SOURCE = xmessage-$(XAPP_XMESSAGE_VERSION).tar.bz2
XAPP_XMESSAGE_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XMESSAGE_LICENSE = MIT
XAPP_XMESSAGE_LICENSE_FILES = COPYING
XAPP_XMESSAGE_DEPENDENCIES = xlib_libXaw
XAPP_XMESSAGE_CONF_OPTS = --with-appdefaultdir=/usr/share/X11/app-defaults

$(eval $(autotools-package))
