################################################################################
#
# squashfs
#
################################################################################

SQUASHFS_VERSION = 4.3
SQUASHFS_SOURCE = squashfs$(SQUASHFS_VERSION).tar.gz
SQUASHFS_SITE = http://downloads.sourceforge.net/project/squashfs/squashfs/squashfs$(SQUASHFS_VERSION)
SQUASHFS_LICENSE = GPLv2+
SQUASHFS_LICENSE_FILES = COPYING

# no libattr in BR
SQUASHFS_MAKE_ARGS = XATTR_SUPPORT=0

ifeq ($(BR2_PACKAGE_SQUASHFS_LZ4),y)
SQUASHFS_DEPENDENCIES += lz4
SQUASHFS_MAKE_ARGS += LZ4_SUPPORT=1 COMP_DEFAULT=lz4
else
SQUASHFS_MAKE_ARGS += LZ4_SUPPORT=0
endif

ifeq ($(BR2_PACKAGE_SQUASHFS_LZMA),y)
SQUASHFS_DEPENDENCIES += xz
SQUASHFS_MAKE_ARGS += LZMA_XZ_SUPPORT=1 COMP_DEFAULT=lzma
else
SQUASHFS_MAKE_ARGS += LZMA_XZ_SUPPORT=0
endif

ifeq ($(BR2_PACKAGE_SQUASHFS_XZ),y)
SQUASHFS_DEPENDENCIES += xz
SQUASHFS_MAKE_ARGS += XZ_SUPPORT=1 COMP_DEFAULT=xz
else
SQUASHFS_MAKE_ARGS += XZ_SUPPORT=0
endif

ifeq ($(BR2_PACKAGE_SQUASHFS_LZO),y)
SQUASHFS_DEPENDENCIES += lzo
SQUASHFS_MAKE_ARGS += LZO_SUPPORT=1 COMP_DEFAULT=lzo
else
SQUASHFS_MAKE_ARGS += LZO_SUPPORT=0
endif

ifeq ($(BR2_PACKAGE_SQUASHFS_GZIP),y)
SQUASHFS_DEPENDENCIES += zlib
SQUASHFS_MAKE_ARGS += GZIP_SUPPORT=1 COMP_DEFAULT=gzip
else
SQUASHFS_MAKE_ARGS += GZIP_SUPPORT=0
endif

HOST_SQUASHFS_DEPENDENCIES = host-zlib host-lz4 host-lzo host-xz

# no libattr/xz in BR
HOST_SQUASHFS_MAKE_ARGS = \
	XATTR_SUPPORT=0 \
	XZ_SUPPORT=1    \
	GZIP_SUPPORT=1  \
	LZ4_SUPPORT=1	\
	LZO_SUPPORT=1	\
	LZMA_XZ_SUPPORT=1

define SQUASHFS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE)    \
		CC="$(TARGET_CC)"           \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)"   \
		EXTRA_LDFLAGS="$(TARGET_LDFLAGS)" \
		$(SQUASHFS_MAKE_ARGS) \
		-C $(@D)/squashfs-tools/
endef

define SQUASHFS_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(SQUASHFS_MAKE_ARGS) \
		-C $(@D)/squashfs-tools/ INSTALL_DIR=$(TARGET_DIR)/usr/bin install
endef

define HOST_SQUASHFS_BUILD_CMDS
	$(HOST_MAKE_ENV) $(MAKE) \
		CC="$(HOSTCC)" \
		EXTRA_CFLAGS="$(HOST_CFLAGS)"   \
		EXTRA_LDFLAGS="$(HOST_LDFLAGS)" \
		$(HOST_SQUASHFS_MAKE_ARGS) \
		-C $(@D)/squashfs-tools/
endef

define HOST_SQUASHFS_INSTALL_CMDS
	$(HOST_MAKE_ENV) $(MAKE) $(HOST_SQUASHFS_MAKE_ARGS) \
		-C $(@D)/squashfs-tools/ INSTALL_DIR=$(HOST_DIR)/usr/bin install
endef

$(eval $(generic-package))
$(eval $(host-generic-package))
