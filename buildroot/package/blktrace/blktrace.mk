################################################################################
#
# blktrace
#
################################################################################

BLKTRACE_VERSION = 1.2.0
BLKTRACE_SITE = http://brick.kernel.dk/snaps
BLKTRACE_DEPENDENCIES = libaio
BLKTRACE_LICENSE = GPL-2.0+
BLKTRACE_LICENSE_FILES = COPYING

# 0001-btt-make-device-devno-use-PATH_MAX-to-avoid-overflow.patch
BLKTRACE_IGNORE_CVES += CVE-2018-10689

define BLKTRACE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) $(TARGET_CONFIGURE_OPTS)
endef

define BLKTRACE_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) $(TARGET_CONFIGURE_OPTS) install \
		DESTDIR=$(TARGET_DIR) prefix=/usr
endef

$(eval $(generic-package))
