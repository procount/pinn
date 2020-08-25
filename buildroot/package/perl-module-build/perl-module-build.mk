################################################################################
#
# perl-module-build
#
################################################################################

PERL_MODULE_BUILD_VERSION = 0.4231
PERL_MODULE_BUILD_SOURCE = Module-Build-$(PERL_MODULE_BUILD_VERSION).tar.gz
PERL_MODULE_BUILD_SITE = $(BR2_CPAN_MIRROR)/authors/id/L/LE/LEONT
PERL_MODULE_BUILD_LICENSE = Artistic or GPL-1.0+
PERL_MODULE_BUILD_LICENSE_FILES = LICENSE
PERL_MODULE_BUILD_DISTNAME = Module-Build

$(eval $(host-perl-package))
