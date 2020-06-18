################################################################################
#
# tcllib
#
################################################################################

TCLLIB_VERSION = 1.13
TCLLIB_SOURCE = tcllib-$(TCLLIB_VERSION).tar.bz2
TCLLIB_SITE = http://downloads.sourceforge.net/project/tcllib/tcllib/$(TCLLIB_VERSION)
TCLLIB_LICENSE = TCL
TCLLIB_LICENSE_FILES = license.terms
TCLLIB_DEPENDENCIES = host-tcl
TCLLIB_CONF_ENV = ac_cv_path_tclsh="$(HOST_DIR)/bin/tclsh$(TCL_VERSION_MAJOR)"

$(eval $(autotools-package))
