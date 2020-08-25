################################################################################
#
# qjson
#
################################################################################

QJSON_VERSION = 0.9.0
QJSON_SITE = $(call github,flavio,qjson,$(QJSON_VERSION))
QJSON_INSTALL_STAGING = YES
QJSON_DEPENDENCIES = qt5base
QJSON_LICENSE = LGPL-2.1
QJSON_LICENSE_FILES = COPYING.lib

$(eval $(cmake-package))
