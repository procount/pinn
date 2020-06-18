################################################################################
#
# xapp_xditview
#
################################################################################

XAPP_XDITVIEW_VERSION = 1.0.5
XAPP_XDITVIEW_SOURCE = xditview-$(XAPP_XDITVIEW_VERSION).tar.bz2
XAPP_XDITVIEW_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XDITVIEW_LICENSE = MIT
XAPP_XDITVIEW_LICENSE_FILES = COPYING
XAPP_XDITVIEW_DEPENDENCIES = xlib_libXaw
XAPP_XDITVIEW_CONF_OPTS = --with-appdefaultdir=/usr/share/X11/app-defaults

$(eval $(autotools-package))
