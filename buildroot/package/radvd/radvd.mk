################################################################################
#
# radvd
#
################################################################################

RADVD_VERSION = 2.18
RADVD_SITE = http://www.litech.org/radvd/dist
RADVD_DEPENDENCIES = host-bison host-flex host-pkgconf
# We need to ignore <linux/if_arp.h>, because radvd already includes
# <net/if_arp.h>, which conflicts with <linux/if_arp.h>.
RADVD_CONF_ENV = \
	ac_cv_prog_cc_c99='-std=gnu99' \
	ac_cv_header_linux_if_arp_h=no
# For 0002-Don-t-force-fstack-protector-the-toolchain-might-lac.patch
RADVD_AUTORECONF = YES
RADVD_LICENSE = BSD-4-Clause-like
RADVD_LICENSE_FILES = COPYRIGHT

# We don't provide /etc/radvd.conf, so disable the service by default.
define RADVD_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 package/radvd/50-radvd.preset \
		$(TARGET_DIR)/usr/lib/systemd/system-preset/50-radvd.preset
endef

define RADVD_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 package/radvd/S50radvd $(TARGET_DIR)/etc/init.d/S50radvd
endef

$(eval $(autotools-package))
