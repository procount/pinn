***************************
Freescale i.MX8QM MEK board
***************************

This file documents the Buildroot support for the NXP i.MX8QM MEK board.

You will find a reference to the board on nxp.com:
https://www.nxp.com/design/development-boards/i.mx-evaluation-and-development-boards/i.mx-8quadmax-multisensory-enablement-kit-mek:MCIMX8QM-CPU

You can also find the get started guide here:
https://www.nxp.com/document/guide/get-started-with-the-i.mx-8quadmax-mek:GS-iMX-8QM-MEK

Build
=====

First, configure Buildroot for the i.MX8QM MEK board:

$ make freescale_imx8qmmek_defconfig

Build all components:

$ make

You will find in output/images/ the following files:
  - ahab-container.img
  - bl31.bin
  - boot.vfat
  - fsl-imx8qm-mek.dtb
  - Image
  - imx8-boot-sd.bin
  - mkimg.commit
  - mx8qm-mek-scfw-tcm.bin
  - mx8qm-val-scfw-tcm.bin
  - mx8qm-a0-ddr4-scfw-tcm.bin
  - mx8qm-a0-mek-scfw-tcm.bin
  - mx8qm-a0-val-scfw-tcm.bin
  - mx8qm-ddr4-scfw-tcm.bin
  - rootfs.ext2
  - rootfs.ext4
  - rootfs.tar
  - sdcard.img
  - u-boot-atf.bin
  - u-boot.bin
  - u-boot-hash.bin

Create a bootable SD card
=========================

To determine the device associated to the SD card have a look in the
/proc/partitions file:

  cat /proc/partitions

Buildroot prepares a bootable "sdcard.img" image in the output/images/
directory, ready to be dumped on a SD card. Launch the following
command as root:

  dd if=output/images/sdcard.img of=/dev/<your-sd-device>

*** WARNING! This will destroy all the card content. Use with care! ***

For details about the medium image layout, see the definition in
board/freescale/common/imx/genimage.cfg.template_imx8.

Boot the i.MX8QM MEK board
==========================

To boot your newly created system:
- insert the SD card in the SD slot of the board;
- Configure the boot switch as follows:
SW2:	OFF	OFF	ON	ON	OFF	OFF
- put a micro USB cable into the Debug USB Port and connect using a terminal
  emulator at 115200 bps, 8n1;
- power on the board.

Enjoy!
