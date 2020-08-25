################################################################################
#
# mpdecimal
#
################################################################################

MPDECIMAL_SITE = http://www.bytereef.org/software/mpdecimal/releases
MPDECIMAL_VERSION = 2.4.2
MPDECIMAL_INSTALL_STAGING = YES
MPDECIMAL_LICENSE = BSD-2-Clause
MPDECIMAL_LICENSE_FILES = LICENSE.txt
MPDECIMAL_CONF_OPTS = LD="$(TARGET_CC)"
MPDECIMAL_AUTORECONF = YES

# On i386, by default, mpdecimal tries to uses <fenv.h> which is not
# available in musl/uclibc. So in this case, we tell mpdecimal to use
# the generic 32 bits code, which is anyway the one used on ARM,
# PowerPC, etc.
ifeq ($(BR2_TOOLCHAIN_USES_GLIBC),)
ifeq ($(BR2_i386),y)
MPDECIMAL_CONF_ENV += MACHINE=ansi32
endif
endif

$(eval $(autotools-package))
