################################################################################
#
# Build the iso96600 root filesystem image
#
################################################################################

#
# We need to handle three cases:
#
#  1. The ISO9660 filesystem will really be the real root filesystem
#     itself. This is when BR2_TARGET_ROOTFS_ISO9660_INITRD is
#     disabled.
#
#  2. The ISO9660 filesystem will be a filesystem with just a kernel
#     image, initrd and grub. This is when
#     BR2_TARGET_ROOTFS_ISO9660_INITRD is enabled, but
#     BR2_TARGET_ROOTFS_INITRAMFS is disabled.
#
#  3. The ISO9660 filesystem will be a filesystem with just a kernel
#     image and grub. This is like (2), except that the initrd is
#     built into the kernel image. This is when
#     BR2_TARGET_ROOTFS_INITRAMFS is enabled (regardless of the value
#     of BR2_TARGET_ROOTFS_ISO9660_INITRD).

ROOTFS_ISO9660_BOOT_MENU = $(call qstrip,$(BR2_TARGET_ROOTFS_ISO9660_BOOT_MENU))

ROOTFS_ISO9660_DEPENDENCIES = host-cdrkit linux

ifeq ($(BR2_TARGET_ROOTFS_INITRAMFS),y)
ROOTFS_ISO9660_USE_INITRD = YES
endif

ifeq ($(BR2_TARGET_ROOTFS_ISO9660_INITRD),y)
ROOTFS_ISO9660_USE_INITRD = YES
endif

ifeq ($(ROOTFS_ISO9660_USE_INITRD),YES)
ROOTFS_ISO9660_TMP_TARGET_DIR = $(FS_DIR)/rootfs.iso9660.tmp
define ROOTFS_ISO9660_CREATE_TEMPDIR
	$(RM) -rf $(ROOTFS_ISO9660_TMP_TARGET_DIR)
	mkdir -p $(ROOTFS_ISO9660_TMP_TARGET_DIR)
endef
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_CREATE_TEMPDIR
else ifeq ($(BR2_TARGET_ROOTFS_ISO9660_TRANSPARENT_COMPRESSION),y)
ROOTFS_ISO9660_DEPENDENCIES += host-zisofs-tools
ROOTFS_ISO9660_TMP_TARGET_DIR = $(FS_DIR)/rootfs.iso9660.tmp
# This must be early, before we copy the bootloader files.
define ROOTFS_ISO9660_MKZFTREE
	$(RM) -rf $(ROOTFS_ISO9660_TMP_TARGET_DIR)
	$(HOST_DIR)/bin/mkzftree -X -z 9 -p $(PARALLEL_JOBS) \
		$(TARGET_DIR) \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)
endef
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_MKZFTREE
ROOTFS_ISO9660_GENISOIMAGE_OPTS += -z
else
ROOTFS_ISO9660_TMP_TARGET_DIR = $(TARGET_DIR)
endif

ifeq ($(BR2_TARGET_ROOTFS_ISO9660_GRUB2),y)
ROOTFS_ISO9660_DEPENDENCIES += grub2
ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH = \
	$(ROOTFS_ISO9660_TMP_TARGET_DIR)/boot/grub/grub.cfg
ROOTFS_ISO9660_BOOT_IMAGE = boot/grub/grub-eltorito.img
define ROOTFS_ISO9660_INSTALL_BOOTLOADER
	$(INSTALL) -D -m 0644 $(BINARIES_DIR)/grub-eltorito.img \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/boot/grub/grub-eltorito.img
endef
else ifeq ($(BR2_TARGET_ROOTFS_ISO9660_ISOLINUX),y)
ROOTFS_ISO9660_DEPENDENCIES += syslinux
ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH = \
	$(ROOTFS_ISO9660_TMP_TARGET_DIR)/isolinux/isolinux.cfg
ROOTFS_ISO9660_BOOT_IMAGE = isolinux/isolinux.bin
define ROOTFS_ISO9660_INSTALL_BOOTLOADER
	$(INSTALL) -D -m 0644 $(BINARIES_DIR)/syslinux/* \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/isolinux/
	$(INSTALL) -D -m 0644 $(HOST_DIR)/share/syslinux/ldlinux.c32 \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/isolinux/ldlinux.c32
endef
endif

define ROOTFS_ISO9660_PREPARATION
	$(INSTALL) -D -m 0644 $(ROOTFS_ISO9660_BOOT_MENU) \
		$(ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH)
	$(SED) "s%__KERNEL_PATH__%/boot/$(LINUX_IMAGE_NAME)%" \
		$(ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH)
	$(ROOTFS_ISO9660_INSTALL_BOOTLOADER)
endef

ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_PREPARATION

define ROOTFS_ISO9660_DISABLE_EXTERNAL_INITRD
	$(SED) '/__INITRD_PATH__/d'  $(ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH)
endef

# Copy the kernel to temporary filesystem
define ROOTFS_ISO9660_COPY_KERNEL
	$(INSTALL) -D -m 0644 $(LINUX_IMAGE_PATH) \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/boot/$(LINUX_IMAGE_NAME)
endef

ifeq ($(ROOTFS_ISO9660_USE_INITRD),YES)
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_COPY_KERNEL

# If initramfs is used, disable loading the initrd as the rootfs is
# already inside the kernel image. Otherwise, make sure a cpio is
# generated and use it as the initrd.
ifeq ($(BR2_TARGET_ROOTFS_INITRAMFS),y)
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_DISABLE_EXTERNAL_INITRD
else
ROOTFS_ISO9660_DEPENDENCIES += rootfs-cpio
define ROOTFS_ISO9660_COPY_INITRD
	$(INSTALL) -D -m 0644 $(BINARIES_DIR)/rootfs.cpio$(ROOTFS_CPIO_COMPRESS_EXT) \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/boot/initrd
	$(SED) "s%__INITRD_PATH__%/boot/initrd%" \
		$(ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH)
endef
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_COPY_INITRD
endif

else # ROOTFS_ISO9660_USE_INITRD
ifeq ($(BR2_TARGET_ROOTFS_ISO9660_TRANSPARENT_COMPRESSION),y)
# We must use the uncompressed kernel image
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_COPY_KERNEL
endif

ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_DISABLE_EXTERNAL_INITRD

endif # ROOTFS_ISO9660_USE_INITRD

define ROOTFS_ISO9660_CMD
	$(HOST_DIR)/bin/genisoimage -J -R -b $(ROOTFS_ISO9660_BOOT_IMAGE) \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		$(ROOTFS_ISO9660_GENISOIMAGE_OPTS) \
		-o $@ $(ROOTFS_ISO9660_TMP_TARGET_DIR)
endef

ifeq ($(BR2_TARGET_ROOTFS_ISO9660_HYBRID),y)
define ROOTFS_ISO9660_GEN_HYBRID
	$(HOST_DIR)/bin/isohybrid -t 0x96 $@
endef

ROOTFS_ISO9660_POST_GEN_HOOKS += ROOTFS_ISO9660_GEN_HYBRID
endif

$(eval $(rootfs))
