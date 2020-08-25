################################################################################
#
# xfont_font-bh-ttf
#
################################################################################

XFONT_FONT_BH_TTF_VERSION = 1.0.3
XFONT_FONT_BH_TTF_SOURCE = font-bh-ttf-$(XFONT_FONT_BH_TTF_VERSION).tar.bz2
XFONT_FONT_BH_TTF_SITE = http://xorg.freedesktop.org/releases/individual/font
XFONT_FONT_BH_TTF_LICENSE = Bigelow & Holmes License (no modification)
XFONT_FONT_BH_TTF_LICENSE_FILES = COPYING

XFONT_FONT_BH_TTF_INSTALL_STAGING_OPTS = DESTDIR=$(STAGING_DIR) MKFONTSCALE=$(HOST_DIR)/bin/mkfontscale MKFONTDIR=$(HOST_DIR)/bin/mkfontdir install
XFONT_FONT_BH_TTF_INSTALL_TARGET_OPTS = DESTDIR=$(TARGET_DIR) MKFONTSCALE=$(HOST_DIR)/bin/mkfontscale MKFONTDIR=$(HOST_DIR)/bin/mkfontdir install-data
XFONT_FONT_BH_TTF_DEPENDENCIES = xfont_font-util host-xfont_font-util host-xapp_mkfontscale host-xapp_bdftopcf

$(eval $(autotools-package))
