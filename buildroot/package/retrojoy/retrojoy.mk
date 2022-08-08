################################################################################
#
# retrojoy
#
################################################################################

RETROJOY_VERSION = 1.0
RETROJOY_SITE = $(TOPDIR)/../retrojoy
RETROJOY_SITE_METHOD = local
RETROJOY_LICENSE = Proprietary
#RETROJOY_LICENSE_FILES = 

define RETROJOY_BUILD_CMDS
	cd $(@D)
        $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
#	$(TARGET_MAKE_ENV) $(MAKE)    \
#		CC="$(TARGET_CC)"           \
#		EXTRA_CFLAGS="$(TARGET_CFLAGS)"  -Wall -static -c11 -Ofast -fomit-frame-pointer -funroll-loops -s \
# -I/opt/vc/include \
# -I/opt/vc/include/interface/vcos/pthreads \
# -I/opt/vc/include/interface/vmcs_host \
# -I/opt/vc/include/interface/vmcs_host/linux \
# -L/opt/vc/lib
#		EXTRA_LDFLAGS="$(TARGET_LDFLAGS)" \
#		-C $(@D) all 
		$(TARGET_STRIP) $(@D)/retrojoy
endef

define RETROJOY_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/retrojoy $(TARGET_DIR)/usr/bin/retrojoy
endef

$(eval $(generic-package))

