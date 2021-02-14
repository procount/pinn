# PINN (PINN is not NOOBS)
#### An enhanced Operating System installer for the Raspberry Pi

PINN is a version of the NOOBS Operating System Installer for the Raspberry Pi with the following additional features:

## Change History

### V3.5.5

- **Joysticks** - New joystick drivers including triggers and deadzones. Support for 2 joysticks. 
- **Firmware**  - Removed firmware upgrades for very old OSes.
- **Install**   - Install icon changed to warn against deletion {#380}
- **CM4**       - Added dwc2 USB driver for CM4
- **Backup**    - Better backups by deleting socket files (#442) and setting attributes (#447).
- **ssh**       - Uses permanent storage
- **progress**  - prevent progressdialog from closing

### V3.5.4

- **buildroot**    - improve some build scripts
- **dualhdmi**     - fix license
- **Installation** - Refactor close dialogs for better error handling
- **Memory**       - Display amount of Pi memory fitted
- **partuuid**     - Allow installation to USB without partuuid
- **Filesystems**  - Added ext3 & exfat
- **Wifi**         - Updated wifi firmware

### V3.5.2

- **KERNEL** - Fix issue with PI4 8GB models
- **Translations** - Update it translations
- **HDMI** - Automatic switching between HDMI0 and HDMI1

### V3.5

- **KERNEL** - Bump firmware & kernel as NOOBS 3.5 for CM4
- **Certificates** - Fix arora certificate errors
- **Translations** - Update it and zh_TW translations
- **Prompt** - Update shell prompt
- **Clarify** - Clarify text between replace & reinstall
- **Logs** - Add Show Log in Fix up menu

### V3.3.4.4

- **Labels**       - Fix partition labels when replacing OS

### V3.3.4.3

- **Backup**       - Fix backup of Raspberry Pi OS
- **Install**      - Use multithreaded xz
- **Flavours**     - Fix use of .txt files
- **bugfix**       - fix crash if no source (#414)

### V3.3.4.2

- **Firmware**     - Update firmware 5.4.45 for usb-boot (beta)
- **Name Change**  - Update recommended OS name
- **KeybdTrigger** - Fix keyboard trigger option
- **Translations** - Update Italian language

### V3.3.4.1

- **CloneSD**      - Remove drive restrictions on Clone SD
- **Set Time**     - Added indication that time is in UTC
- **Replace**      - Fixed shortcut for Replace OS
- **Buildversions** - Made buildversions more intuitive (for me!)
- **Legacy**       - Accounted for Pi4 when identifying legacy hardware

### V3.3.3

- **MBR fix**      - Better detection of non-MBR formatted SD cards.
- **Joysticks**    - Better detection of joysticks that are slow to connect. 
- **Reload Repos** - New button to reload the distribution list from internet.
- **fontsize**     - Added rudimentary font size adjustment using +/- keys.

### V3.3.2

- **Joystick**        - Added support for USB Joysticks/Gamepads (Xbox-style)
- **Silentinstall**   - Fixed a bug when silent-installing from the network
- **VirtualKeyboard** - Added a Virtual Keyboard for wifi and option dialogs

### V3.3c

- **Translations** - Updated a few translations (it, zh_TW, ko, de)
- **HDMI Pi4**     - Removed hdmi_force_hotplug
- **Downloads**    - Fixed download progressbar
- **Replace**      - Changed accelerator key from L to C

### V3.3a

- **Tidy up** - Development moved to Ubuntu 18.04 prompting a tidy up

### V3.3

- **PI4 support**  - Added support for the new RPi4. Includes Raspbian Buster in full version.

### V3.2.4c

- **Hyperpixel**   - Fixed hyperpixel 4 overlay file

### V3.2.4b

- **Wifi Drivers** - Re-added Realtek rtl8188eu and rtl8192cu modular wifi drivers.

### V3.2.4a

- **Build**       - Modified to build on Ubuntu 18.10 'cosmic'
- **Progress**    - Experimental modification to measure installation progress more accurately
- **Wifi dialog** - Removed focus for better use by keyboard
- **Zipfles**     - Use bsdtar to unzip inline for checksums
- **Esc**         - Esc key exits all menus
- **Firmware**    - Don't automatically downgrade firmware
- **Options**     - Added shortcuts for better keyboard use
- **GPIO**        - Allows gpiochannel and gpiochannelValue to be specified to trigger the recovery menu

### V3.2.3

- **Customisations** - Fix bug introduced by backup preventing customisations.

### V3.2.2

- **Backup hotfix** - Fix for OSnames with spaces.
- **Translations**    - Updated IT & zh_TW
- **Iconsizes** - fixed to 40x40
- **Download** - ignore partials

### V3.2.1

- **Bootselectiondialog** - update for CEC and keyboard

### V3.2

- **Added Ts & C's dialog** - for Future OSes
- **Resumable download**    - in case of download errors
- **More robust file transfer** - in case of download errors
- **Change boot selection order** - Just for aesthetics
- **Added file checksums** - for download verification
- **Added partition_setup.sh checksum** - for download verification
- **Added options dialog** - To make option setting easier.

### V3.1.1

- **alias**                 - Fix underscores
- **backup**                - Much faster backups on multicore processors
- **silentreinstallnewer**  - exits to boot menu & stops remotetimeout
- **menu**                  - Removed PageDown->nextmenu function because it conflicts
- **Translations**          - Updated IT & zh_TW translations

## V3.1

- **Nano**              - Added nano text editor
- **Rename**            - Allows giving an OS an Alias name & change its description	
- **Hyperpixel**        - Added full kernel drivers to support Pimoroni Hyperpixel 3.5" and 4" touchscreens 
- **ScreenSwitch**      - Performs automatic screen switching at bootup
- **dhcpcd**            - Now configurable

## V3.0.4    

- **forceupdatepinn**   - Add forceupdatepinn option
- **OS Replace**        - Fixed problem replacing an OS when PINN is booted from USB

## V3.0.3

- **Backup USB**        - Fix for multiple USB sticks.
- **Translation**       - Ko & zh_tw.ts updated.

## V3.0.2

- **Backup USB**        - Improved backup size calculation for USB installed OSes.

## V3.0.1

- **Backup/Download**   - Removed the need to reboot after a download or backup.
- **Wallpapers**        - removed wallpapers folder & Allowed JPEGs
- **Default wallpaper   - Changed default to wallpaper.jpg

## V3.0

- **Backup**                  - Backup an OS in PINN format
- **Time**                    - Added button to set date & time	
- **Download**                - Fixed foldername of some OSes when downloaded
- **Iconcache**               - Cache icons when connected to internet to speedup startup

## V2.8.5.8

- **RPi 3A+**                 - Supports RPi 3A+

## V2.8.5.7

- **Firmware**                - Disabled firmware downgrade on 3B+
- **Clear**                   - Added Clear button to clear all selections
- **Kernel Bump**             - Linux kernel 4.14.74, firmware and userland bumped to match NOOBS 2.9
- **Replace**                 - Removed RiscOS & Windows_10 from OS replacement
- **SilentInstall**           - Fixed waiting for USB images
- **networktimeout**          - 0 means wait indefinitely for network before silentinstall

## V2.8.5.5

- **Boot menu**               - Sticky default now uses bootmenutimeout
- **rebootp**                 - Added rebootp command to recovery shell

## V2.8.5.4

- **Boot menu**               - Fixed focus and sticky default partuuid issues.
- **wallpaper_resize**        - Customised wallpaper can now be resized to fill the screen

## V2.8.5.3

- **Partuuid**                - Fixed an issue from v2.8 where partuuids were introduced to installed_os.json
- **Wallpaper**               - Customised wallpaper is now available by writing your own `wallpaper.png` file to PINN's recovery partition

## V2.8.5.2

- **VNCnotice**               - Displays a notice on the main screen when vncinstall is in use
- **PINN update**             - Prevents crash on manual update when there is no network
- **ae-ts**                   - tidy up of translations

## V2.8.5.1

- **Reinstall**               - BUGFIX for Re-install.
- **translations**            - Updated it and zh_TW translations.
- **silentreinstallnewer**    - **USE WITH CAUTION**. With `select=` it will silently re-install any selected OSes that are newer than the installed version.

## V2.8.5

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

This new option allows PINN to be used more easily in a remote headless environment.

### V2.4.5

This is a small maintenance release for some cosmetic changes and bugfixes mainly.

- **background**  - Specify the background colour as background=r,g,b
- **style**       - Change the style with style=motif/windows/platinum
- **configpath**  - (partially) allow remote flavours to customise an OS
- **dsi**         - Allows the HDMI/DSI switching option to work with OSes installed to USB

### V2.4.4c

Finally, PINN can now re-install any installed OS WITHOUT affecting the other OSes, eliminating one of my NOOBS bugbears.

- **Re-install**  - Re-install an OS without affecting others
- **no_update**   - disable PINN self-update (manual refesh of PINN available)
- **update PINN** - A manual PINN update check can now be done (for when no_update is used)
- **Edit PINN**   - The recovery.cmdline and config.txt fiels of PINN can now be edited.
- **no_cursor**   - The no_cursor option now only affects the bootselection dialog
- **Wipe**        - The Wipe Disk command has been temporarily removed
- **Max OSes**    - The number of OSes has been restricted so that boot partitions numbers are &lt;63
- **self-update** - Fixes broken self-update since v2.4.2h

### V2.4.3

A major change to the User Interface in preparation for some new features.

- **3 Toolbars**      - Main Menu, Archival and Maintenance.
- **OS Groups**       - OS lists are grouped into General, Minimal, Educational, Media and Gaming.
- **Download OS**     - Download an OS to USB for local installation.
- **Fsck**            - Check and fix your filesystems in case of error.
- **Wipe Disk**       - delete all OSes and restore drive to full capacity.
- **Info button**     - Go directly to the webpage of each OS.

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

- **btrfs**           - Included support for btrfs file systems

### V2.4

- **Update**          - Rebase onto NOOBS 2.4
- **repo**            - Support "repo=" from NOOBS
- **USB support**     - Supports USB BOOT and USB ROOTFS

### V2.3

- **PiZeroW**         - Further update to match NOOBS 2.3. Add wifi RegDB for channel 13. Add missing DTBs

### V2.2

- **PiZeroW**         - Update firmware/kernel for PiZero Wifi version

### V2.1.4

- **IP address**      - Shown in window title
- **CEC key mapping** - Replaced  0 button with PLAY button for greater applicability
- **BOOT**            - Added BOOT button to boot selection dialog for use with limited TV remotes
- **Tarball names     - Allow local tarballs to have a different name to the partition labell
- **SD Card**         - Recommend 8GB card minimum
- **SD Card**         - Update SD card requirements
- **Network**         - Do not continue polling for connectivity during installation
- **Update**          - Bump kernel and firmware versions


### V2.1.3

- **Translations**    - Added many translations from the community

### V2.1.2 

- **Default Boot**    - A default OS can be set in a multi-boot setup for quicker boots.
- **Bootmenutimout**  - The timeout of the boot selection dialog can be changed.
- **SHIFT key**       - The PINN interface can be reached using the Left mouse button or a CEC enabled TV remote key as well as the SHIFT key.

#### Bugfixes

- The build dependencies introduced in v2.1.1 have been fixed.

### V2.1.1

- **CEC Support**     - Added CEC support for controlling PINN with a TV remote

### V2.1

- **IoTpHAT support** - Updated to match NOOBS v2.1 to include IoTpHAT wifi

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

see  [github](https://github.com/procount/pinn) for full information





