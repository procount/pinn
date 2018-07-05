# PINN (PINN is not NOOBS)
#### An enhanced Operating System installer for the Raspberry Pi

PINN is a version of the NOOBS Operating System Installer for the Raspberry Pi. See the change history below for the additional features.

### - [If you have PINN v2.4.3 - v2.4.4b installed, please manually update to v2.4.4c or later](https://www.raspberrypi.org/forums/viewtopic.php?f=63&t=142574&start=200#p1239359)

Documentation on how to use PINN can be found in **[README_PINN.md](README_PINN.md)**, which includes all of the original NOOBS documentation.

If you are already familiar with NOOBS' features, you may prefer to read the reduced documenation that just contains **[PINN's new features](changes.md)** alone.

The latest version of **[PINN-lite](http://sourceforge.net/projects/pinn/files/pinn-lite.zip)** can be downloaded from [sourceforge](http://www.sourceforge.net/projects/pinn).

Source code is available from [github](https://github.com/procount/pinn)

see  [github](https://github.com/procount/pinn) for full information

## Quick Start

#### Format your SD card as FAT32

For **Windows** users, we recommend formatting your SD card using the SD Association's Formatting Tool, which can be downloaded from https://www.sdcard.org/downloads/formatter_4/ .
If you are still using v4 of this tool, you will need to set the "FORMAT SIZE ADJUSTMENT" option to "ON" in the "Options" menu to ensure that the entire SD card volume is formatted - not just a single partition. However, this tool has now been upgraded to v5 where this feature is now the default and is no longer selectable. For more detailed and beginner-friendly formatting instructions, please refer to http://www.raspberrypi.org/quick-start-guide .

The SD Association's Formatting Tool is also available for <b>Mac</b> users. However, note that the default OSX Disk Utility is also capable of formatting the entire disk (select the SD card volume and choose "Erase" with "MS-DOS" format).

For **Linux** users, we recommend `gparted` (or the command line version `parted`). (Update: Norman Dunbar has written up the following formatting instructions for Linux users: http://qdosmsq.dunbar-it.co.uk/blog/2013/06/NOOBS-for-raspberry-pi/)

#### Copy the pinn files to your SD card.

- Download **[pinn-lite.zip](http://sourceforge.net/projects/pinn/files/pinn-lite.zip)** from [sourceforge](http://www.sourceforge.net/projects/pinn)
- Extract the files from pinn-lite.zip file onto the SD card. (Windows built-in zip features may have trouble with this file. If so, use another program such as 7zip.)

Please note that in some cases it may extract the files into a folder, if this is the case then please copy across the files from inside the folder rather than the folder itself.

#### Boot pinn on your PI
- Put the PINN SD card into your Pi and boot it.
- On first boot the "RECOVERY" FAT partition will be automatically resized to a minimum, so if you ever put your SD card back into a Windows machine it will look like a very small partition. THIS IS NORMAL.
- PINN does not come supplied with any OSes. You must download them from the internet or provide them on a USB stick. So,....
- Connect your Pi to the internet with an Ethernet cable (easiest)
- Or If you prefer to use Wifi, click on the wifi button and select your SSID and password. If the wifi button is greyed out, choose another type of wifi dongle.
- A list of OSes that are available to install will be displayed.
- Select one or more OSes that you want and click INSTALL.


## Change History

## V2.8.5.1

- **Reinstall**               - BUGFIX for Re-install.
- **translations**            - Updated it and zh_TW translations.
- **silentreinstallnewer**    - **USE WITH CAUTION**. With `select=` it will silently re-install any selected OSes that are newer than the installed version.

### V2.8.5

- **Boot Cancel**             - Added a new cancel button to the bootselection dialog which will reboot back into PINN
- **Swedish keyboard**        - Swedish keyboard fixed when language selected
- **Bootable selection**      - The bootselection dialog is only available if a bootable OS is installed.
- **Select Option**           - Auto select/check OSes to be installed. "allsd,allusb,allnetwork,waitsd,waitusb,waitnetwork,waitall,allinstalled,<osname>"
- **SilentInstall**           - with Select will auto-install selected OSes
- **Added Disablesdimages**   - Disablesdimages in recovery.cmdline will prevent OSes on SD card from being shown
- **Network start**           - The network is always started, even with silentinstall.
- **Flavour download**        - Flavours can now be downloaded from remote repos. Only need to download 1
- **Added flavours.tar.xz**   - Remote flavours need to include flavours.tar.xz
- **RPi Model Name**          - Added name of RPi model to MainMenu screen

### V2.8.4

- **Replace OSes**            - Allows the replacement of one OS with another
- **Project Spaces**          - Added Project Spaces
- **Reserve disk space**      - Reserve some space on PINN's partition before formatting
- **mkimage**                 - Added mkimage from uboot
- **xz**                      - Allow XZ Compression
- **Update Dialog**           - This now cancels any timeout setting

### V2.8.3

- **Programmable CEC**        - Fixed bugs
- **networktimeout**          - Added timeout to detect network presence

### V2.8.2

- **Reinstall**        - bugfix to Reinstall option following partuuid change on USBs

### V2.8.1

- **Programmable CEC** - Program your TV remote for use with PINN

### V2.8

- **Fix menu**        - Replaced Fsck with Fix menu option to include a wider range of tools
- **Store meta**      - Additional meta files stored on installation for fix up tools
- **Partuuid**        - Store partuuid references for better USB support
- **Tab fix**         - Fixed synchronisation of OS tab display

### V2.6.2

- **pinn_init.sh**    - Improve calling of script to run before PINN
- **config.txt**      - deleted

### V2.6.1

- **Kernel**          - Bump kernel to suppress missing mailbox commands in old firmware
- **pinn_init.sh**    - Add initialisation script to run before PINN

### V2.6

- **Firmware**        - Update firmware
- **Kernel**          - Update kernel to 4.14

### V2.5.5

- **Self-Update**     - A small fix to the self-update ignore feature

### V2.5.4

- **Countdown**       - Re-enabled the boot selection dialog countdown timer.

### V2.5.3

- **Switch firmware** - Enable switching of firmware for 3B+ and other models.

### V2.5.2a

Fixes the incorrect recovery.cmdline bug in v2.5.2.
(There's a reason why I didn't make this v2.7 just yet!)

### V2.5.2

- **OS Firmware**   - Includes a better method of slipstreaming new Firmware
- **BUG: Cmdline**  - Note that the recovery.cmdline is incorrect in this version
### V2.5.1

- **firmware**      - New firmware for Rpi 3B+
- **Kernel**        - Update to kernel 4.9.80
- **OS firmware**   - Latest firmware is slip streamed onto old OSes
- **wifi drivers**  - Some wifi drivers temporarily removed until they can be upgraded
- **Arora**         - Prevented multiple instances of Arora browser running

### V2.4.5f

- **rtl8812 wifi**   - added wifi driver for rtl8812 5GHz dongles

### V2.4.5e

- **remotetimeout**  - Enable network for remote boot alteration.

### V2.4.5b

- **background**  - The background has now been fixed on the bootmenu
- **reinstall**   - Reinstall should now use data from the NEW version rather than the OLD version

### V2.4.5

This is a small maintenance release for some cosmetic changes and bugfixes mainly.

- **background**  - Specify the background colour as background=r,g,b
- **style**       - Change the style with style=motif/windows/platinum
- **configpath**  - (partially) allow remote flavours to customise an OS
- **dsi**         - Allows the HDMI/DSI switching option to work with OSes installed to USB

### V2.4.4

Finally, PINN can now re-install any installed OS WITHOUT affecting the other OSes, eliminating one of my NOOBS bugbears.

- **Re-install**  - Re-install an OS without affecting others
- **no_update**   - disable PINN self-update (manual refesh of PINN available)
- **update PINN** - A manual PINN update check can now be done (for when no_update is used)
- **Edit PINN**   - The recovery.cmdline and config.txt fiels of PINN can now be edited.
- **no_cursor**   - The no_cursor option now only affects the bootselection dialog
- **Wipe**        - The Wipe Disk command has been temporarily removed
- **Max OSes**    - The number of OSes has been restricted so that boot partitions numbers are &lt;63

### V2.4.3

A major change to the User Interface in preparation for some new features.

- **3 Toolbars**  - Main Menu, Archival and Maintenance.
- **OS Groups**   - OS lists are grouped into General, Minimal, Educational, Media and Gaming.
- **Download OS** - Download an OS to USB for local installation.
- **Fsck**        - Check and fix your filesystems in case of error.
- **Wipe Disk**   - delete all OSes and restore drive to full capacity.
- **Info button** - Go directly to the webpage of each OS.

### V2.4.2h

- **Translations**    - udpate to Portugese + other translations
- **Password**        - A new password will not expire
- **Slideshow**       - Slides during installation are now scaled to be the same size.
- **>1TB MSDs**       - Installing OSes to a USB Mass Storage Device > 1TB would fail. 2TB is the maximum size.

### V2.4.2

- **repo_list**       - Added repo_list cmdline argument

#### Bugfixes

- **Win10IoT**        - Fixed annoying warning messages on installation
- **RTAndrod**        - re-included missing cpio to allow installation

### V2.4.1

- **btrfs**           - Added support for btrfs file systems

### V2.4

- **Update**          - Rebase onto NOOBS 2.4
- **repo**            - Support "repo=" from NOOBS
- **USB support**     - Supports USB BOOT and USB ROOTFS

### V2.3

- **PiZeroW**         - Further update to match NOOBS 2.3. Added regDB for channel 13 & missing DTBs

### V2.2

- **PiZeroW**         - Update firmware/kernel for PiZero Wifi version

### V2.1.4

- **IP address**      - Shown in window title
- **CEC key mapping** - Replaced  0 button with PLAY button for greater applicability
- **BOOT**            - Added BOOT button to boot selection dialog for use with limited TV remotes
- **Tarball names**   - Allow local tarballs to have a different name to the partition labell
- **SD Card**         - Recommend 8GB card minimum
- **SD Card**         - Update SD card requirements
- **Network**         - Do not continue polling for connectivity during installation
- **Update**          - Bump kernel and firmware versions

### V2.1.3

- **Translations**    - Added many translations from the community

### V2.1.2

- **Default Boot**    - A default OS can be set in a multi-boot setup for quicker boots.
- **Bootmenutimout**  - The timeout of the boot selection dialog can be changed.
- **SHIFT key**       - The PINN interface can be reached using the Left mouse button as well as shift.

### V2.1.1

- **CEC Support**     - Added CEC support to control PINN with a TV remote

### V2.1

- **IoTpHAT support** - Updated to match NOOBS v2.1 for IoTpHAT wifi

### V2.0

- **Network Drivers** - Added the popular RTL8188eu wifi driver (for HubPiWi and others)
- **New Firmware**    - Updated Firmware and Kernel to match NOOBS 2.0

### V1.9.5 

- **Self-Update**     - Notification and download of new releases

#### Bugfixes

- **Network Drivers** - Fixed the popular RTL8188cus wifi driver.
- **RPI3 ACT_LED**    - Now displays correctly on SD card access


### V1.9.4

- **Network Drivers** - A number of additional wifi and ethernet drivers have been added.

#### Bugfixes

- **Dialog Box**      - The initialisation dialog box is removed under network errors


### V1.9.3

- **Clone SD Card**   - Copy the SD card to another card in a USB reader (BETA)
- **Rescue shell**    - Exiting the rescue shell now enters the PINN recovery program instead of another shell.
- **VNCSHARE**        - Sets up PINN to use VNC at the same time as an attached screen

#### Bugfixes

- **https**           - Fixed the ability to use https protocol (which was preventing Arch from installing sometimes).
- **wifi config**     - A user supplied wpa_supplicant.conf file is now ALWAYS copied to /settings and then renamed to wpa_supplicant.conf.bak so that it does not continually overwrite any manual changes made through the GUI.


### V1.9.2 

- **Password**        - Change or reset your password for each OS.
- **DSI/HDMI**        - Auto-switching of DSI/HDMI screen with HDMI taking priority 

#### BugFixes

- **NoobsConfig**     - Config filenames now now have all spaces converted to underscores 
- **Multiple Os**     - Multiple installed OSes can now be selected again. 
- **DHCP**            - ClientID is now used instead of DUID to request an IP address 


### V1.9.1 (based on NOOBS v1.9)

- **USB support**     - store your OS installations on USB stick to avoid wasting SD card space and avoid downloading each time.
- **Alternative Src** - Install OSes from an alternative website, or local webserver
- **ARCH support**    - Install the Arch Linux OS directly from the Arch website
- **VGA666 support**  - Alow PINN to be used with Gert's VGA666 adaptor
- **NOOBSCONFIG support** - Allow customisation of an OS installation as it installs
- **SSH support**     - SSH remotely into you PINN installation.
- **Progress**        - View your installation progress via alternative means


