################################################################################
#
# bluez5_utils
#
################################################################################

# Keep the version and patches in sync with bluez5_utils-headers
BLUEZ5_UTILS_VERSION = 5.54
BLUEZ5_UTILS_SOURCE = bluez-$(BLUEZ5_UTILS_VERSION).tar.xz
BLUEZ5_UTILS_SITE = $(BR2_KERNEL_MIRROR)/linux/bluetooth
BLUEZ5_UTILS_INSTALL_STAGING = YES
BLUEZ5_UTILS_LICENSE = GPL-2.0+, LGPL-2.1+
BLUEZ5_UTILS_LICENSE_FILES = COPYING COPYING.LIB

BLUEZ5_UTILS_DEPENDENCIES = \
	$(if $(BR2_PACKAGE_BLUEZ5_UTILS_HEADERS),bluez5_utils-headers) \
	dbus \
	libglib2

BLUEZ5_UTILS_CONF_OPTS = \
	--enable-tools \
	--enable-library \
	--disable-cups \
	--with-dbusconfdir=/etc

ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_OBEX),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-obex
BLUEZ5_UTILS_DEPENDENCIES += libical
else
BLUEZ5_UTILS_CONF_OPTS += --disable-obex
endif

ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_CLIENT),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-client
BLUEZ5_UTILS_DEPENDENCIES += readline
else
BLUEZ5_UTILS_CONF_OPTS += --disable-client
endif

# experimental plugins
ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_EXPERIMENTAL),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-experimental
else
BLUEZ5_UTILS_CONF_OPTS += --disable-experimental
endif

# enable health plugin
ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_PLUGINS_HEALTH),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-health
else
BLUEZ5_UTILS_CONF_OPTS += --disable-health
endif

# enable mesh profile
ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_PLUGINS_MESH),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-external-ell --enable-mesh
BLUEZ5_UTILS_DEPENDENCIES += ell json-c readline
else
BLUEZ5_UTILS_CONF_OPTS += --disable-external-ell --disable-mesh
endif

# enable midi profile
ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_PLUGINS_MIDI),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-midi
BLUEZ5_UTILS_DEPENDENCIES += alsa-lib
else
BLUEZ5_UTILS_CONF_OPTS += --disable-midi
endif

# enable nfc plugin
ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_PLUGINS_NFC),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-nfc
else
BLUEZ5_UTILS_CONF_OPTS += --disable-nfc
endif

# enable sap plugin
ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_PLUGINS_SAP),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-sap
else
BLUEZ5_UTILS_CONF_OPTS += --disable-sap
endif

# enable sixaxis plugin
ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_PLUGINS_SIXAXIS),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-sixaxis
else
BLUEZ5_UTILS_CONF_OPTS += --disable-sixaxis
endif

# install gatttool (For some reason upstream choose not to do it by default)
ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_DEPRECATED),y)
define BLUEZ5_UTILS_INSTALL_GATTTOOL
	$(INSTALL) -D -m 0755 $(@D)/attrib/gatttool $(TARGET_DIR)/usr/bin/gatttool
endef
BLUEZ5_UTILS_POST_INSTALL_TARGET_HOOKS += BLUEZ5_UTILS_INSTALL_GATTTOOL
# hciattach_bcm43xx defines default firmware path in `/etc/firmware`, but
# Broadcom firmware blobs are usually located in `/lib/firmware`.
BLUEZ5_UTILS_CONF_ENV += \
	CPPFLAGS='$(TARGET_CPPFLAGS) -DFIRMWARE_DIR=\"/lib/firmware\"'
BLUEZ5_UTILS_CONF_OPTS += --enable-deprecated
else
BLUEZ5_UTILS_CONF_OPTS += --disable-deprecated
endif

# enable test
ifeq ($(BR2_PACKAGE_BLUEZ5_UTILS_TEST),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-test
else
BLUEZ5_UTILS_CONF_OPTS += --disable-test
endif

# use udev if available
ifeq ($(BR2_PACKAGE_HAS_UDEV),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-udev
BLUEZ5_UTILS_DEPENDENCIES += udev
else
BLUEZ5_UTILS_CONF_OPTS += --disable-udev
endif

# integrate with systemd if available
ifeq ($(BR2_PACKAGE_SYSTEMD),y)
BLUEZ5_UTILS_CONF_OPTS += --enable-systemd
BLUEZ5_UTILS_DEPENDENCIES += systemd
else
BLUEZ5_UTILS_CONF_OPTS += --disable-systemd
endif

$(eval $(autotools-package))
