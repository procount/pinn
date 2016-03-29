
# PINN (PINN Is Not NOOBS)
## Change documentation v1.9p1

#### An easy enhanced Operating System installer for the Raspberry Pi

PINN is designed to make it easy to select and install operating systems for the Raspberry Pi without having to worry about manually imaging your SD card.

The latest version of [PINN-lite](http://downloads.sourceforge.net/projects/pinn/pinn-lite.zip) can be downloaded from [sourceforge](http://www.sourceforge.net/projects/pinn).

## Advanced Usage (for experts and teachers)

### How to install OSes from an alternative source

If your Pi is connected to the internet, PINN will present a list of OSes available from the Raspberry Pi website. An alternative source of OSes to be downloaded from the internet can be specified by adding <b>alt_image_source=http://newurl.com/os_list_v3.json</b> to the argument list in recovery.cmdline, where `newurl.com` is the name of the alternative server and `os_list_v3.json` is the list of information about the alternative OSes. This list of OSes will be added to the default download list. This can be useful for storing the default repository on a local LAN server, or for adding your own list of OSes to those available in PINN.

To suppress the default URL and only use the alternative image source for downloading OS images, add <b>no_default_source</b>. Using this option without `alt_image_source` will prevent all internet downloads and just allow local OSes on the SD card or USB memory stick to be listed.

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

===

## Troubleshooting

#### How to access the shell or SSH into PINN

1. Even if the PINN GUI is launched, the busybox shell can still be accessed by pressing CTRL-ALT-F2. Use CTL-ALT-F1 to get back to the gui.

2. You can now also SSH into the PINN basic busybox shell. To enable this feature, add `ssh` to the argument list in the recovery.cmdline. SSH is also accessible from the rescueshell, but allow 5 seconds after boot to allow the network to establish. PINN SSH does not store any keys so it takes a while to connect at first after each boot as it generates new keys for that session.

Use the username of 'root' and password 'raspberry' to login to the shell via the console or SSH.

#### How to use PINN Headlessly (VNC)

It is possible to access PINN without a keyboard and mouse using VNC over a network.

Append `vncinstall` to the argument list in the `recovery.cmdline` file which is found in the root PINN directory. Using `forcetrigger` in addition can be useful as it is not always easy to connect quickly enough to see the splash screen and hold the shift key remotely. This will force you to use VNC to continue the boot process into your installed OS on each boot, so it is best to remove `forcetrigger` once the required OS's have been installed. Alternatively you could use `gpiotriggerenable` (see below).

To connect over VNC you need to know the IP address of the Pi. Connect to port 5900 of this IP address. Using tightvnc, you need to specify this as <ip address>:5900 e.g. 192.168.1.0:5900.

When VNC is selected there will be no GUI present on any attached display as all output is redirected over the VNC network connection.


