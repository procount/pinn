################################################################################
#
# opencore-amr
#
################################################################################

OPENCORE_AMR_VERSION = 0.1.5
OPENCORE_AMR_SITE = http://downloads.sourceforge.net/project/opencore-amr/opencore-amr
OPENCORE_AMR_INSTALL_STAGING = YES
OPENCORE_AMR_LICENSE = Apache-2.0
OPENCORE_AMR_LICENSE_FILES = LICENSE

$(eval $(autotools-package))
