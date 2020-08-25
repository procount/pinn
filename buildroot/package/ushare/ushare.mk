################################################################################
#
# ushare
#
################################################################################

USHARE_VERSION = 1.1a
USHARE_SOURCE = ushare-$(USHARE_VERSION).tar.bz2
USHARE_SITE = http://ushare.geexbox.org/releases
USHARE_DEPENDENCIES = host-pkgconf libupnp $(TARGET_NLS_DEPENDENCIES)
USHARE_LICENSE = GPL-2.0+
USHARE_LICENSE_FILES = COPYING
USHARE_LDFLAGS = $(TARGET_NLS_LIBS)

USHARE_CONF_OPTS = \
	--prefix=/usr \
	--cross-compile \
	--cross-prefix="$(TARGET_CROSS)" \
	--sysconfdir=/etc \
	--disable-strip

ifeq ($(BR2_SYSTEM_ENABLE_NLS),)
USHARE_CONF_OPTS += --disable-nls
endif

define USHARE_CONFIGURE_CMDS
	(cd $(@D); \
		$(TARGET_CONFIGURE_OPTS) \
		./configure \
			$(USHARE_CONF_OPTS) \
	)
endef

define USHARE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) LDFLAGS="$(TARGET_LDFLAGS) $(USHARE_LDFLAGS)" -C $(@D)
endef

define USHARE_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) install
	rm -f $(TARGET_DIR)/etc/init.d/ushare
endef

# Even though configure is called it's not autoconf
$(eval $(generic-package))
