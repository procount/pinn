# V1.9 Features
#### How to access the shell or SSH into NOOBS

1. Even if the NOOBS GUI is launched, the busybox shell can still be accessed by pressing CTRL-ALT-F2. Use CTL-ALT-F1 to get back to the gui. 

2. You can now also SSH into the NOOBS basic busybox shell. To enable this feature, add `ssh` to the argument list in the recovery.cmdline. SSH is also accessible from the rescueshell, but allow 5 seconds after boot to allow the network to establish. NOOBS SSH does not store any keys so it takes a while to connect at first after each boot as it generates new keys for that session.

Use the username of 'root' and password 'raspberry' to login to the shell via the console or SSH.

### How to install an OS from USB

Due to the increasing size of OSes, it is becoming increasingly difficult to store their compressed installation files on the NOOBS SD card. So NOOBS-full now only includes one default Raspbian installation and the remainder are downloadable using a network connection.
To counter this, NOOBS supports storing the compressed OSes on a USB stick aswell. This has the following advantages:

1. The OSes are available to install when a network is not available.
* They do not take up any valuable space on the SD card.

The OSes should be stored on the USB stick in the same format as they would be on a NOOBS SD card:

1. Each OS should be stored in its own folder beneath the /os/ folder.
* They should contain compressed images of their partitions in .tar.xz format
* All supporting json files etc. shall also be included.
The USB stick should be inserted into the RPi before NOOBS is booted. Ideally it should be connected to the RPi and not via a usb hub as this may introduce a delay preventing the USB stick from being recognised.

If the same OS is available on the SD card, the USB stick and the network, only the most recent version is displayed.

### How to install OSes from an alternative source
 
An alternative source of OSes to be downloaded from the internet can be specified by adding <b>alt_image_source=http://newurl.com/os_list.json</b> to the argument list in recovery.cmdline, where `newurl.com` is the name of the alternative server and `os_list.json` is the list of information about the alternative OSes. This list of OSes will be added to the default download list. This can be useful for storing the default repository on a local LAN server, or for adding your own list of OSes to those available in NOOBS.

To suppress the default URL and only use the alternative image source for downloading OS images, add <b>no_default_source</b>. Using this option without `alt_image_source` will prevent all internet downloads and just allow local OSes on the SD card to be listed.

### How to use with Gert's VGA666 DPI display screen

The VGA666 adaptor connects to the GPIO pins and allows a VGA display to be attached to the RPi. The normal VGA666 installation instructions should be followed to allow it to work with NOOBS.

Create a config.txt file with the following lines in it:

<code>add dtoverlay=VGA666<br>
enable_dpi_lcd=1<br>
display_default_lcd=1<br>
dpi_group=<group> (e.g. dpi_group=1, or dpi_group=2)<br>
dpi_mode=<mode> (e.g. dpi_mode=28 - see tvservice for a list of possible modes)<br></code>

### How to customise an OS install

NOOBS can be used as a recovery program, so if your OS gets corrupted, messed up or otherwise goes wrong, you can reinstall a clean version of the OS and start again.
On the other hand, starting again from scratch can be painful, especially if you can't remember how you set up your perfect OS environment.

NOOBS now includes support for `noobsconfig` (see http://github.com/procount/noobsconfig), so it can customise an OS installation by copying additional files over the top of the OS before it is booted for the first time. These files can typically be used to setup wifi, install course material, or inject run-once scripts that execute on boot to perform more complciated installations or customisations.
By keeping these customizations separate from the OS, it avoids having to rebuild a custom OS each time a new version of the OS is released.

### Installation Progress

During the installation of the operating systems, NOOBS will write the percentage completed to a text
file called /tmp/progress. The format of this file is an integer (0-100) followed by a space, 
a '%' symbol and a line feed. It is updated only when the progress changes by at least 1%. 
Sometimes NOOBS will not know the maximum size, so in this case it shows the amount data written in MBs.
This feature mimics the progress dialog on the display and is useful in headless installations.

To make use of this feature a background shell script can be used. If a /background.sh script 
exists, it will be executed in the background whilst NOOBS runs. This can be used to read the 
/tmp/progress file and display the progress on the serial port, or a GPIO display etc.

### Installing Arch linux

Arch linux distros use extended file attributes that are not stored in a standard TAR archive.
Therefore they use BSDTAR to create their distro archives which are not compatible with NOOBS.
NOOBS adds BSDTAR support to install these files, allowing ARCH linux to be installed
by NOOBS. The partition_setup.sh file has been modified to allow ARCH linux distros to be 
installed directly from their website without converting into NOOBS's XZ format.

To install Arch it is necessary to add another repository to NOOBS.
Edit recovery.cmdline and add `alt-image-source=https://kent.dl.sourceforge.net/project/pinn/os/os_list_v3.json`

In addition, the recovery.cmdline should have `disablesafemode` added as a parameter since without it, the VGA666 will force NOOBS into safe mode due to its connections to the GPIO header.

# V1.9.2 Features
### Bugfixes

1. **Noobsconfig filenames** - All configuration filenames based on `<osname>_<partition>` (with a .txt, .tar or .tar.xz extension) now have all spaces converted to underscores. This was omitted in the previous version compared with the noobsconfig scripts.
* **Multiple OS selection** - Information about multiply installed OSes was not stored properly, so only the first installed OS would be selectable.
* **DHCP** - NOOBS Issue #336 & PR #337 have been implemented to allow DHCP to work with Microsoft hardened servers.

### How to auto-switch HDMI/DSI screen configurations

If an HDMI and a DSI screeen (such as the Raspberry Pi Touch Screen) are both connected to the Raspberry Pi, the selection of which screen is to be used as the default needs to be selected in the config.txt file, which means constantly having to change config.txt to match whichever screen is required. This is because the DSI screen can only be selected at boot time, although without *any* configuration the GPU will select the DSI screen in preference to the HDMI. PINN provides some limited ability to reverse this prioriy. 

This use case assumes that the DSI screen is always connected, and is normally used. But if an HDMI screen is connected, then the display will automatically switch to it. 


1. Create a `config.txt` file in the PINN root partition. Ensure it has the line `ignore_lcd=1` to disable the DSI screen and select the HDMI screen.
* Edit `recovery.cmdline` and add the `dsi` keyword.
* In the boot partition of any installed OS, create a `config.dsi` file to configure the DSI screen, and a `config.hdmi` file to configure the hdmi screen. (This can mostly be setup using a custom flavour with the noobsconfig feature so it can be installed automatically.)

When PINN boots, the HDMI screen will be selected, so to use any PINN feature, an HDMI screen must be connected. 

PINN will automatically boot the last selected OS after it times out. If a HDMI screen is attached, PINN will copy the config.hdmi file to config.txt on the selected OS and reboot into it.

If an HDMI screen was not detected, PINN will copy the config.dsi file to config.txt on the selected OS and reboot into it.

### How to Recover from a lost password

If you have changed your login password for an OS and forget what it is, PINN will allow you to set it back to its default or set it to a new value.

1. On the PINN screen, select the new `Advanced` menu.
* Select the installed OS that you want to reset the password in.
* Selecting the change password button will display a dialog box to enter the new password details.
* Enter the username you want to change the password of.
* Enter the new password (twice). Both copies must match. The second will be displayed red if it is different.
* The password strength meter is a useful indication of how good a password you have created.
* The `Use Dafault` button will enter the default username and password for the selected OS.
* Tick the `show password` box to display the passwords on the screen.

# V1.9.3 Features

### Bugfixes
1. Fixed the ability to use https protocol (which was preventing Arch from installing sometimes).
* A user supplied wpa_supplicant.conf file is now ALWAYS copied to /settings and then renamed to wpa_supplicant.conf.bak so that it does not continually overwrite any manual changes made through the GUI.

### Clone SD Card

Raspbian has recently added an SD Card Copier tool called piclone, which is great. For anyone concerned about it cloning a live system, 
it is now ported into PINN so that it can clone the SD card offline, whilst the OS is not running. Simply insert a second SD card into 
the Pi using a USB card reader and clone your entire SD card onto it. All partitions will be copied onto the second card whilst the last 
partition will be sized fit the remainder of the SD card. In this way your existing data can be migrated to a smaller or larger SD card.

Note that if you have installed multiple OSes using PINN, only the last partition on the SD card will be resized. 

### Rescue shell

Entering `rescueshell` in the `recovery.cmdline` file will cause the shell to be entered as soon as PINN is booted. On typing `exit`, 
the PINN recovery program will now continue, instead of exiting into another shell. See NOOBS issue #271.

### VNCshare

Adding `vncshare` to recovery.cmdline is similar to `vncinstall` except the display is output both to the selected display AND to a 
remote VNC client simultaneously. This avoids having to keep editing recovery.cmdline to switch from one to the other as they are both 
always available.

# V1.9.4 Features

### Bugfixes
1. Removed the "Please wait while PINN initialises" dialog box under some rare network circumstances.

### Additional Network Drivers

The following network drivers have been added to support a wider range of wifi and network adapters:
* zd1211rw
* rtl8192cu
* rt2500usb
* rt73usb
* rt2800usb
* mt7601u
* rtl8150
* r8152
* asix
* ax88179_178a
* dm9601
* smsc95xx

#V1.9.5 Features

### Bugfixes

1. The rtl8188cus driver has been updated to work with the cfg80211 interface.
2. The ACT LED will now work properly on the RPi 3B

### Self Update

If PINN is connected to the internet, it will check for the latest version available for download. If a newer version is available, a dialog box will pop-up showing the latest version number and the recent cahnges made. You then have the option to cancel the update, download it, or ignore this version.
If you cancel, the dialog box will keep popping up to remind you each time PINN starts.
If you select ignore, this version will be ignored and no dialogs will popup again until another later version is released.
If you choose to download this update, the zip file will be downloaded and will overwrite your current version of PINN. PINN will then reboot the RPi and restart PINN.

#v2.0 Features

rtl8188eu wifi driver with cfg80211 interface added (for e.g. HubPiWi)
Linux Kernel and rpi-firmware updated to match NOOBS v2.0

#V2.1 Features

Updated to match NOOBS v2.1
Now supports the IoTpHAT wifi.

#V2.1.1 Features

Added CEC support
