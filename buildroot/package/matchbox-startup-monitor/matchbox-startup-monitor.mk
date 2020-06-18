################################################################################
#
# matchbox-startup-monitor
#
################################################################################

MATCHBOX_STARTUP_MONITOR_VERSION = 0.1
MATCHBOX_STARTUP_MONITOR_SOURCE = mb-applet-startup-monitor-$(MATCHBOX_STARTUP_MONITOR_VERSION).tar.bz2
MATCHBOX_STARTUP_MONITOR_SITE = http://downloads.yoctoproject.org/releases/matchbox/mb-applet-startup-monitor/$(MATCHBOX_STARTUP_MONITOR_VERSION)
MATCHBOX_STARTUP_MONITOR_LICENSE = GPL-2.0+
MATCHBOX_STARTUP_MONITOR_LICENSE_FILES = COPYING
MATCHBOX_STARTUP_MONITOR_DEPENDENCIES = matchbox-lib startup-notification

$(eval $(autotools-package))
