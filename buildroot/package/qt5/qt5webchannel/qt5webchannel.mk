################################################################################
#
# qt5webchannel
#
################################################################################

QT5WEBCHANNEL_VERSION = $(QT5_VERSION)
QT5WEBCHANNEL_SITE = $(QT5_SITE)
QT5WEBCHANNEL_SOURCE = qtwebchannel-$(QT5_SOURCE_TARBALL_PREFIX)-$(QT5WEBCHANNEL_VERSION).tar.xz
QT5WEBCHANNEL_DEPENDENCIES = qt5base qt5websockets
QT5WEBCHANNEL_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
QT5WEBCHANNEL_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
QT5WEBCHANNEL_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
QT5WEBCHANNEL_LICENSE = GPL-2.0 or GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
QT5WEBCHANNEL_LICENSE_FILES = LICENSE.GPLv2 LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif
ifeq ($(BR2_PACKAGE_QT5BASE_EXAMPLES),y)
QT5WEBCHANNEL_LICENSE += , BSD-3-Clause (examples)
endif

ifeq ($(BR2_PACKAGE_QT5DECLARATIVE),y)
QT5WEBCHANNEL_DEPENDENCIES += qt5declarative
endif

define QT5WEBCHANNEL_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define QT5WEBCHANNEL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QT5WEBCHANNEL_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install
endef

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
define QT5WEBCHANNEL_INSTALL_TARGET_JAVASCRIPT
	$(INSTALL) -m 0644 -D $(@D)/examples/webchannel/shared/qwebchannel.js \
		$(TARGET_DIR)/var/www/qwebchannel.js
endef
else
define QT5WEBCHANNEL_INSTALL_TARGET_JAVASCRIPT
	$(INSTALL) -m 0644 -D $(@D)/src/webchannel/qwebchannel.js \
		$(TARGET_DIR)/var/www/qwebchannel.js
endef
endif

ifeq ($(BR2_PACKAGE_QT5DECLARATIVE_QUICK),y)
define QT5WEBCHANNEL_INSTALL_TARGET_QMLS
	cp -dpfr $(STAGING_DIR)/usr/qml/QtWebChannel $(TARGET_DIR)/usr/qml/
endef
endif

ifeq ($(BR2_PACKAGE_QT5BASE_EXAMPLES),y)
define QT5WEBCHANNEL_INSTALL_TARGET_EXAMPLES
	cp -dpfr $(STAGING_DIR)/usr/lib/qt/examples/webchannel $(TARGET_DIR)/usr/lib/qt/examples/
endef
endif

ifneq ($(BR2_STATIC_LIBS),y)
define QT5WEBCHANNEL_INSTALL_TARGET_LIBS
	cp -dpf $(STAGING_DIR)/usr/lib/libQt5WebChannel.so.* $(TARGET_DIR)/usr/lib
endef
endif

define QT5WEBCHANNEL_INSTALL_TARGET_CMDS
	$(QT5WEBCHANNEL_INSTALL_TARGET_LIBS)
	$(QT5WEBCHANNEL_INSTALL_TARGET_QMLS)
	$(QT5WEBCHANNEL_INSTALL_TARGET_JAVASCRIPT)
	$(QT5WEBCHANNEL_INSTALL_TARGET_EXAMPLES)
endef

$(eval $(generic-package))
