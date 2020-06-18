################################################################################
#
# rs485conf
#
################################################################################

RS485CONF_VERSION = 5c8d00cf70950fab3454549b81dea843d844492a
RS485CONF_SITE = $(call github,mniestroj,rs485conf,$(RS485CONF_VERSION))
RS485CONF_LICENSE = BSD-3-Clause
RS485CONF_LICENSE_FILES = LICENSE

define RS485CONF_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define RS485CONF_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) PREFIX="$(TARGET_DIR)/usr" -C $(@D) install
endef

$(eval $(generic-package))
