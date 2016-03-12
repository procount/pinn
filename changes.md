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
