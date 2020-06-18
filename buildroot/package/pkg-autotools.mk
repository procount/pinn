################################################################################
# Autotools package infrastructure
#
# This file implements an infrastructure that eases development of
# package .mk files for autotools packages. It should be used for all
# packages that use the autotools as their build system.
#
# See the Buildroot documentation for details on the usage of this
# infrastructure
#
# In terms of implementation, this autotools infrastructure requires
# the .mk file to only specify metadata information about the
# package: name, version, download URL, etc.
#
# We still allow the package .mk file to override what the different
# steps are doing, if needed. For example, if <PKG>_BUILD_CMDS is
# already defined, it is used as the list of commands to perform to
# build the package, instead of the default autotools behaviour. The
# package can also define some post operation hooks.
#
################################################################################


#
# Utility function to upgrade config.sub and config.guess files
#
# argument 1 : directory into which config.guess and config.sub need
# to be updated. Note that config.sub and config.guess are searched
# recursively in this directory.
#
define CONFIG_UPDATE
	for file in config.guess config.sub; do \
		for i in $$(find $(1) -name $$file); do \
			cp support/gnuconfig/$$file $$i; \
		done; \
	done
endef

# This function generates the ac_cv_file_<foo> value for a given
# filename. This is needed to convince configure script doing
# AC_CHECK_FILE() tests that the file actually exists, since such
# tests cannot be done in a cross-compilation context. This function
# takes as argument the path of the file. An example usage is:
#
#  FOOBAR_CONF_ENV = \
#	$(call AUTOCONF_AC_CHECK_FILE_VAL,/dev/random)=yes
AUTOCONF_AC_CHECK_FILE_VAL = ac_cv_file_$(subst -,_,$(subst /,_,$(subst .,_,$(1))))

#
# Hook to update config.sub and config.guess if needed
#
define UPDATE_CONFIG_HOOK
	@$(call MESSAGE,"Updating config.sub and config.guess")
	$(call CONFIG_UPDATE,$(@D))
endef

#
# Hook to patch libtool to make it work properly for cross-compilation
#
define LIBTOOL_PATCH_HOOK
	@$(call MESSAGE,"Patching libtool")
	$(Q)for i in `find $($(PKG)_DIR) -name ltmain.sh`; do \
		ltmain_version=`sed -n '/^[ \t]*VERSION=/{s/^[ \t]*VERSION=//;p;q;}' $$i | \
		sed -e 's/\([0-9]*\.[0-9]*\).*/\1/' -e 's/\"//'`; \
		ltmain_patchlevel=`sed -n '/^[ \t]*VERSION=/{s/^[ \t]*VERSION=//;p;q;}' $$i | \
		sed -e 's/\([0-9]*\.[0-9]*\.*\)\([0-9]*\).*/\2/' -e 's/\"//'`; \
		if test $${ltmain_version} = '1.5'; then \
			patch -i support/libtool/buildroot-libtool-v1.5.patch $${i}; \
		elif test $${ltmain_version} = "2.2"; then\
			patch -i support/libtool/buildroot-libtool-v2.2.patch $${i}; \
		elif test $${ltmain_version} = "2.4"; then\
			if test $${ltmain_patchlevel:-0} -gt 2; then\
				patch -i support/libtool/buildroot-libtool-v2.4.4.patch $${i}; \
			else \
				patch -i support/libtool/buildroot-libtool-v2.4.patch $${i}; \
			fi \
		fi \
	done
endef

#
# Hook to patch common issue with configure on powerpc64{,le} failing
# to detect shared library support:
#
define CONFIGURE_FIX_POWERPC64_HOOK
	@$(call MESSAGE,"Checking configure (powerpc64/powerpc64le)")
	support/scripts/fix-configure-powerpc64.sh $($(PKG)_DIR)
endef

#
# Hook to gettextize the package if needed
#
define GETTEXTIZE_HOOK
	@$(call MESSAGE,"Gettextizing")
	$(Q)cd $($(PKG)_SRCDIR) && $(GETTEXTIZE) $($(PKG)_GETTEXTIZE_OPTS)
endef

#
# Hook to autoreconf the package if needed
#
define AUTORECONF_HOOK
	@$(call MESSAGE,"Autoreconfiguring")
	$(Q)cd $($(PKG)_SRCDIR) && $($(PKG)_AUTORECONF_ENV) $(AUTORECONF) $($(PKG)_AUTORECONF_OPTS)
endef

################################################################################
# inner-autotools-package -- defines how the configuration, compilation and
# installation of an autotools package should be done, implements a
# few hooks to tune the build process for autotools specifities and
# calls the generic package infrastructure to generate the necessary
# make targets
#
#  argument 1 is the lowercase package name
#  argument 2 is the uppercase package name, including a HOST_ prefix
#             for host packages
#  argument 3 is the uppercase package name, without the HOST_ prefix
#             for host packages
#  argument 4 is the type (target or host)
################################################################################

define inner-autotools-package

ifndef $(2)_LIBTOOL_PATCH
 ifdef $(3)_LIBTOOL_PATCH
  $(2)_LIBTOOL_PATCH = $$($(3)_LIBTOOL_PATCH)
 else
  $(2)_LIBTOOL_PATCH ?= YES
 endif
endif

ifndef $(2)_MAKE
 ifdef $(3)_MAKE
  $(2)_MAKE = $$($(3)_MAKE)
 else
  $(2)_MAKE ?= $$(MAKE)
 endif
endif

ifndef $(2)_AUTORECONF
 ifdef $(3)_AUTORECONF
  $(2)_AUTORECONF = $$($(3)_AUTORECONF)
 else
  $(2)_AUTORECONF ?= NO
 endif
endif

ifndef $(2)_GETTEXTIZE
 ifdef $(3)_GETTEXTIZE
  $(2)_GETTEXTIZE = $$($(3)_GETTEXTIZE)
 else
  $(2)_GETTEXTIZE ?= NO
 endif
endif

ifeq ($(4),host)
 $(2)_GETTEXTIZE_OPTS ?= $$($(3)_GETTEXTIZE_OPTS)
endif

ifeq ($(4),host)
 $(2)_AUTORECONF_OPTS ?= $$($(3)_AUTORECONF_OPTS)
endif

$(2)_CONF_ENV			?=
$(2)_CONF_OPTS			?=
$(2)_MAKE_ENV			?=
$(2)_MAKE_OPTS			?=
$(2)_INSTALL_OPTS                ?= install
$(2)_INSTALL_STAGING_OPTS	?= DESTDIR=$$(STAGING_DIR) install
$(2)_INSTALL_TARGET_OPTS		?= DESTDIR=$$(TARGET_DIR) install

#
# Configure step. Only define it if not already defined by the package
# .mk file. And take care of the differences between host and target
# packages.
#
ifndef $(2)_CONFIGURE_CMDS
ifeq ($(4),target)

# Configure package for target
define $(2)_CONFIGURE_CMDS
	(cd $$($$(PKG)_SRCDIR) && rm -rf config.cache && \
	$$(TARGET_CONFIGURE_OPTS) \
	$$(TARGET_CONFIGURE_ARGS) \
	$$($$(PKG)_CONF_ENV) \
	CONFIG_SITE=/dev/null \
	./configure \
		--target=$$(GNU_TARGET_NAME) \
		--host=$$(GNU_TARGET_NAME) \
		--build=$$(GNU_HOST_NAME) \
		--prefix=/usr \
		--exec-prefix=/usr \
		--sysconfdir=/etc \
		--localstatedir=/var \
		--program-prefix="" \
		--disable-gtk-doc \
		--disable-gtk-doc-html \
		--disable-doc \
		--disable-docs \
		--disable-documentation \
		--with-xmlto=no \
		--with-fop=no \
		$$(if $$($$(PKG)_OVERRIDE_SRCDIR),,--disable-dependency-tracking) \
		--enable-ipv6 \
		$$(NLS_OPTS) \
		$$(SHARED_STATIC_LIBS_OPTS) \
		$$(QUIET) $$($$(PKG)_CONF_OPTS) \
	)
endef
else

# Configure package for host
# disable all kind of documentation generation in the process,
# because it often relies on host tools which may or may not be
# installed.
define $(2)_CONFIGURE_CMDS
	(cd $$($$(PKG)_SRCDIR) && rm -rf config.cache; \
	$$(HOST_CONFIGURE_OPTS) \
	CFLAGS="$$(HOST_CFLAGS)" \
	LDFLAGS="$$(HOST_LDFLAGS)" \
	$$($$(PKG)_CONF_ENV) \
	CONFIG_SITE=/dev/null \
	./configure \
		--prefix="$$(HOST_DIR)" \
		--sysconfdir="$$(HOST_DIR)/etc" \
		--localstatedir="$$(HOST_DIR)/var" \
		--enable-shared --disable-static \
		--disable-gtk-doc \
		--disable-gtk-doc-html \
		--disable-doc \
		--disable-docs \
		--disable-documentation \
		--disable-debug \
		--with-xmlto=no \
		--with-fop=no \
		--disable-nls \
		$$(if $$($$(PKG)_OVERRIDE_SRCDIR),,--disable-dependency-tracking) \
		$$(QUIET) $$($$(PKG)_CONF_OPTS) \
	)
endef
endif
endif

$(2)_POST_PATCH_HOOKS += UPDATE_CONFIG_HOOK

ifeq ($$($(2)_AUTORECONF),YES)

# This has to come before autoreconf
ifeq ($$($(2)_GETTEXTIZE),YES)
$(2)_PRE_CONFIGURE_HOOKS += GETTEXTIZE_HOOK
$(2)_DEPENDENCIES += host-gettext
endif
$(2)_PRE_CONFIGURE_HOOKS += AUTORECONF_HOOK
# default values are not evaluated yet, so don't rely on this defaulting to YES
ifneq ($$($(2)_LIBTOOL_PATCH),NO)
$(2)_PRE_CONFIGURE_HOOKS += LIBTOOL_PATCH_HOOK
endif
$(2)_DEPENDENCIES += host-automake host-autoconf host-libtool

else # ! AUTORECONF = YES

# default values are not evaluated yet, so don't rely on this defaulting to YES
ifneq ($$($(2)_LIBTOOL_PATCH),NO)
$(2)_POST_PATCH_HOOKS += LIBTOOL_PATCH_HOOK
endif

endif

# Append a configure hook if building for a powerpc64 (or powerpc64le) arch.
# Must be added after other pre-configure hooks that might regenerate the
# configure script and overwrite the changes made here.
ifneq ($$(filter powerpc64%,$$(if $$(filter target,$(4)),$$(ARCH),$$(HOSTARCH))),)
$(2)_PRE_CONFIGURE_HOOKS += CONFIGURE_FIX_POWERPC64_HOOK
endif

#
# Build step. Only define it if not already defined by the package .mk
# file.
#
ifndef $(2)_BUILD_CMDS
ifeq ($(4),target)
define $(2)_BUILD_CMDS
	$$(TARGET_MAKE_ENV) $$($$(PKG)_MAKE_ENV) $$($$(PKG)_MAKE) $$($$(PKG)_MAKE_OPTS) -C $$($$(PKG)_SRCDIR)
endef
else
define $(2)_BUILD_CMDS
	$$(HOST_MAKE_ENV) $$($$(PKG)_MAKE_ENV) $$($$(PKG)_MAKE) $$($$(PKG)_MAKE_OPTS) -C $$($$(PKG)_SRCDIR)
endef
endif
endif

#
# Host installation step. Only define it if not already defined by the
# package .mk file.
#
ifndef $(2)_INSTALL_CMDS
define $(2)_INSTALL_CMDS
	$$(HOST_MAKE_ENV) $$($$(PKG)_MAKE_ENV) $$($$(PKG)_MAKE) $$($$(PKG)_INSTALL_OPTS) -C $$($$(PKG)_SRCDIR)
endef
endif

#
# Staging installation step. Only define it if not already defined by
# the package .mk file.
#
ifndef $(2)_INSTALL_STAGING_CMDS
define $(2)_INSTALL_STAGING_CMDS
	$$(TARGET_MAKE_ENV) $$($$(PKG)_MAKE_ENV) $$($$(PKG)_MAKE) $$($$(PKG)_INSTALL_STAGING_OPTS) -C $$($$(PKG)_SRCDIR)
endef
endif

#
# Target installation step. Only define it if not already defined by
# the package .mk file.
#
ifndef $(2)_INSTALL_TARGET_CMDS
define $(2)_INSTALL_TARGET_CMDS
	$$(TARGET_MAKE_ENV) $$($$(PKG)_MAKE_ENV) $$($$(PKG)_MAKE) $$($$(PKG)_INSTALL_TARGET_OPTS) -C $$($$(PKG)_SRCDIR)
endef
endif

# Call the generic package infrastructure to generate the necessary
# make targets
$(call inner-generic-package,$(1),$(2),$(3),$(4))

endef

################################################################################
# autotools-package -- the target generator macro for autotools packages
################################################################################

autotools-package = $(call inner-autotools-package,$(pkgname),$(call UPPERCASE,$(pkgname)),$(call UPPERCASE,$(pkgname)),target)
host-autotools-package = $(call inner-autotools-package,host-$(pkgname),$(call UPPERCASE,host-$(pkgname)),$(call UPPERCASE,$(pkgname)),host)
