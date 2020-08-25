################################################################################
#
# uboot-tools
#
################################################################################

UBOOT_TOOLS_VERSION = 2020.04
UBOOT_TOOLS_SOURCE = u-boot-$(UBOOT_TOOLS_VERSION).tar.bz2
UBOOT_TOOLS_SITE = ftp://ftp.denx.de/pub/u-boot
UBOOT_TOOLS_LICENSE = GPL-2.0+
UBOOT_TOOLS_LICENSE_FILES = Licenses/gpl-2.0.txt
UBOOT_TOOLS_INSTALL_STAGING = YES

# u-boot 2020.01+ needs make 4.0+
UBOOT_TOOLS_DEPENDENCIES = $(BR2_MAKE_HOST_DEPENDENCY)
HOST_UBOOT_TOOLS_DEPENDENCIES = $(BR2_MAKE_HOST_DEPENDENCY)

define UBOOT_TOOLS_CONFIGURE_CMDS
	mkdir -p $(@D)/include/config
	touch $(@D)/include/config/auto.conf
endef

UBOOT_TOOLS_MAKE_OPTS = CROSS_COMPILE="$(TARGET_CROSS)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	HOSTCFLAGS="$(HOST_CFLAGS)" \
	STRIP=$(TARGET_STRIP)

ifeq ($(BR2_PACKAGE_UBOOT_TOOLS_FIT_SUPPORT),y)
UBOOT_TOOLS_MAKE_OPTS += CONFIG_FIT=y CONFIG_MKIMAGE_DTC_PATH=dtc
UBOOT_TOOLS_DEPENDENCIES += dtc
endif

ifeq ($(BR2_PACKAGE_UBOOT_TOOLS_FIT_SIGNATURE_SUPPORT),y)
UBOOT_TOOLS_MAKE_OPTS += CONFIG_FIT_SIGNATURE=y CONFIG_FIT_SIGNATURE_MAX_SIZE=0x10000000
UBOOT_TOOLS_DEPENDENCIES += openssl host-pkgconf
endif

ifeq ($(BR2_PACKAGE_UBOOT_TOOLS_FIT_CHECK_SIGN),y)
define UBOOT_TOOLS_INSTALL_FIT_CHECK_SIGN
	$(INSTALL) -m 0755 -D $(@D)/tools/fit_check_sign $(TARGET_DIR)/usr/bin/fit_check_sign
endef
endif

define UBOOT_TOOLS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(BR2_MAKE) -C $(@D) $(UBOOT_TOOLS_MAKE_OPTS) \
		CROSS_BUILD_TOOLS=y tools-only
	$(TARGET_MAKE_ENV) $(BR2_MAKE) -C $(@D) $(UBOOT_TOOLS_MAKE_OPTS) \
		envtools no-dot-config-targets=envtools
endef

ifeq ($(BR2_PACKAGE_UBOOT_TOOLS_MKIMAGE),y)
define UBOOT_TOOLS_INSTALL_MKIMAGE
	$(INSTALL) -m 0755 -D $(@D)/tools/mkimage $(TARGET_DIR)/usr/bin/mkimage
endef
endif

ifeq ($(BR2_PACKAGE_UBOOT_TOOLS_MKENVIMAGE),y)
define UBOOT_TOOLS_INSTALL_MKENVIMAGE
	$(INSTALL) -m 0755 -D $(@D)/tools/mkenvimage $(TARGET_DIR)/usr/bin/mkenvimage
endef
endif

ifeq ($(BR2_PACKAGE_UBOOT_TOOLS_FWPRINTENV),y)
define UBOOT_TOOLS_INSTALL_FWPRINTENV
	$(INSTALL) -m 0755 -D $(@D)/tools/env/fw_printenv $(TARGET_DIR)/usr/sbin/fw_printenv
	ln -sf fw_printenv $(TARGET_DIR)/usr/sbin/fw_setenv
endef
endif

ifeq ($(BR2_PACKAGE_UBOOT_TOOLS_DUMPIMAGE),y)
define UBOOT_TOOLS_INSTALL_DUMPIMAGE
	$(INSTALL) -m 0755 -D $(@D)/tools/dumpimage $(TARGET_DIR)/usr/sbin/dumpimage
endef
endif

define UBOOT_TOOLS_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0755 $(@D)/tools/env/lib.a $(STAGING_DIR)/usr/lib/libubootenv.a
	$(INSTALL) -D -m 0644 $(@D)/tools/env/fw_env.h $(STAGING_DIR)/usr/include/fw_env.h
endef

define UBOOT_TOOLS_INSTALL_TARGET_CMDS
	$(UBOOT_TOOLS_INSTALL_MKIMAGE)
	$(UBOOT_TOOLS_INSTALL_MKENVIMAGE)
	$(UBOOT_TOOLS_INSTALL_FWPRINTENV)
	$(UBOOT_TOOLS_INSTALL_DUMPIMAGE)
	$(UBOOT_TOOLS_INSTALL_FIT_CHECK_SIGN)
endef

define HOST_UBOOT_TOOLS_CONFIGURE_CMDS
	mkdir -p $(@D)/include/config
	touch $(@D)/include/config/auto.conf
endef

HOST_UBOOT_TOOLS_MAKE_OPTS = HOSTCC="$(HOSTCC)" \
	HOSTCFLAGS="$(HOST_CFLAGS)" \
	HOSTLDFLAGS="$(HOST_LDFLAGS)"

ifeq ($(BR2_PACKAGE_HOST_UBOOT_TOOLS_FIT_SUPPORT),y)
HOST_UBOOT_TOOLS_MAKE_OPTS += CONFIG_FIT=y CONFIG_MKIMAGE_DTC_PATH=dtc
HOST_UBOOT_TOOLS_DEPENDENCIES += host-dtc
endif

ifeq ($(BR2_PACKAGE_HOST_UBOOT_TOOLS_FIT_SIGNATURE_SUPPORT),y)
HOST_UBOOT_TOOLS_MAKE_OPTS += CONFIG_FIT_SIGNATURE=y CONFIG_FIT_SIGNATURE_MAX_SIZE=0x10000000
HOST_UBOOT_TOOLS_DEPENDENCIES += host-openssl
endif

define HOST_UBOOT_TOOLS_BUILD_CMDS
	$(BR2_MAKE1) -C $(@D) $(HOST_UBOOT_TOOLS_MAKE_OPTS) tools-only
endef

define HOST_UBOOT_TOOLS_INSTALL_CMDS
	$(INSTALL) -m 0755 -D $(@D)/tools/mkimage $(HOST_DIR)/bin/mkimage
	$(INSTALL) -m 0755 -D $(@D)/tools/mkenvimage $(HOST_DIR)/bin/mkenvimage
	$(INSTALL) -m 0755 -D $(@D)/tools/dumpimage $(HOST_DIR)/bin/dumpimage
endef

$(eval $(generic-package))
$(eval $(host-generic-package))

# Convenience variables for other mk files that make use of mkimage

MKIMAGE = $(HOST_DIR)/bin/mkimage

# mkimage supports arm blackfin m68k microblaze mips mips64 nios2 powerpc ppc sh sparc sparc64 x86
# KERNEL_ARCH can be arm64 arc arm blackfin m68k microblaze mips nios2 powerpc sh sparc i386 x86_64 xtensa
# For arm64, arc, xtensa we'll just keep KERNEL_ARCH
# For mips64, we'll just keep mips
# For i386 and x86_64, we need to convert
ifeq ($(KERNEL_ARCH),x86_64)
MKIMAGE_ARCH = x86
else ifeq ($(KERNEL_ARCH),i386)
MKIMAGE_ARCH = x86
else
MKIMAGE_ARCH = $(KERNEL_ARCH)
endif
