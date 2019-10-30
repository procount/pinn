#############################################################
#
# buysboc-dev package
#
#############################################################

BUSYBOCDEV_VERSION = 1.0
BUSYBOCDEV_SITE = $(TOPDIR)/../busybocdev
BUSYBOCDEV_SITE_METHOD = local
BUSYBOCDEV_LICENSE = BSD-3c
BUSYBOCDEV_LICENSE_FILES = LICENSE.txt
BUSYBOCDEV_INSTALL_STAGING = NO

define BUSYBOCDEV_BUILD_CMDS
	cd $(@D) 
        $(TARGET_MAKE_ENV) $(MAKE)    \
                CC="$(TARGET_CC)"           \
                EXTRA_CFLAGS="$(TARGET_CFLAGS)"   \
                EXTRA_LDFLAGS="$(TARGET_LDFLAGS)" \
		-C $(@D) all \
		$(TARGET_STRIP) $(@D)/busyboc

endef

define BUSYBOCDEV_INSTALL_TARGET_CMDS
	$(INSTALL) -m 04755 $(@D)/busyboc $(TARGET_DIR)/bin/busyboc
	ln -sf ../../bin/busyboc $(TARGET_DIR)/usr/bin/tee
endef


define HOST_BUSYBOCDEV_BUILD_CMDS
        $(HOST_MAKE_ENV) $(MAKE) \
                CC="$(HOSTCC)" \
                EXTRA_CFLAGS="$(HOST_CFLAGS)"   \
                EXTRA_LDFLAGS="$(HOST_LDFLAGS)" \
                -C $(@D) all \
		$(TARGET_STRIP) $(@D)/busyboc
endef

define HOST_BUSYBOCDEV_INSTALL_CMDS
	$(INSTALL) -m 04755 $(@D)/busyboc $(HOST_DIR)/usr/bin/busyboc
endef


$(eval $(generic-package))
$(eval $(host-generic-package))



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

