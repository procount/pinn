################################################################################
#
# phidgetwebservice
#
################################################################################

PHIDGETWEBSERVICE_VERSION = 2.1.8.20170607
PHIDGETWEBSERVICE_SOURCE = phidgetwebservice_$(PHIDGETWEBSERVICE_VERSION).tar.gz
PHIDGETWEBSERVICE_SITE = http://www.phidgets.com/downloads/libraries
PHIDGETWEBSERVICE_DEPENDENCIES = libphidget
PHIDGETWEBSERVICE_LICENSE = LGPL-3.0
PHIDGETWEBSERVICE_LICENSE_FILES = COPYING

$(eval $(autotools-package))
