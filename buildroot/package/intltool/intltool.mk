################################################################################
#
# intltool
#
################################################################################

INTLTOOL_VERSION = 0.51.0
INTLTOOL_SITE = https://launchpad.net/intltool/trunk/$(INTLTOOL_VERSION)/+download
INTLTOOL_LICENSE = GPL-2.0+
INTLTOOL_LICENSE_FILES = COPYING

HOST_INTLTOOL_DEPENDENCIES = host-gettext host-libxml-parser-perl

$(eval $(host-autotools-package))
