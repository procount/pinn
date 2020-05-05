#############################################################
#
# SCSI header fix
#
#############################################################


SCSIFIX_VERSION = 3.3.4
SCSIFIX_SITE = package/scsifix/data
SCSIFIX_SITE_METHOD = local
SCSIFIX_LICENSE = GPLv2.1
SCSIFIX_INSTALL_STAGING = YES

define SCSIFIX_BUILD_CMDS
endef

define SCSIFIX_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/scsi.h $(TARGET_DIR)/usr/include/scsi.h
	$(INSTALL) -m 0755 $(@D)/scsi_ioctl.h $(TARGET_DIR)/usr/include/scsi_ioctl.h
	$(INSTALL) -m 0755 $(@D)/sg.h $(TARGET_DIR)/usr/include/sg.h
endef

define SCSIFIX_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0755 $(@D)/scsi.h $(STAGING_DIR)/usr/include/scsi.h
	$(INSTALL) -m 0755 $(@D)/scsi_ioctl.h $(STAGING_DIR)/usr/include/scsi_ioctl.h
	$(INSTALL) -m 0755 $(@D)/sg.h $(STAGING_DIR)/usr/include/sg.h
endef


$(eval $(generic-package))
