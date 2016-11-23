
# PINN (PINN Is Not NOOBS)
#### An easy enhanced Operating System installer for the Raspberry Pi

PINN is designed to make it easy to select and install operating systems for the Raspberry Pi without having to worry about manually imaging your SD card.

The latest version of [PINN-lite](http://downloads.sourceforge.net/projects/pinn/pinn-lite.zip) can be downloaded from [sourceforge](http://www.sourceforge.net/projects/pinn).

![alt text](screenshots/os_installed.png "PINN Interface")

<sup>*NOTE: The list of OSes in this image is indicative only. It will vary according to your Raspberry Pi model and the availability of OSes on the installation sources.</sup>

### About
On first boot PINN will format your SD card and allow you to select which OSes you want to install from a list. This OS list is automatically generated from both locally available OSes (i.e. those contained in the `/os` directory on disk or an attached USB memory stick) or those available from a remote repository (network connection required).

Only the latest version of each OS will ever be displayed meaning that you can be sure that you have installed the most up-to-date release of your selected OS.

On any subsequent boot you can then press the SHIFT key to enter the PINN interface and easily reinstall your choice of OSes.

The PINN interface provides the following functionality:
- <b>Install</b>: Installs the selected OSes onto your SD card. Changing this selection erases all OSes currently installed.
- <b>Edit Config</b>: Opens a text editor allowing the cmdline and config for the selected installed OS to be edited.
- <b>Online Help</b>: [Networking Required] Open a browser that displays the Raspberry Pi Help page ( http://www.raspberrypi.org/help/ ), allowing people to quickly access help and troubleshooting.
- <b>Exit</b>: Quits PINN and reboots the Pi into the OS boot menu.
- <b>Language Selection</b>: Allows you to select the language to be displayed.
- <b>Keyboard Layout Selection</b>: Allows you to select the keyboard layout to be used.
- <b>Display Mode Selection</b>: By default, PINN will output over HDMI at your display's preferred resolution, even if no HDMI display is connected. If you do not see any output on your HDMI display or are using the composite output, press 1, 2, 3 or 4 on your keyboard to select HDMI preferred mode, HDMI safe mode, composite PAL mode or composite NTSC mode respectively.

Note that all user settings (language, keyboard layout, display mode) will persist between reboots and will also be automatically passed to the installed OSes. This means that if you can see the PINN interface on your display device then you should be able to see the OS CLI/GUI when it boots too!
### Setup

To set up a blank SD card with PINN:
- Format an SD card that is 4GB or greater in size as FAT (see instructions on how to do this below)
- Download and extract the files from the PINN zip file. (Windows built-in zip features may have trouble with this file. If so, use another program such as 7zip.)
- Copy the extracted files onto the SD card that you just formatted so that this file is at the root directory of the SD card.
<b> Please note that in some cases it may extract the files into a folder, if this is the case then please copy across the files from inside the folder rather than the folder itself.</b>

On first boot the "RECOVERY" FAT partition will be automatically resized to a minimum and a list of OSes that are available to install will be displayed.

### Operating System Choice

PINN is only available in 1 format:
- `PINN-Lite` does not include any Operating Systems at all.

#### OS Network Download

PINN allows additional Operating Systems to be downloaded from a remote repository. To do this, the Raspberry Pi must be connected to a wired network, or it can connect over Wifi using the [Raspberry Pi USB wifi dongle](https://www.raspberrypi.org/products/usb-wifi-dongle/) or the Raspberry Pi 3 Model B built-in wifi.

Once connected, the Pi will only show a list of Operating Systems that are appropriate to your Pi Model. If you want to see ALL available OSes, edit the `recovery.cmdline` file in the root PINN directory and append `showall` to the arguments list.

####Wired Networks

If a wired ethernet cable is plugged into the Pi before PINN starts, PINN will connect via DHCP to a remote download repository and present a list of available Operating Systems that are available for installation.

#### Wifi Networks

If you have the official [Rapberry Pi USB wifi Dongle](https://www.raspberrypi.org/products/usb-wifi-dongle/), or are using the Raspberry Pi 3 Model B with built-in wifi, the wifi icon on the PINN toolbar will be available. Click on this to select your Wifi SSID network and enter the wifi password.

![alt text](screenshots/wifi_selector.png "Select your wifi network and enter the password")

#### Network Drivers

From v1.9.4, the following network drivers have been added to support a wider range of wifi and network adapters:
* zd1211rw
* rtl8192cu - fixed in v1.9.5
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

from v2.0
* rtl8188eu - for hubpiwi and others
from v2.1
* IoTpHAT

### How to Format an SD card as FAT

For <b>Windows</b> users, we recommend formatting your SD card using the SD Association's Formatting Tool, which can be downloaded from https://www.sdcard.org/downloads/formatter_4/ You will need to set "FORMAT SIZE ADJUSTMENT" option to "ON" in the "Options" menu to ensure that the entire SD card volume is formatted - not just a single partition. For more detailed and beginner-friendly formatting instructions, please refer to http://www.raspberrypi.org/quick-start-guide

The SD Association's Formatting Tool is also available for <b>Mac</b> users although the default OSX Disk Utility is also capable of formatting the entire disk (select the SD card volume and choose "Erase" with "MS-DOS" format).

For <b>Linux</b> users we recommend `gparted` (or the command line version `parted`). (Update: Norman Dunbar has written up the following formatting instructions for Linux users: http://qdosmsq.dunbar-it.co.uk/blog/2013/06/NOOBS-for-raspberry-pi/ )

===

### Screenshots

#### OS Installation

Simply select the checkbox next to each OS you want to install using either a mouse or keyboard (arrow keys to traverse the list, enter to toggle the selected OS's checkbox), then click the "Install" icon (or press "i" on your keyboard) to install the selection. The icons shown on the right of the list indicate whether the OS is being installed from the SD card (SD card icon) or from the online OS repository (Ethernet icon).

![alt text](screenshots/os_selected.png "Select your choice of OSes to install")

<sup>*NOTE: The list of OSes in this image is indicative only. It will vary according to your Raspberry Pi model and the availability of OSes on the installation sources.</sup>


#### Online Help via Web Browser

The built-in Arora web browser allows you to easily get help via the Raspberry Pi Forums (wired network connection required).

![alt text](screenshots/browser.png "Search the Raspberry Pi forums for help via the built-in web browser")

#### Easy Config File Editor

The built-in config file editor allows you to edit the config file of the OS currently highlighted in the OS list. This allows you to easily add license keys to different OS installs through the same interface.

Note that the output mode selected by the user through pressing one of number keys 1 to 4 (for HDMI preferred, HDMI VGA, Composite PAL and Composite NTSC respectively), will be automatically set in the `config.txt` files of your installed OSes. This means that you shouldn't have to worry about manually changing your display settings to get your installed OS to display correctly on your display device.

![alt text](screenshots/config_editor.png "Easily edit the config files of any installed OS")

#### Installer Slideshow

An installer slideshow guides you through your first steps with each OS while it installs.

![alt text](screenshots/installer_slides.png "An installer slideshow guides you through your first steps with each OS")

#### OS Boot Selector

After multiple OSes have been installed, you can select which OS to boot through this selection window that is automatically displayed. PINN will remember your choice and boot this OS by default unless a different option has been selected within 10 seconds.

Note that if only one OS is installed then the boot selector will not be displayed and the OS will be automatically booted.

![alt text](screenshots/boot_select.png "Easily select which OS you want to boot from a list of those currently installed")

#### Self Update

If PINN is connected to the internet, it will check for the latest version available for download. If a newer version is available, a dialog box will pop-up showing the latest version number and the recent cahnges made. You then have the option to cancel the update, download it, or ignore this version.
If you cancel, the dialog box will keep popping up to remind you each time PINN starts.
If you select ignore, this version will be ignored and no dialogs will popup again until another later version is released.
If you choose to download this update, the zip file will be downloaded and will overwrite your current version of PINN. PINN will then reboot the RPi and restart PINN.

#### CEC Control (v2.1.1 onwards)

PINN can be controlled with a TV remote if the HDMI TV has CEC support.
The CEC remote keys are mapped as follows:
To simulate a mouse the following keys are used:
- Cursor Up/Down/Left/Right - moves the mouse cursor in each direction
- Select - mouse click
For quicker selection the following keys may be easier:
- Channel Up - previous list item
- Channel Down - next list item
- 0 key - Enter Key
- Exit Key - Escape
- 9 key - Toggles the advanced menu
- 1-8 keys - select the menu buttons numbered from left to right

Any of the above remote keys can be used instead of the keyboard shift key on startup to access the main menu.

==

## Advanced Usage (for experts and teachers)

### How to install OSes from an alternative source

If your Pi is connected to the internet, PINN will present a list of OSes available from the Raspberry Pi website. An alternative source of OSes to be downloaded from the internet can be specified by adding <b>alt_image_source=http://newurl.com/os_list_v3.json</b> to the argument list in recovery.cmdline, where `newurl.com` is the name of the alternative server and `os_list_v3.json` is the list of information about the alternative OSes. This list of OSes will be added to the default download list. This can be useful for storing the default repository on a local LAN server, or for adding your own list of OSes to those available in PINN.

To suppress the default URL and only use the alternative image source for downloading OS images, add <b>no_default_source</b>. Using this option without `alt_image_source` will prevent all internet downloads and just allow local OSes on the SD card or USB memory stick to be listed.

### How to Automatically Install an OS

Even if you are using your Pi without a display, you can still use PINN to easily install an OS of your choice. To set up PINN to automatically and silently (i.e. without requiring any user input) install a specific OS, follow these steps:

1. Copy the OS folder for the OS you want to install into the `/os` dir (or alternatively delete all other OSes contained in the `/os` dir so that only your chosen OS remains.

2. If the OS you want to automatically install has multiple flavours available, edit the `flavours.json` file so that it only contains the flavour entry that you want to install.

3. Edit the `recovery.cmdline` file in the root PINN directory and append `silentinstall` to the arguments list.

When you now boot your Pi using an SD card containing the modified version of PINN that you just created, it will automatically install the OS you chose and boot into it after the installation has finished.

### How to install an OS from USB

Due to the increasing size of OSes, it is becoming increasingly difficult to store their compressed installation files on the PINN SD card. PINN-lite contains no OS installation files by default, but you can copy selected OSes into the /os folder if you wish.

To counter this lack of SD card space, PINN supports storing the compressed OSes on a USB stick as well. This has the following advantages:

 1. The OSes are available to install when a network is not available.
 2. They do not take up any valuable space on the SD card.

The OSes should be stored on the USB stick in the same format as they would be on a PINN SD card:

 1. Each OS should be stored in its own folder beneath the /os/ folder.
 2. They should contain compressed images of their partitions in .tar.xz format
 3. All supporting json files etc. shall also be included.

The USB stick should be inserted into the RPi before PINN is booted. Ideally it should be connected to the RPi and not via a usb hub as this may introduce a delay preventing the USB stick from being recognised.
If the same OS is available on the SD card, the USB stick and the network, only the most recent version is displayed.

### Installation Progress

During the installation of the operating systems, PINN will write the percentage completed to a text
file called /tmp/progress. The format of this file is an integer (0-100) followed by a space, 
a '%' symbol and a line feed. It is updated only when the progress changes by at least 1%. 
Sometimes PINN will not know the maximum size, so in this case it shows the amount data written in MBs.
This feature mimics the progress dialog on the display and is useful in headless installations.

### Background shell script

To make use of the installation progress feature a background shell script can be used. If a /background.sh script exists, it will be executed in the background whilst PINN runs. This can be used to read the /tmp/progress file and display the progress on the serial port, or a GPIO display etc.

### Installing Arch linux

Arch linux distros use extended file attributes that are not stored in a standard TAR archive.
Therefore they use BSDTAR to create their distro archives which are not compatible with PINN.
PINN adds BSDTAR support to install these files, allowing ARCH linux to be installed.
The partition_setup.sh file has been modified to allow ARCH linux distros to be
installed directly from their website without converting into PINN's XZ format.

To install Arch it is necessary to add another repository to PINN.
Edit recovery.cmdline and add `alt-image-source=https://raw.githubusercontent.com/procount/pinn-os/master/os/os_list_v3.json`
(This is included by default)

### Preconfiguring a WiFi network

If you already know your WiFi details, you can preconfigure PINN to use it straight away. Put a copy of your `wpa_supplicant.conf` file on the PINN root partition and PINN will read it and store it in its settings for all future uses. The file will be renamed to `wpa_supplicant.conf.bak` to provent it overwriting any subsequent changes you make to the wifi networks using the GUI. 

### How to create a custom OS version

There are two main use cases for which you may want to create a custom version of one of the standard OS releases that is suitable for installation via PINN:
- If you are a teacher wanting to easily deploy a custom OS release containing pre-defined set of packages and files onto a number of SD cards (e.g. to provision a class set of Raspberry Pi's or quickly restore a Raspberry Pi back to custom "factory" settings).
- If you want to be able to back up your existing installed packages and files so that any future OS re-install does not force you back to a clean install.

The following steps allow you to create a modified copy of one of the standard OS releases that contains your custom files, packages and settings.

1. Download a base version of PINN-lite from http://downloads.sourceforge.net/projects/pinn/pinn-lite.zip

2. Extract the PINN-lite zipfile

3. Navigate to the `os` directory

4. Create a copy of the folder containing the OS release that you want to modify and rename it with a custom name.

5. Edit the following fields in the `os.json` file contained in the folder that you just created
  1. "name" - replace the name of the base OS with the name of your custom OS version
  2. "description" - replace the description of the standard OS install with one for your custom OS version

6. [Optional] Rename or replace the existing `<OS>.png` icon file with one matching the name of your custom OS version

7. [Optional] Replace the PNG image files in the `slides` and `slides_vga` directory with your own custom installer slides

8. Edit the following fields in the `partitions.json` file contained in the folder that you just created
  1. "partition_size_nominal" - replace the numerical value with the size of the paritions in your custom OS version
  2. "uncompressed_tarball_size" - replace the numerical value with the size of your filesystem tarballs when uncompressed

9. Replace the `.tar.xz` root and boot filesystem tarballs with copies created from your custom OS version (these instructions assume you're only using a single OS at a time with PINN - they won't work if you're running multiple OSes from a single SD card). The name of these tarballs needs to match the labels given in `partitions.json`.
  1. To create the root tarball you will need to run `tar -cvpf <label>.tar /* --exclude=proc/* --exclude=sys/* --exclude=dev/pts/*` from within the root filesystem of your custom OS version. You should then compress the resulting tarball with `xz -9 -e <label>.tar`.
  2. To create the boot tarball you will need to run `tar -cvpf <label>.tar .` at the root directory of the boot partition of your custom OS version. You should then compress the resulting tarball with `xz -9 -e <label>.tar`.

### How to customise an OS install (noobsconfig)

PINN can be used as a recovery program, so if your OS gets corrupted, messed up or otherwise goes wrong, you can reinstall a clean version of the OS and start again. On the other hand, starting again from scratch can be painful, especially if you can't remember how you set up your perfect OS environment.

Creating a custom OS version (see above) for your preferred setup is one option, but it can look daunting to do, and would need to be repeated if you want to update it to include the latest build of the OS.

[Noobsconfig](http://github.com/procount/noobsconfig) is an alternative option that creates a customised OS by installing a standard OS, but then copies your own files over the top to add your own customisations according to your selected "flavour". These additional files are specified in a simple text file that controls their installation. For example, these can be simple configuration files, workshop exercises, tutorials or wifi setup files. Or they could be scripts that are configured to run on first boot to further configure your OS by automatically installing your favourite packages.

By keeping the customisations separate from the OS distro means:

 1. A custom installation can be created out of a standard OS installation plus some additional files, without having to create a full customised OS
 2. The same customisations can be applied to a newer version of the standard OS installation without having to rebuild a custom OS installation.
 3. By using simple configuration files, it is easy to add files to configure your OS
 4. Different customisation "flavours" of an OS can be created as a master installation card, and the required ones can be selected and installed as required. This can save a lot of SD card space compared to storing a different full customised OS for each flavour.

PINN now includes direct support for `noobsconfig` so it is no longer necessary to add the noobsconfig files to your PINN installation. Just add your configuration and customisations files and install your "flavoured" OS. Please see (https://github.com/procount/noobsconfig) for full documentation on how to use this feature.

The [PiKitchen](http://www.pihardware.com/guides/pi-kitchen) is an example of how comprehensive these configurations can be if you want them to be.

### How to change the default Language, Keyboard layout, Display mode or Boot Partition etc.

Edit the `recovery.cmdline` file in the root PINN directory and append the following arguments where relevant:
- `lang=<two-letter language code>` (e.g. `lang=de` or `lang=en`)
- `keyboard=<two-letter layout code>` (e.g. `keyboard=de` or `keyboard=us`)
- `display=<display mode number>` (e.g. `display=1` or `display=3`)
- `partition=<partition_number>` (e.g. `partition=6`)
- `showall` (shows all available OSes regardless of your Raspberry Pi model)

Note that these defaults will be overwritten by any changes made in the GUI to these settings.

### How to bypass the Recovery splashscreen and boot directly into a fixed partition

After you have installed your chosen OSes, add the following file to the root directory of PINN to force the indicated partition to be booted at power-on.

1. Add a text file named `autoboot.txt` to the root directory of PINN.

2. Add `boot_partition=<partition number>` to the file and save it to disk.

This will also prevent the splashscreen from being displayed at boot. The partition number can be found by running `sudo fdisk -l` the partition will be one of the FAT32 partitions `/dev/mmcblk0p6` would be partition 6. Note that once an `autoboot.txt` file is present, there's then no way to force the PINN GUI to display, until you delete (or rename) the `autoboot.txt` file.

### How to use with the Raspberry Pi Touch Display

If PINN detects you are using the [Raspberry Pi Touch Display](https://www.raspberrypi.org/products/raspberry-pi-touch-display/), it will enable the following functionality:
- A tap on the touchscreen can activate PINN aswell as holding the shift key down.
- A tap on the touchscreen will simulate a mouse click
- A longpress on the touchscreen will simulate a mouse double-click.

### How to use with Gert's VGA666 DPI display screen

The VGA666 adaptor connects to the GPIO pins and allows a VGA display to be attached to the RPi. The normal VGA666 installation instructions should be followed to allow it to work with PINN.

Create a config.txt file with the following lines in it:
```
dtoverlay=VGA666
enable_dpi_lcd=1
display_default_lcd=1
dpi_group=<group> (e.g. dpi_group=1, or dpi_group=2)
dpi_mode=<mode> (e.g. dpi_mode=28 - see tvservice for a list of possible modes)
```
In addition, the recovery.cmdline should have `disablesafemode` added as a parameter since without it, the VGA666 will force PINN into safe mode due to its connections to the GPIO header.

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

If you have changed your login password for an OS and forget what it is, PINN will allow you to reset it back to its default or set it to a new value.

1. On the PINN screen, select the new `Advanced` menu.
* Select the installed OS that you want to reset the password in.
* Selecting the Password button will display a dialog box to enter the new password details.
* Enter the username you want to change the password of.
* Enter the new password (twice). Both copies must match. The second will be displayed red if it is different.
* The password strength meter is a useful indication of how good a password you have created.
* The `Use Dafault` button will enter the default username and password for the selected OS.
* Tick the `show password` box to display the passwords on the screen.

### How to Clone an SD Card

Raspbian has recently added an SD Card Copier tool called piclone, which is great. For anyone concerned about it cloning a live system, 
it has now been ported into PINN so that it can clone the SD card offline, whilst the OS is not running. 
1. On the PINN screen, select the new `Advanced` menu.
* Insert a second SD card into the Pi using a USB card reader
* Select the Clone SD card button
* Select your internal SD card as the source (/dev/mmcblk0) and the SD card in the USB reader as the desination (usually /dev/sda)
* Selct OK to clone your SD card.

All partitions will be copied onto the second card whilst the last partition will be sized fit the remainder of the SD card. 
In this way your existing data can be migrated to a smaller or larger SD card.

Note that if you have installed multiple OSes using PINN, only the last partition on the SD card will be resized. 

===

## Troubleshooting

#### What to do if your SHIFT keypress isn't detected

Try pressing shift only when the grey splashscreen is displayed rather than holding it from boot up.

#### How to boot into "Safe Mode"

To boot into a basic busybox shell rather than launching the PINN GUI, you can *either*:

1. Append `rescueshell` to the argument list in the `recovery.cmdline` file which is found in the root PINN directory. Exiting from this shell will now enter the PINN recovery program.

2. Insert a physical jumper between pins 5 & 6 of GPIO header P1. If you have external hardware or an addon board connected to the GPIO header, you may find that pin 5 is being pulled low and accidentally triggering "Safe Mode". To prevent this you can append `disablesafemode` to the argument list in the `recovery.cmdline` file which is found in the root PINN directory.

#### How to access the shell or SSH into PINN

1. Even if the PINN GUI is launched, the busybox shell can still be accessed by pressing CTRL-ALT-F2. Use CTL-ALT-F1 to get back to the gui.

2. You can now also SSH into the PINN basic busybox shell. To enable this feature, add `ssh` to the argument list in the recovery.cmdline. SSH is also accessible from the rescueshell, but allow 5 seconds after boot to allow the network to establish. PINN SSH does not store any keys so it takes a while to connect at first after each boot as it generates new keys for that session.

Use the username of 'root' and password 'raspberry' to login to the shell via the console or SSH.

#### How to use PINN Headlessly (VNC)

It is possible to access PINN without a keyboard and mouse using VNC over a network.

Append `vncinstall` to the argument list in the `recovery.cmdline` file which is found in the root PINN directory. Using `forcetrigger` in addition can be useful as it is not always easy to connect quickly enough to see the splash screen and hold the shift key remotely. This will force you to use VNC to continue the boot process into your installed OS on each boot, so it is best to remove `forcetrigger` once the required OS's have been installed. Alternatively you could use `gpiotriggerenable` (see below).

To connect over VNC you need to know the IP address of the Pi. Connect to port 5900 of this IP address. Using tightvnc, you need to specify this as <ip address>:5900 e.g. 192.168.1.0:5900.

When VNC is selected there will be no GUI present on any attached display as all output is redirected over the VNC network connection.

Appending `vncshare` to the argument list instead of `vncinstall` will share the screen between the locally attached display and redirect it over the VNC network connection simultaneously.

#### How to enable using the GPIO to trigger entering Recovery Mode

To force Recovery Mode to be entered on boot and to show the PINN interface, you normally press the `SHIFT` key during bootup. If you don't have a keyboard or the `SHIFT` keypress isn't being detected, you should complete the following steps to force the PINN interface to be displayed on boot:

1. Append `gpiotriggerenable` to the argument list in the `recovery.cmdline` file which is found in the root PINN directory.
2. Reboot

To force Recovery Mode being entered on boot, connect GPIO pin 3 on header P1 to GND (pin 25). If GPIO pin 3 remains unconnected then it will boot through to the installed OS as normal.

#### How to force Recovery Mode being entered on boot (overrides GPIO or keyboard input)

Alternatively, if you are unable to use either the GPIO or keyboard to trigger entering Recovery Mode, you can:

1. Append `forcetrigger` to the argument list in the `recovery.cmdline` file which is found in the root PINN directory.
2. Reboot

Note that with this option enabled, the Recovery Mode will be displayed <b>every</b> time you boot from your PINN card (until you edit `recovery.cmdline` again).

#### How to disable using the keyboard to trigger entering Recovery Mode

In some rare cases, you may find that PINN incorrectly detects a `SHIFT` keypress from your keyboard regardless of the presence of user input. In such cases it may be helpful to disable using the keyboard to trigger Recovery Mode being entered.

To prevent a `SHIFT` keypress from entering Recovery Mode on boot (maybe you have a problematic keyboard which is erroneously triggering every time you boot), you can:

1. Append `keyboardtriggerdisable` to the argument list in the `recovery.cmdline` file which is found in the root PINN directory.
2. Reboot

#### How to change display output modes

By default, PINN will output over HDMI at your display’s preferred resolution, even if no HDMI display is connected. If you do not see any output on your HDMI display or are using the composite output, press 1, 2, 3 or 4 on your keyboard to select HDMI preferred mode, HDMI safe mode, composite PAL mode or composite NTSC mode respectively.

If you don't have a keyboard, you can still change the display mode used by PINN through editing the `recovery.cmdline` file in the root PINN directory prior to first boot and appending the following argument:
- `display=<display mode number>` (e.g. `display=1` or `display=3`)

===

## How to Rebuild PINN

Note that this will require a minimum of 6GB free disk space.

#### Get Build Dependencies

On Ubuntu:

`sudo apt-get install build-essential rsync texinfo libncurses-dev whois unzip bc qt4-linguist-tools`

#### Run Build Script

`./BUILDME.sh`

Buildroot will then build the software and all dependencies, putting the result in the `output` directory.

Buildroot by default compiles multiple files in parallel, depending on the number of CPU cores you have.

If your build machine does have a quad core CPU, but relatively little RAM, you may want
to lower the number to prevent swapping:
- `cd buildroot ; make menuconfig`
- "Build options" -> "Number of jobs to run simultaneously"

If your build machine also has some QT5 components, it is useful to `export QT_SELECT=4` before building to ensure the QT4 component versions are selected.

## How to run your Build

In order to setup an SD card with a newly built version of PINN, you will need to:
- Format an SD card that is 4GB or greater in size as FAT
- Replace the `/os` directory in `/output` with the copy contained in the release version of PINN (see above for download links)
- Copy the files in the `/output` directory onto the SD card

## About the Buildroot infrastructure

To add extra packages: `cd buildroot ; make menuconfig`

Recovery software packaging is in: `buildroot/package/recovery`

Kernel configuration used: `buildroot/kernelconfig-recovery.armv6` and `kernelconfig-recovery.armv7`

Main differences with bcmrpi_defconfig:
- `CONFIG_BLK_DEV_INITRD=y` - initramfs support
- `CONFIG_INPUT_EVDEV=y` - evdev support built-in
- `CONFIG_USB_HID=y` - usb HID driver built-in
- All modules disabled.
- (This has changed significantly from v1.5 to use a squashfs)

## Modifying Qt source

Source is in the `recovery` folder.
Be aware that user interface screens will appear larger in Qt Creator then when deployed on the Pi, can
raise font sizes 2 points to compensate.

Several constants can be changed in `config.h`

Wrap code that calls Qt Embedded specific classes (such as QWSServer) between
```C
#ifdef Q_WS_QWS
```
and
```C
#endif
```
so that the project also compiles and can be tested under standard Qt.

## Adding/Updating Translations

References:

http://qt-project.org/doc/qt-4.8/i18n-source-translation.html

http://qt-project.org/doc/qt-4.8/linguist-manual.html

To set up a git pre-commit hook to automatically update the translation files, run the following commands in the project root:
- `chmod +x pre-commit-translation-update-hook.sh`
- `cp pre-commit-translation-update-hook.sh .git/hooks/pre-commit`

To add a new translation:
- Add to `recovery/recovery.pro` the following: `TRANSLATIONS += translation_<languagecode>.ts`
- Run `lupdate recovery/recovery.pro` which extracts strings from the source code and generates/updates the *.ts* files.
- The *.ts* can then be sent to the translator, opened in Qt Linguist and filled in.
- Add a line for the *.ts* file in to `recovery/icons.qrc`, but substitute *.ts* extension with *.qm* . This file contains a list
  of resource files that will be embedded into the application's executable during build.
- Add a flag icon for your language from http://www.famfamfam.com/lab/icons/flags/ flag icon collection or if it
  doesn't have the one you need, you may use some other small png icon for it. Copy the icon file to the `recovery/icons`
  folder and add a line for it into `recovery/icons.qrc` as well.


### Legal compliance

Copyright (c) 2013, Raspberry Pi
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of the Raspberry Pi Foundation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#### Third party licenses:

Recovery software directly links to:
- Qt libraries, available under LGPL and commercial license.

Currently used icon sets:
- http://www.fatcow.com/free-icons - Creative commons Attribution license
- http://www.famfamfam.com/lab/icons/flags - "These flag icons are available for free use for any purpose with no requirement for attribution."
- http://www.oxygen-icons.org/ - Available under Creative Common Attribution-ShareAlike 3.0 and LGPL license

Licenses of utility software build by buildroot:
Type `cd buildroot ; make legal-info` to generate a list, which will be available under `output/legal-info`.
