# Updating buildroot to a later version

This guide explains what changes need to be done to the default buildroot from buildroot.org to get a working buildroot for PINN.

## Preparing for the upgrade

The first step is to rename the directory `buildroot` to `buildroot.old` so you still have the old buildroot as backup and can copy files from there.
Then download the version of buildroot you want to use and copy it into a new directory called `buildroot`.

## Adding PINN changes to the new buildroot

Now you have to copy the following files/folders from the old buildroot to the new buildroot:

- **Arora browser**: The directory `package/arora` in the old buildroot contains arora, the web browser used in PINN.
- **Rebootp**: The directory `package/rebootp` in the old buildroot contains rebootp, a script which reboots the Raspberry Pi to a specific partition of the sd card.
- **PINN GUI**: The directory `package/recovery` in the old buildroot contains the PINN GUI.
- **SCSI header fix**: The directory `package/scsifix` in the old buildroot contains hader file to allow parted to be build.
- **Kernel patches**: Copy every .patch file in the directory `linux` of the old buildroot to the new buildroot.
- **Raspberry Pi packages**: Buildroot contains some packages for the Raspberry Pi: `rpi-firmware`, `rpi-userland` and `rpi-wifi-firmware`. Delete the .hash file in these directorys.
- **Configuration**: Copy the .config file from the old buildroot to the new one.
- **Linux hash file**: Delete linux/linux.hash in the new buildroot. PIN doesn't use the official linux kernel, so this file is not needed.
- **Package settings**: It is not enoguh to just copy the packages for PINN, you also need to add the following code in package/Config.in after the end of the menu `package managers`
```
menu "PINN packages"
	source "package/arora/Config.in"
	source "package/scsifix/Config.in"
	source "package/rebootp/Config.in"
	source "package/recovery/Config.in"
endmenu
```
- **Linux changes**: These changes are done to `linux/Config.in` and `linux/linux.mk`. Some just fix bugs and may not be required in future versions of buildroot. This is an example diff, line numbers may be different in other versions of buildroot.
```
diff --git a/buildroot/linux/Config.in b/buildroot/linux/Config.in
index dd55320a..c367b333 100644
--- a/buildroot/linux/Config.in
+++ b/buildroot/linux/Config.in
@@ -393,15 +393,7 @@ config BR2_LINUX_KERNEL_DTB_IS_SELF_BUILT
 config BR2_LINUX_KERNEL_APPENDED_DTB
 	bool
 
+config BR_LINUX_KERNEL_RPI
+	select BR2_LINUX_KERNEL_DTB_IS_SELF_BUILT
+	bool "Raspberry Pi kernel fixes"
+	help
+	  Fix install path of dtb files for the
+	  Raspberry Pi.
+
 config BR2_LINUX_KERNEL_INTREE_DTS_NAME
+	depends on !BR2_LINUX_KERNEL_DTB_IS_SELF_BUILT
 	string "In-tree Device Tree Source file names"
 	help
 	  Name of in-tree device tree source file, without
--- a/buildroot/linux/linux.mk
+++ b/buildroot/linux/linux.mk
@@ -162,9 +162,7 @@ endif
 # going to be installed in the target filesystem.
 LINUX_VERSION_PROBED = `$(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) --no-print-directory -s kernelrelease 2>/dev/null`
 
+ifeq ($(BR2_LINUX_KERNEL_DTB_IS_SELF_BUILT),)
+	LINUX_DTS_NAME += $(call qstrip,$(BR2_LINUX_KERNEL_INTREE_DTS_NAME))
+endif
-LINUX_DTS_NAME += $(call qstrip,$(BR2_LINUX_KERNEL_INTREE_DTS_NAME))
 
 # We keep only the .dts files, so that the user can specify both .dts
 # and .dtsi files in BR2_LINUX_KERNEL_CUSTOM_DTS_PATH. Both will be
@@ -432,16 +430,6 @@ define LINUX_INSTALL_DTB
 endef
 endif # BR2_LINUX_KERNEL_APPENDED_DTB
 endif # BR2_LINUX_KERNEL_DTB_IS_SELF_BUILT
+ifeq ($(BR_LINUX_KERNEL_RPI),y)
+define LINUX_INSTALL_DTB
+ 	cp $(KERNEL_ARCH_PATH)/boot/dts/bcm27*.dtb \
+  		$(1)/
+ 	mkdir -p $(1)/overlays/
+ 	cp $(KERNEL_ARCH_PATH)/boot/dts/overlays/*.dtbo \
+ 		$(KERNEL_ARCH_PATH)/boot/dts/overlays/README \
+ 		$(1)/overlays/
+endef
+endif
 endif # BR2_LINUX_KERNEL_DTS_SUPPORT
 
 ifeq ($(BR2_LINUX_KERNEL_APPENDED_DTB),y)
@@ -608,11 +596,9 @@ endif
 endif
 
 ifeq ($(BR2_LINUX_KERNEL_DTS_SUPPORT):$(strip $(LINUX_DTS_NAME)),y:)
+ifeq ($(BR2_LINUX_KERNEL_DTB_IS_SELF_BUILT),)
 $(error No kernel device tree source specified, check your \
 	BR2_LINUX_KERNEL_INTREE_DTS_NAME / BR2_LINUX_KERNEL_CUSTOM_DTS_PATH settings)
 endif
+endif
 
 endif # BR_BUILDING
```

## Removing unsupported settings

Now run `make menuconfig` in the buildroot directory. If buildroot now tells you there are legacy settings enabled, try to replace them with supported settings or disable them completely if they aren't needed anymore.
You can probably change GCC version and binutils version without errors. Password encryption can also be changed without issues. If the QT version requires to be changed, you may need to port the PINN code to the next QT version. 

## Updating packages

Now run `./BUILDME.sh update-firmware update-kernel update-userland nobuild` in the directory whre you cloned PINN,

## Finishing & testing changes

You can delete the buildroot.old directory now.
To test if everyting builds successfully, run `./BUILDME.sh`.
If it builds successfully, and the new version boots and works sucessfully on every Raspberry Pi, you now have managed to update buildroot in PINN.

