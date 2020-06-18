################################################################################
#
# lbreakout2
#
################################################################################

LBREAKOUT2_VERSION_MAJOR = 2.6
LBREAKOUT2_VERSION = $(LBREAKOUT2_VERSION_MAJOR).5
LBREAKOUT2_SITE = https://downloads.sourceforge.net/lgames/lbreakout2/$(LBREAKOUT2_VERSION_MAJOR)
LBREAKOUT2_LICENSE = GPL-2.0+
LBREAKOUT2_LICENSE_FILES = COPYING

LBREAKOUT2_DEPENDENCIES = sdl libpng $(TARGET_NLS_DEPENDENCIES)

LBREAKOUT2_CONF_ENV = \
	SDL_CONFIG="$(STAGING_DIR)/usr/bin/sdl-config" \
	LIBS=$(TARGET_NLS_LIBS)

ifeq ($(BR2_PACKAGE_LBREAKOUT2_AUDIO),y)
LBREAKOUT2_DEPENDENCIES += sdl_mixer
LBREAKOUT2_CONF_OPTS += --enable-audio=yes
else
LBREAKOUT2_CONF_OPTS += --disable-audio
endif

ifeq ($(BR2_PACKAGE_LBREAKOUT2_NET),y)
LBREAKOUT2_DEPENDENCIES += sdl_net
LBREAKOUT2_CONF_OPTS += --enable-network=yes
else
LBREAKOUT2_CONF_OPTS += --disable-network
endif

$(eval $(autotools-package))
