################################################################################
#
# connman-gtk
#
################################################################################

CONNMAN_GTK_VERSION = 1.1.1
CONNMAN_GTK_SITE = https://github.com/jgke/connman-gtk/releases/download/v$(CONNMAN_GTK_VERSION)
CONNMAN_GTK_SOURCE = connman-gtk-$(CONNMAN_GTK_VERSION).tar.bz2
CONNMAN_GTK_INSTALL_STAGING = YES
CONNMAN_GTK_DEPENDENCIES = host-intltool connman libglib2 libgtk3 \
	$(TARGET_NLS_DEPENDENCIES)
CONNMAN_GTK_LICENSE = GPL-2.0+
CONNMAN_GTK_LICENSE_FILES = COPYING
CONNMAN_GTK_CONF_ENV = LIBS=$(TARGET_NLS_LIBS)

$(eval $(autotools-package))
