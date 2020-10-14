################################################################################
#
# rpi-wifi-firmware
#
################################################################################

RPI_WIFI_FIRMWARE_VERSION = 7533cd1f124f07d87ca6fd11a4a2143748ed806c
RPI_WIFI_FIRMWARE_SITE = $(call github,RPi-Distro,firmware-nonfree,$(RPI_WIFI_FIRMWARE_VERSION))
RPI_WIFI_FIRMWARE_LICENSE = Proprietary
RPI_WIFI_FIRMWARE_LICENSE_FILES = LICENCE.broadcom_bcm43xx

define RPI_WIFI_FIRMWARE_INSTALL_TARGET_CMDS

    #BRCM
	$(INSTALL) -D -m 0644 $(@D)/brcm/brcmfmac43143.bin $(TARGET_DIR)/lib/firmware/brcm/brcmfmac43143.bin
	$(INSTALL) -D -m 0644 $(@D)/brcm/brcmfmac43430-sdio.bin $(TARGET_DIR)/lib/firmware/brcm/brcmfmac43430-sdio.bin
	$(INSTALL) -D -m 0644 $(@D)/brcm/brcmfmac43430-sdio.txt $(TARGET_DIR)/lib/firmware/brcm/brcmfmac43430-sdio.txt

        $(INSTALL) -D -m 0644 $(@D)/brcm/brcmfmac43455-sdio.bin $(TARGET_DIR)/lib/firmware/brcm/brcmfmac43455-sdio.bin
        $(INSTALL) -D -m 0644 $(@D)/brcm/brcmfmac43455-sdio.txt $(TARGET_DIR)/lib/firmware/brcm/brcmfmac43455-sdio.txt
        $(INSTALL) -D -m 0644 $(@D)/brcm/brcmfmac43455-sdio.clm_blob $(TARGET_DIR)/lib/firmware/brcm/brcmfmac43455-sdio.clm_blob

	$(INSTALL) -D -m 0644 $(@D)/brcm/brcmfmac43456-sdio.bin $(TARGET_DIR)/lib/firmware/brcm/brcmfmac43456-sdio.bin
	$(INSTALL) -D -m 0644 $(@D)/brcm/brcmfmac43456-sdio.txt $(TARGET_DIR)/lib/firmware/brcm/brcmfmac43456-sdio.txt
	$(INSTALL) -D -m 0644 $(@D)/brcm/brcmfmac43456-sdio.clm_blob $(TARGET_DIR)/lib/firmware/brcm/brcmfmac43456-sdio.clm_blob

    #RALINK
	$(INSTALL) -D -m 0644 $(@D)/rt3290.bin  $(TARGET_DIR)/lib/firmware/rt3290.bin

	$(INSTALL) -D -m 0644 $(@D)/mt7601u.bin $(TARGET_DIR)/lib/firmware/mt7601u.bin

	$(INSTALL) -D -m 0644 $(@D)/rt73.bin    $(TARGET_DIR)/lib/firmware/rt73.bin

	$(INSTALL) -D -m 0644 $(@D)/rt2860.bin  $(TARGET_DIR)/lib/firmware/rt2860.bin
	$(INSTALL) -D -m 0644 $(@D)/rt2870.bin  $(TARGET_DIR)/lib/firmware/rt2870.bin

	$(INSTALL) -D -m 0644 $(@D)/rt2561.bin  $(TARGET_DIR)/lib/firmware/rt2561.bin
	$(INSTALL) -D -m 0644 $(@D)/rt2561s.bin $(TARGET_DIR)/lib/firmware/rt2561s.bin
	$(INSTALL) -D -m 0644 $(@D)/rt2661.bin  $(TARGET_DIR)/lib/firmware/rt2661.bin
	$(INSTALL) -D -m 0644 $(@D)/rt3070.bin  $(TARGET_DIR)/lib/firmware/rt3070.bin
	$(INSTALL) -D -m 0644 $(@D)/rt3071.bin  $(TARGET_DIR)/lib/firmware/rt3071.bin
	$(INSTALL) -D -m 0644 $(@D)/rt3090.bin  $(TARGET_DIR)/lib/firmware/rt3090.bin

    #REALTEK
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8192cfw.bin       $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8192cfw.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8192cfwU.bin      $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8192cfwU.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8192cfwU_B.bin    $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8192cfwU_B.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8192cufw.bin      $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8192cufw.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8192cufw_A.bin    $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8192cufw_A.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8192cufw_B.bin    $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8192cufw_B.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8192cufw_TMSC.bin $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8192cufw_TMSC.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8192defw.bin      $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8192defw.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8192sefw.bin      $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8192sefw.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8712u.bin         $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8712u.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8723befw.bin      $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8723befw.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8723fw.bin        $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8723fw.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8723fw_B.bin      $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8723fw_B.bin

	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8188efw.bin       $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8188efw.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8188eufw.bin      $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8188eufw.bin

	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8821aefw.bin      $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8821aefw.bin
	$(INSTALL) -D -m 0644 $(@D)/rtlwifi/rtl8821aefw_wowlan.bin $(TARGET_DIR)/lib/firmware/rtlwifi/rtl8821aefw_wowlan.bin
endef

$(eval $(generic-package))
