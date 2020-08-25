################################################################################
#
# exiv2
#
################################################################################

EXIV2_VERSION = 0.27.3
EXIV2_SITE = $(call github,Exiv2,exiv2,v$(EXIV2_VERSION))
EXIV2_INSTALL_STAGING = YES
EXIV2_LICENSE = GPL-2.0+
EXIV2_LICENSE_FILES = COPYING

EXIV2_CONF_OPTS += -DEXIV2_BUILD_SAMPLES=OFF

# The following CMake variable disables a TRY_RUN call in the -pthread
# test which is not allowed when cross-compiling.
EXIV2_CONF_OPTS += -DTHREADS_PTHREAD_ARG=OFF

ifeq ($(BR2_PACKAGE_EXIV2_LENSDATA),y)
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_LENSDATA=ON
else
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_LENSDATA=OFF
endif

ifeq ($(BR2_PACKAGE_EXIV2_PNG),y)
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_PNG=ON
EXIV2_DEPENDENCIES += zlib
else
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_PNG=OFF
endif

ifeq ($(BR2_PACKAGE_EXIV2_XMP),y)
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_XMP=ON
EXIV2_DEPENDENCIES += expat
else
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_XMP=OFF
endif

EXIV2_DEPENDENCIES += $(TARGET_NLS_DEPENDENCIES)

ifeq ($(BR2_SYSTEM_ENABLE_NLS),y)
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_NLS=ON
else
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_NLS=OFF
endif

$(eval $(cmake-package))
