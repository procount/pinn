################################################################################
#
# python-lxml
#
################################################################################

PYTHON_LXML_VERSION = 4.5.1
PYTHON_LXML_SITE = https://files.pythonhosted.org/packages/03/a8/73d795778143be51d8b86750b371b3efcd7139987f71618ad9f4b8b65543
PYTHON_LXML_SOURCE = lxml-$(PYTHON_LXML_VERSION).tar.gz

# Not including the GPL, because it is used only for the test scripts.
PYTHON_LXML_LICENSE = BSD-3-Clause, Others
PYTHON_LXML_LICENSE_FILES = \
	LICENSES.txt \
	doc/licenses/BSD.txt \
	doc/licenses/elementtree.txt \
	src/lxml/isoschematron/resources/rng/iso-schematron.rng

# python-lxml can use either setuptools, or distutils as a fallback.
# So, we use setuptools.
PYTHON_LXML_SETUP_TYPE = setuptools

PYTHON_LXML_DEPENDENCIES = libxml2 libxslt zlib
HOST_PYTHON_LXML_DEPENDENCIES = host-libxml2 host-libxslt host-zlib

# python-lxml needs these scripts in order to properly detect libxml2 and
# libxslt compiler and linker flags
PYTHON_LXML_BUILD_OPTS = \
	--xslt-config=$(STAGING_DIR)/usr/bin/xslt-config \
	--xml2-config=$(STAGING_DIR)/usr/bin/xml2-config
HOST_PYTHON_LXML_BUILD_OPTS = \
	--xslt-config=$(HOST_DIR)/bin/xslt-config \
	--xml2-config=$(HOST_DIR)/bin/xml2-config

$(eval $(python-package))
$(eval $(host-python-package))
