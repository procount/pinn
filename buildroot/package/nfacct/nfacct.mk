################################################################################
#
# nfacct
#
################################################################################

NFACCT_VERSION = 1.0.2
NFACCT_SOURCE = nfacct-$(NFACCT_VERSION).tar.bz2
NFACCT_SITE = http://www.netfilter.org/projects/nfacct/files
NFACCT_DEPENDENCIES = host-pkgconf \
	libnetfilter_acct
NFACCT_LICENSE = GPL-2.0
NFACCT_LICENSE_FILES = COPYING

$(eval $(autotools-package))
