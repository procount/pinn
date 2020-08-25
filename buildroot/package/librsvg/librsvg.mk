################################################################################
#
# librsvg
#
################################################################################

LIBRSVG_VERSION_MAJOR = 2.40
LIBRSVG_VERSION = $(LIBRSVG_VERSION_MAJOR).20
LIBRSVG_SITE = http://ftp.gnome.org/pub/gnome/sources/librsvg/$(LIBRSVG_VERSION_MAJOR)
LIBRSVG_SOURCE = librsvg-$(LIBRSVG_VERSION).tar.xz
LIBRSVG_INSTALL_STAGING = YES
LIBRSVG_CONF_ENV = LIBS=$(TARGET_NLS_LIBS)
LIBRSVG_CONF_OPTS = --disable-pixbuf-loader --disable-tools
HOST_LIBRSVG_CONF_OPTS = --enable-introspection=no
LIBRSVG_DEPENDENCIES = cairo host-gdk-pixbuf gdk-pixbuf libcroco libglib2 libxml2 pango \
	$(if $(BR2_PACKAGE_LIBGTK3),libgtk3) \
	$(TARGET_NLS_DEPENDENCIES)
HOST_LIBRSVG_DEPENDENCIES = host-cairo host-gdk-pixbuf host-libcroco host-libglib2 host-libxml2 host-pango
LIBRSVG_LICENSE = LGPL-2.0+
LIBRSVG_LICENSE_FILES = COPYING.LIB

ifeq ($(BR2_PACKAGE_GOBJECT_INTROSPECTION),y)
LIBRSVG_CONF_OPTS += --enable-introspection
LIBRSVG_DEPENDENCIES += gobject-introspection
else
LIBRSVG_CONF_OPTS += --disable-introspection
endif

$(eval $(autotools-package))
$(eval $(host-autotools-package))
