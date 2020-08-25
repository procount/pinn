################################################################################
#
# mosh
#
################################################################################

MOSH_VERSION = 1.3.2
MOSH_SITE = https://mosh.org
MOSH_DEPENDENCIES = zlib ncurses protobuf host-pkgconf
MOSH_LICENSE = GPL-3.0+ with exception
MOSH_LICENSE_FILES = COPYING COPYING.iOS

# protobuf needs c++11 (since 3.6.0)
MOSH_CONF_ENV += CXXFLAGS="$(TARGET_CXXFLAGS) -std=c++11"

ifeq ($(BR2_PACKAGE_OPENSSL),y)
MOSH_CONF_OPTS += --with-crypto-library=openssl
MOSH_DEPENDENCIES += openssl
else
MOSH_CONF_OPTS += --with-crypto-library=nettle
MOSH_DEPENDENCIES += nettle
endif

# help the detection of the SSP support: mosh configure.ac doesn't do
# a link test, so it doesn't detect when the toolchain doesn't have
# libssp.
ifeq ($(BR2_TOOLCHAIN_HAS_SSP),)
MOSH_CONF_ENV += \
	ax_cv_check_cflags__Werror___fstack_protector_all=no \
	ax_cv_check_cxxflags__Werror___fstack_protector_all=no
endif

$(eval $(autotools-package))
