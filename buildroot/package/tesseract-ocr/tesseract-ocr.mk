################################################################################
#
# tesseract-ocr
#
################################################################################

TESSERACT_OCR_VERSION = 4.1.1
TESSERACT_OCR_DATA_VERSION = 4.0.0
TESSERACT_OCR_SITE = $(call github,tesseract-ocr,tesseract,$(TESSERACT_OCR_VERSION))
TESSERACT_OCR_LICENSE = Apache-2.0
TESSERACT_OCR_LICENSE_FILES = LICENSE

# Source from github, no configure script provided
TESSERACT_OCR_AUTORECONF = YES

# cairo, pango and icu are optional dependencies, but only needed for
# building training tools, which are only built explicitly with "make
# training", which is not done by this package.
TESSERACT_OCR_DEPENDENCIES = leptonica host-pkgconf
TESSERACT_OCR_INSTALL_STAGING = YES
TESSERACT_OCR_CONF_ENV = \
	ac_cv_prog_have_asciidoc=false \
	LIBLEPT_HEADERSDIR=$(STAGING_DIR)/usr/include/leptonica
TESSERACT_OCR_CONF_OPTS = \
	--disable-opencl

# Language data files download
ifeq ($(BR2_PACKAGE_TESSERACT_OCR_LANG_ENG),y)
TESSERACT_OCR_DATA_FILES += eng.traineddata
endif

ifeq ($(BR2_PACKAGE_TESSERACT_OCR_LANG_FRA),y)
TESSERACT_OCR_DATA_FILES += fra.traineddata
endif

ifeq ($(BR2_PACKAGE_TESSERACT_OCR_LANG_DEU),y)
TESSERACT_OCR_DATA_FILES += deu.traineddata
endif

ifeq ($(BR2_PACKAGE_TESSERACT_OCR_LANG_SPA),y)
TESSERACT_OCR_DATA_FILES += spa.traineddata
endif

ifeq ($(BR2_PACKAGE_TESSERACT_OCR_LANG_CHI_SIM),y)
TESSERACT_OCR_DATA_FILES += chi_sim.traineddata
endif

ifeq ($(BR2_PACKAGE_TESSERACT_OCR_LANG_CHI_TRA),y)
TESSERACT_OCR_DATA_FILES += chi_tra.traineddata
endif

TESSERACT_OCR_EXTRA_DOWNLOADS = \
	$(addprefix https://github.com/tesseract-ocr/tessdata/raw/$(TESSERACT_OCR_DATA_VERSION)/,\
		$(TESSERACT_OCR_DATA_FILES))

# Language data files installation
define TESSERACT_OCR_INSTALL_LANG_DATA
	$(foreach langfile,$(TESSERACT_OCR_DATA_FILES), \
		$(INSTALL) -D -m 0644 $(TESSERACT_OCR_DL_DIR)/$(langfile) \
			$(TARGET_DIR)/usr/share/tessdata/$(langfile)
	)
endef

TESSERACT_OCR_POST_INSTALL_TARGET_HOOKS += TESSERACT_OCR_INSTALL_LANG_DATA

$(eval $(autotools-package))
