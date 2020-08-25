################################################################################
#
# Build the yaffs2 root filesystem image
#
################################################################################

ROOTFS_YAFFS2_DEPENDENCIES = host-yaffs2utils

define ROOTFS_YAFFS2_CMD
	$(HOST_DIR)/bin/mkyaffs2 --all-root $(TARGET_DIR) $@
endef

$(eval $(rootfs))
