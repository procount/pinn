################################################################################
#
# xdriver_xf86-video-glint
#
################################################################################

XDRIVER_XF86_VIDEO_GLINT_VERSION = 1.2.9
XDRIVER_XF86_VIDEO_GLINT_SOURCE = xf86-video-glint-$(XDRIVER_XF86_VIDEO_GLINT_VERSION).tar.bz2
XDRIVER_XF86_VIDEO_GLINT_SITE = http://xorg.freedesktop.org/releases/individual/driver
XDRIVER_XF86_VIDEO_GLINT_LICENSE = MIT
XDRIVER_XF86_VIDEO_GLINT_LICENSE_FILES = COPYING
XDRIVER_XF86_VIDEO_GLINT_DEPENDENCIES = xserver_xorg-server libdrm xorgproto

$(eval $(autotools-package))
