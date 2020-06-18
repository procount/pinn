################################################################################
#
# perl-json-tiny
#
################################################################################

PERL_JSON_TINY_VERSION = 0.58
PERL_JSON_TINY_SOURCE = JSON-Tiny-$(PERL_JSON_TINY_VERSION).tar.gz
PERL_JSON_TINY_SITE = $(BR2_CPAN_MIRROR)/authors/id/D/DA/DAVIDO
PERL_JSON_TINY_LICENSE = Artistic-2.0
PERL_JSON_TINY_LICENSE_FILES = LICENSE
PERL_JSON_TINY_DISTNAME = JSON-Tiny

$(eval $(perl-package))
