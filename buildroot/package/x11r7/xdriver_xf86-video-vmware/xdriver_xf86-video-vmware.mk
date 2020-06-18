################################################################################
#
# xdriver_xf86-video-vmware
#
################################################################################

XDRIVER_XF86_VIDEO_VMWARE_VERSION = 13.3.0
XDRIVER_XF86_VIDEO_VMWARE_SOURCE = xf86-video-vmware-$(XDRIVER_XF86_VIDEO_VMWARE_VERSION).tar.bz2
XDRIVER_XF86_VIDEO_VMWARE_SITE = http://xorg.freedesktop.org/releases/individual/driver
XDRIVER_XF86_VIDEO_VMWARE_LICENSE = MIT
XDRIVER_XF86_VIDEO_VMWARE_LICENSE_FILES = COPYING
XDRIVER_XF86_VIDEO_VMWARE_DEPENDENCIES = mesa3d xserver_xorg-server xorgproto

ifeq ($(BR2_PACKAGE_HAS_UDEV),y)
XDRIVER_XF86_VIDEO_VMWARE_CONF_OPTS += --with-libudev
XDRIVER_XF86_VIDEO_VMWARE_DEPENDENCIES += udev
else
XDRIVER_XF86_VIDEO_VMWARE_CONF_OPTS += --without-libudev
endif

$(eval $(autotools-package))
