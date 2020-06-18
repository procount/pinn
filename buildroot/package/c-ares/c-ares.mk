################################################################################
#
# c-ares
#
################################################################################

C_ARES_VERSION = 1.15.0
C_ARES_SITE = http://c-ares.haxx.se/download
C_ARES_INSTALL_STAGING = YES
C_ARES_CONF_OPTS = --with-random=/dev/urandom
# Rebuild configure to avoid XC_CHECK_USER_CFLAGS
C_ARES_AUTORECONF = YES
C_ARES_LICENSE = MIT
C_ARES_LICENSE_FILES = LICENSE.md

$(eval $(autotools-package))
$(eval $(host-autotools-package))
