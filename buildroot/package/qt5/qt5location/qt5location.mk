################################################################################
#
# qt5location
#
################################################################################

QT5LOCATION_VERSION = $(QT5_VERSION)
QT5LOCATION_SITE = $(QT5_SITE)
QT5LOCATION_SOURCE = qtlocation-$(QT5_SOURCE_TARBALL_PREFIX)-$(QT5LOCATION_VERSION).tar.xz
QT5LOCATION_DEPENDENCIES = qt5base
QT5LOCATION_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
QT5LOCATION_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
QT5LOCATION_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
QT5LOCATION_LICENSE = GPL-2.0 or GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
QT5LOCATION_LICENSE_FILES = LICENSE.GPLv2 LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

ifeq ($(BR2_PACKAGE_QT5DECLARATIVE),y)
QT5LOCATION_DEPENDENCIES += qt5declarative
endif

define QT5LOCATION_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define QT5LOCATION_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QT5LOCATION_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install
endef

ifeq ($(BR2_PACKAGE_QT5DECLARATIVE_QUICK),y)
define QT5LOCATION_INSTALL_TARGET_QMLS
	cp -dpfr $(STAGING_DIR)/usr/qml/QtPositioning $(TARGET_DIR)/usr/qml/
	cp -dpfr $(STAGING_DIR)/usr/qml/QtLocation $(TARGET_DIR)/usr/qml/
endef
define QT5LOCATION_INSTALL_TARGET_LOCATION
	cp -dpf $(STAGING_DIR)/usr/lib/libQt5Location.so.* $(TARGET_DIR)/usr/lib
	cp -dpfr $(STAGING_DIR)/usr/lib/qt/plugins/geoservices $(TARGET_DIR)/usr/lib/qt/plugins/
endef
ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
define QT5LOCATION_INSTALL_TARGET_POSITION_QUICK
	cp -dpf $(STAGING_DIR)/usr/lib/libQt5PositioningQuick.so.* $(TARGET_DIR)/usr/lib
endef
endif
ifeq ($(BR2_PACKAGE_QT5BASE_EXAMPLES),y)
define QT5LOCATION_INSTALL_TARGET_EXAMPLES
	cp -dpfr $(STAGING_DIR)/usr/lib/qt/examples/location $(TARGET_DIR)/usr/lib/qt/examples/
	cp -dpfr $(STAGING_DIR)/usr/lib/qt/examples/positioning $(TARGET_DIR)/usr/lib/qt/examples/
endef
endif
endif

define QT5LOCATION_INSTALL_TARGET_POSITION
	cp -dpf $(STAGING_DIR)/usr/lib/libQt5Positioning.so.* $(TARGET_DIR)/usr/lib
	cp -dpfr $(STAGING_DIR)/usr/lib/qt/plugins/position $(TARGET_DIR)/usr/lib/qt/plugins/
endef

define QT5LOCATION_INSTALL_TARGET_CMDS
	$(QT5LOCATION_INSTALL_TARGET_POSITION)
	$(QT5LOCATION_INSTALL_TARGET_POSITION_QUICK)
	$(QT5LOCATION_INSTALL_TARGET_LOCATION)
	$(QT5LOCATION_INSTALL_TARGET_QMLS)
	$(QT5LOCATION_INSTALL_TARGET_EXAMPLES)
endef

$(eval $(generic-package))
