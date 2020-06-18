################################################################################
#
# gummiboot
#
################################################################################

GUMMIBOOT_SITE = https://gitlab.freedesktop.org/archived-projects/gummiboot.git
GUMMIBOOT_SITE_METHOD = git
GUMMIBOOT_VERSION = 2bcd919c681c952eb867ef1bdb458f1bc49c2d55
GUMMIBOOT_LICENSE = LGPL-2.1+
GUMMIBOOT_LICENSE_FILES = LICENSE

# The git archive does not have the autoconf/automake stuff generated.
GUMMIBOOT_AUTORECONF = YES
GUMMIBOOT_DEPENDENCIES = gnu-efi host-pkgconf util-linux
GUMMIBOOT_INSTALL_TARGET = NO
GUMMIBOOT_INSTALL_IMAGES = YES

ifeq ($(BR2_i386),y)
GUMMIBOOT_IMGARCH = ia32
else ifeq ($(BR2_x86_64),y)
GUMMIBOOT_IMGARCH = x64
endif

GUMMIBOOT_CONF_OPTS = \
	--host=$(BR2_ARCH) \
	--with-efi-libdir=$(STAGING_DIR)/usr/lib \
	--with-efi-ldsdir=$(STAGING_DIR)/usr/lib \
	--with-efi-includedir=$(STAGING_DIR)/usr/include \
	--disable-manpages

define GUMMIBOOT_INSTALL_IMAGES_CMDS
	$(INSTALL) -D -m 0644 $(@D)/gummiboot$(GUMMIBOOT_IMGARCH).efi \
		$(BINARIES_DIR)/efi-part/EFI/BOOT/boot$(GUMMIBOOT_IMGARCH).efi
	echo "boot$(GUMMIBOOT_IMGARCH).efi" > \
		$(BINARIES_DIR)/efi-part/startup.nsh
	$(INSTALL) -D -m 0644 boot/gummiboot/loader.conf \
		$(BINARIES_DIR)/efi-part/loader/loader.conf
	$(INSTALL) -D -m 0644 boot/gummiboot/buildroot.conf \
		$(BINARIES_DIR)/efi-part/loader/entries/buildroot.conf
endef

$(eval $(autotools-package))
