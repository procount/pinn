#### How to access the shell or SSH into NOOBS

1. Even if the NOOBS GUI is launched, the busybox shell can still be accessed by pressing CTRL-ALT-F2. Use CTL-ALT-F1 to get back to the gui. 

2. You can now also SSH into the NOOBS basic busybox shell. To enable this feature, add `ssh` to the argument list in the recovery.cmdline. SSH is also accessible from the rescueshell, but allow 5 seconds after boot to allow the network to establish. NOOBS SSH does not store any keys so it takes a while to connect at first after each boot as it generates new keys for that session.

Use the username of 'root' and password 'raspberry' to login to the shell via the console or SSH.

### How to install an OS from USB

Due to the increasing size of OSes, it is becoming increasingly difficult to store their compressed installation files on the NOOBS SD card. So NOOBS-full now only includes one default Raspbian installation and the remainder are downloadable using a network connection.
To counter this, NOOBS supports storing the compressed OSes on a USB stick aswell. This has the following advantages:
1. The OSes are available to install when a network is not available.
2. They do not take up any valuable space on the SD card.
The OSes should be stored on the USB stick in the same format as they would be on a NOOBS SD card:
1. Each OS should be stored in its own folder beneath the /os/ folder.
2. They should contain compressed images of their partitions in .tar.xz format
3. All supporting json files etc. shall also be included.
The USB stick should be inserted into the RPi before NOOBS is booted. Ideally it should be connected to the RPi and not via a usb hub as this may introduce a delay preventing the USB stick from being recognised.
If the same OS is available on the SD card, the USB stick and the network, only the most recent version is displayed.

### How to install OSes from an alternative source
 
An alternative source of OSes to be downloaded from the internet can be specified by adding <b>alt_image_source=http://newurl.com/os_list.json</b> to the argument list in recovery.cmdline, where `newurl.com` is the name of the alternative server and `os_list.json` is the list of information about the alternative OSes. This list of OSes will be added to the default download list. This can be useful for storing the default repository on a local LAN server, or for adding your own list of OSes to those available in NOOBS.

To suppress the default URL and only use the alternative image source for downloading OS images, add <b>no_default_source</b>. Using this option without `alt_image_source` will prevent all internet downloads and just allow local OSes on the SD card to be listed.

### How to use with Gert's VGA666 DPI display screen

The VGA666 adaptor connects to the GPIO pins and allows a VGA display to be attached to the RPi. The normal VGA666 installation instructions should be followed to allow it to work with NOOBS.

Create a config.txt file with the following lines in it:

add dtoverlay=VGA666
enable_dpi_lcd=1
display_default_lcd=1
dpi_group=<group> (e.g. dpi_group=1, or dpi_group=2)
dpi_mode=<mode> (e.g. dpi_mode=28 - see tvservice for a list of possible modes)

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
