Run the emulation with:

  qemu-system-ppc -M g3beige -kernel output/images/vmlinux -drive file=output/images/rootfs.ext2,format=raw -append "console=ttyS0 rootwait root=/dev/hda" -serial stdio -net nic,model=rtl8139 -net user

The login prompt will appear in the terminal that started Qemu. The
graphical window is the framebuffer.

Tested with QEMU 2.12.0
