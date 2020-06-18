################################################################################
#
# webrtc-audio-processing
#
################################################################################

WEBRTC_AUDIO_PROCESSING_VERSION = 0.3.1
WEBRTC_AUDIO_PROCESSING_SOURCE = webrtc-audio-processing-$(WEBRTC_AUDIO_PROCESSING_VERSION).tar.xz
WEBRTC_AUDIO_PROCESSING_SITE = http://freedesktop.org/software/pulseaudio/webrtc-audio-processing
WEBRTC_AUDIO_PROCESSING_INSTALL_STAGING = YES
WEBRTC_AUDIO_PROCESSING_LICENSE = BSD-3-Clause
WEBRTC_AUDIO_PROCESSING_LICENSE_FILES = COPYING
WEBRTC_AUDIO_PROCESSING_DEPENDENCIES = host-pkgconf
# 0001-Proper-detection-of-cxxabi.h-and-execinfo.h.patch
WEBRTC_AUDIO_PROCESSING_AUTORECONF = YES

ifeq ($(BR2_SOFT_FLOAT),y)
WEBRTC_AUDIO_PROCESSING_CONF_OPTS += --with-ns-mode=fixed
endif

$(eval $(autotools-package))
