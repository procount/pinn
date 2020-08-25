################################################################################
#
# tz
#
################################################################################

TZ_DEPENDENCIES = host-tzdata host-tzdump
TZ_LICENSE = Public domain

TZ_LOCALTIME = $(call qstrip,$(BR2_TARGET_LOCALTIME))
ifneq ($(TZ_LOCALTIME),)
define TZ_SET_LOCALTIME
	if [ ! -f $(TARGET_DIR)/usr/share/zoneinfo/uclibc/$(TZ_LOCALTIME) ]; then \
		printf "Error: '%s' is not a valid timezone, check your BR2_TARGET_LOCALTIME setting\n" \
			"$(TZ_LOCALTIME)"; \
		exit 1; \
	fi
	ln -sf ../usr/share/zoneinfo/uclibc/$(TZ_LOCALTIME) $(TARGET_DIR)/etc/TZ
endef
endif

define TZ_BUILD_CMDS
	(cd $(HOST_DIR)/share/zoneinfo/posix/; \
		for i in $$(find . -type f); do \
			mkdir -p $(@D)/output/$$(dirname $$i); \
			$(TZDUMP) -p . -q $${i#./} | sed '1d' > $(@D)/output/$$i; \
		done \
	)
endef

define TZ_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0644 $(HOST_DIR)/share/zoneinfo/zone.tab \
		$(TARGET_DIR)/usr/share/zoneinfo/zone.tab
	$(INSTALL) -D -m 0644 $(HOST_DIR)/share/zoneinfo/iso3166.tab \
		$(TARGET_DIR)/usr/share/zoneinfo/iso3166.tab
	mkdir -p $(TARGET_DIR)/usr/share/zoneinfo/uclibc
	cp -a $(@D)/output/* $(TARGET_DIR)/usr/share/zoneinfo/uclibc
	$(TZ_SET_LOCALTIME)
endef

$(eval $(generic-package))
