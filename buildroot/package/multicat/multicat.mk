################################################################################
#
# multicat
#
################################################################################

MULTICAT_VERSION = 2.3
MULTICAT_SOURCE = multicat-$(MULTICAT_VERSION).tar.bz2
MULTICAT_SITE = https://get.videolan.org/multicat/$(MULTICAT_VERSION)
MULTICAT_LICENSE = GPL-2.0+
MULTICAT_LICENSE_FILES = COPYING

MULTICAT_DEPENDENCIES = bitstream

# Makefile does not use LDFLAGS. Use LDLIBS for that instead.
MULTICAT_MAKE_ENV = $(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) \
	LDLIBS="$(TARGET_LDFLAGS)"

define MULTICAT_BUILD_CMDS
	$(MULTICAT_MAKE_ENV) $(MAKE) -C $(@D)
endef

define MULTICAT_INSTALL_TARGET_CMDS
	$(MULTICAT_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) PREFIX=/usr install
endef

$(eval $(generic-package))
