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
