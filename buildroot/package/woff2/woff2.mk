################################################################################
#
# woff2
#
################################################################################

WOFF2_VERSION = 1.0.2
WOFF2_SOURCE = v$(WOFF2_VERSION).tar.gz
WOFF2_SITE = https://github.com/google/woff2/archive
WOFF2_LICENSE = MIT
WOFF2_LICENSE_FILES = LICENSE
WOFF2_INSTALL_STAGING = YES
WOFF2_DEPENDENCIES = brotli
WOFF2_CONF_OPTS = \
	-DNOISY_LOGGING=OFF

# The CMake build files for woff2 manually set some RPATH handling options
# which make the installation steps fail with static builds, so pass this
# to prevent any attempt of mangling RPATH that CMake would do.
ifneq ($(BR2_SHARED_LIBS),y)
WOFF2_CONF_OPTS += -DCMAKE_SKIP_RPATH=ON
endif

$(eval $(cmake-package))
