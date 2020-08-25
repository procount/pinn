################################################################################
#
# perl-http-message
#
################################################################################

PERL_HTTP_MESSAGE_VERSION = 6.25
PERL_HTTP_MESSAGE_SOURCE = HTTP-Message-$(PERL_HTTP_MESSAGE_VERSION).tar.gz
PERL_HTTP_MESSAGE_SITE = $(BR2_CPAN_MIRROR)/authors/id/E/ET/ETHER
PERL_HTTP_MESSAGE_LICENSE = Artistic or GPL-1.0+
PERL_HTTP_MESSAGE_LICENSE_FILES = LICENSE
PERL_HTTP_MESSAGE_DISTNAME = HTTP-Message

$(eval $(perl-package))
