################################################################################
#
# json-c
#
################################################################################

JSON_C_VERSION = 0.14
JSON_C_SITE = https://s3.amazonaws.com/json-c_releases/releases
JSON_C_INSTALL_STAGING = YES
JSON_C_LICENSE = MIT
JSON_C_LICENSE_FILES = COPYING

$(eval $(cmake-package))
$(eval $(host-cmake-package))
