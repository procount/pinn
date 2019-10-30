################################################################################
#
# busyboc - Installs the precompiled package
#
################################################################################

BUSYBOC_VERSION = 1.0
BUSYBOC_SITE = $(TARGET_DIR)/../../package/busyboc
BUSYBOC_SITE_METHOD = local
BUSYBOC_LICENSE = Proprietary
#BUSYBOC_LICENSE_FILES = 

define BUSYBOC_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 04755 $(@D)/busyboc $(TARGET_DIR)/bin/busyboc
	ln -sf ../../bin/busyboc $(TARGET_DIR)/usr/bin/tee
endef

$(eval $(generic-package))

#define SQUASHFS_BUILD_CMDS
#        $(TARGET_MAKE_ENV) $(MAKE)    \
#                CC="$(TARGET_CC)"           \
#                EXTRA_CFLAGS="$(TARGET_CFLAGS)"   \
#                EXTRA_LDFLAGS="$(TARGET_LDFLAGS)" \
#                $(SQUASHFS_MAKE_ARGS) \
#                -C $(@D)/squashfs-tools/
#endef
#
#define SQUASHFS_INSTALL_TARGET_CMDS
#        $(TARGET_MAKE_ENV) $(MAKE) $(SQUASHFS_MAKE_ARGS) \
#                -C $(@D)/squashfs-tools/ INSTALL_DIR=$(TARGET_DIR)/usr/bin install
#endef
#
#define HOST_SQUASHFS_BUILD_CMDS
#        $(HOST_MAKE_ENV) $(MAKE) \
#                CC="$(HOSTCC)" \
#                EXTRA_CFLAGS="$(HOST_CFLAGS)"   \
#                EXTRA_LDFLAGS="$(HOST_LDFLAGS)" \
#                $(HOST_SQUASHFS_MAKE_ARGS) \
#                -C $(@D)/squashfs-tools/
#endef
#
#define HOST_SQUASHFS_INSTALL_CMDS
#        $(HOST_MAKE_ENV) $(MAKE) $(HOST_SQUASHFS_MAKE_ARGS) \
#                -C $(@D)/squashfs-tools/ INSTALL_DIR=$(HOST_DIR)/usr/bin install
#endef

