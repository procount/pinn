################################################################################
#
# libcgroup
#
################################################################################

LIBCGROUP_VERSION = 0.41
LIBCGROUP_SOURCE = libcgroup-$(LIBCGROUP_VERSION).tar.bz2
LIBCGROUP_SITE = http://downloads.sourceforge.net/project/libcg/libcgroup/v$(LIBCGROUP_VERSION)
LIBCGROUP_LICENSE = LGPL-2.1
LIBCGROUP_LICENSE_FILES = COPYING
LIBCGROUP_DEPENDENCIES = host-bison host-flex
LIBCGROUP_INSTALL_STAGING = YES

# 0001-cgrulesengd-remove-umask-0.patch
LIBCGROUP_IGNORE_CVES += CVE-2018-14348

# Undefining _FILE_OFFSET_BITS here because of a "bug" with glibc fts.h
# large file support. See https://bugzilla.redhat.com/show_bug.cgi?id=574992
# for more information.
LIBCGROUP_CONF_ENV = \
	CXXFLAGS="$(TARGET_CXXFLAGS) -U_FILE_OFFSET_BITS" \
	CFLAGS="$(TARGET_CFLAGS) -U_FILE_OFFSET_BITS"

LIBCGROUP_CONF_OPTS = \
	--disable-daemon \
	--disable-initscript-install

ifeq ($(BR2_PACKAGE_LIBCGROUP_TOOLS),y)
LIBCGROUP_CONF_OPTS += --enable-tools
else
LIBCGROUP_CONF_OPTS += --disable-tools
endif

ifeq ($(BR2_PACKAGE_LINUX_PAM),y)
LIBCGROUP_DEPENDENCIES += linux-pam
LIBCGROUP_CONF_OPTS += --enable-pam
else
LIBCGROUP_CONF_OPTS += --disable-pam
endif

$(eval $(autotools-package))
