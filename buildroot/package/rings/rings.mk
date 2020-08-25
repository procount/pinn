################################################################################
#
# rings
#
################################################################################

RINGS_VERSION_MAJOR = 1.3.0
RINGS_VERSION = $(RINGS_VERSION_MAJOR)-1
RINGS_SUBDIR = rings-v_$(subst .,_,$(RINGS_VERSION_MAJOR))
RINGS_LICENSE = MIT
RINGS_LICENSE_FILES = $(RINGS_SUBDIR)/doc/us/license.html

$(eval $(luarocks-package))
