################################################################################
#
# ctorrent
#
################################################################################

CTORRENT_VERSION = dnh3.3.2
CTORRENT_SITE = http://www.rahul.net/dholmes/ctorrent
CTORRENT_LICENSE = GPL-2.0
CTORRENT_LICENSE_FILES = COPYING

ifeq ($(BR2_PACKAGE_OPENSSL),y)
CTORRENT_CONF_OPTS += --with-ssl=yes
CTORRENT_DEPENDENCIES += openssl
else
CTORRENT_CONF_OPTS += --with-ssl=no
endif

$(eval $(autotools-package))
