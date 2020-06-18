################################################################################
#
# lzop
#
################################################################################

LZOP_VERSION = 1.03
LZOP_SITE = http://www.lzop.org/download
LZOP_LICENSE = GPL-2.0+
LZOP_LICENSE_FILES = COPYING
LZOP_DEPENDENCIES = lzo
HOST_LZOP_DEPENDENCIES = host-lzo

$(eval $(autotools-package))
$(eval $(host-autotools-package))

LZOP = $(HOST_DIR)/bin/lzop
