#!/bin/bash
# Bash script to rebuild recovery

#set -e
#set -x

KERNEL="6.1"

BUILDROOT_OLD="buildroot"
BUILDROOT_NEW="buildroot-2023.02"

# Final directory where NOOBS files will be copied to
NOOBS_OUTPUT_DIR="output"
export QT_SELECT=4

function get_package_version {
  PACKAGE=$1
  CONFIG_FILE="package/$PACKAGE/$PACKAGE.mk"
  if [ -f "$CONFIG_FILE" ]; then
    CONFIG_VAR=$(echo "$PACKAGE-version" | tr '[:lower:]-' '[:upper:]_')
    grep -E "^$CONFIG_VAR\s*=\s*.+$" "$CONFIG_FILE" | tr -d ' ' | cut -d= -f2
  fi
}

function get_package_version6 {
  PACKAGE=$1
  CONFIG_FILE="../${BUILDROOT_NEW}/package/$PACKAGE/$PACKAGE.mk"
  if [ -f "$CONFIG_FILE" ]; then
    CONFIG_VAR=$(echo "$PACKAGE-version" | tr '[:lower:]-' '[:upper:]_')
    grep -E "^$CONFIG_VAR\s*=\s*.+$" "$CONFIG_FILE" | tr -d ' ' | cut -d= -f2
  fi
}


function update_github_package_version {
    PACKAGE=$1
    GITHUB_REPO=$2
    BRANCH=$3
    CONFIG_FILE="package/$PACKAGE/$PACKAGE.mk"
    if [ -f "$CONFIG_FILE" ]; then
        OLDREV=$(get_package_version $PACKAGE)
        if [ -z "$OLDREV" ]; then
            echo "Error getting OLDREV for $PACKAGE";
        else
            REPO_API=https://api.github.com/repos/$GITHUB_REPO/git/refs/heads/$BRANCH
            GITREV=$(curl -s ${REPO_API} | awk '{ if ($1 == "\"sha\":") { print substr($2, 2, 40) } }')
            if [ -z "$GITREV" ]; then
                echo "Error getting GITREV for $PACKAGE ($BRANCH)";
            else
                if [ "$OLDREV" == "$GITREV" ]; then
                    echo "Package $PACKAGE ($BRANCH) is already newest version"
                else
                    CONFIG_VAR=$(echo "$PACKAGE-version" | tr '[:lower:]-' '[:upper:]_')
                    sed -ri "s/(^$CONFIG_VAR\s*=\s*)[0-9a-f]+$/\1$GITREV/" "$CONFIG_FILE"
                    echo "Package $PACKAGE ($BRANCH) updated to version $GITREV"
                fi
            fi
        fi
    else
        echo "$CONFIG_FILE doesn't exist"
    fi
}


function get_kernel_version {
    CONFIG_FILE=../${BUILDROOT_NEW}/.config
    CONFIG_VAR=BR2_LINUX_KERNEL_VERSION
    grep -E "^$CONFIG_VAR=\".+\"$" "$CONFIG_FILE" | tr -d '"' | cut -d= -f2
}


function update_github_kernel_version {
    PACKAGE=kernel
    GITHUB_REPO=$1
    BRANCH=$2
    CONFIG_FILE=.config
    if [ -f "$CONFIG_FILE" ]; then
        OLDREV=$(get_kernel_version)
        if [ -z "$OLDREV" ]; then
            echo "Error getting OLDREV for $PACKAGE";
        else
            REPO_API=https://api.github.com/repos/$GITHUB_REPO/git/refs/heads/$BRANCH
            GITREV=$(curl -s ${REPO_API} | awk '{ if ($1 == "\"sha\":") { print substr($2, 2, 40) } }')
            if [ -z "$GITREV" ]; then
                echo "Error getting GITREV for $PACKAGE ($BRANCH)";
            else
                if [ "$OLDREV" == "$GITREV" ]; then
                    echo "$PACKAGE ($BRANCH) is already newest version"
                else
                    CONFIG_VAR=BR2_LINUX_KERNEL_CUSTOM_REPO_VERSION
                    sed -ri "s/(^$CONFIG_VAR=\")[0-9a-f]+(\")$/\1$GITREV\2/" "$CONFIG_FILE"
                    CONFIG_VAR=BR2_LINUX_KERNEL_VERSION
                    sed -ri "s/(^$CONFIG_VAR=\")[0-9a-f]+(\")$/\1$GITREV\2/" "$CONFIG_FILE"
                    echo "$PACKAGE ($BRANCH) updated to version $GITREV"
                fi
            fi
        fi
    else
        echo "$CONFIG_FILE doesn't exist"
    fi
}

function select_bits64 {

    #BITS is the version we want to build
    BITS=$1 #0=32, 1=64

    #PREVBITS is the previous version
    PREVBITS=`expr 1 - $BITS`

    #If we haven't already built this version
    if ! [ -e .bits$BITS ]; then

        #if we have built the previous version then delete it
        if [ -e .bits$PREVBITS ]; then
            rm .bits$PREVBITS
            make clean
        fi
	make
        #otherwise assume no version has been built, so no need to make clean
    else #Just build the Linux kernel
        make linux-reconfigure
    fi
    touch .bits$BITS
}

function select_kernelconfig {
    ARCH=$1
    CONFIG_FILE=.config
    if [ $ARCH == "armv8" ]; then
        cp brconfig-pinn.$ARCH .config
    else
        CONFIG_VAR=BR2_LINUX_KERNEL_CUSTOM_CONFIG_FILE
        VAR_PREFIX=kernelconfig-recovery
        sed -ri "s/(^$CONFIG_VAR=\"$VAR_PREFIX\.).+(\")$/\1$ARCH\2/" "$CONFIG_FILE"
    fi

}


############################ CONFIGURE OPTIONS ######################################

# WARNING: don't try changing these - you'll break buildroot
BUILD_DIR="output/build"
IMAGES_DIR="output/images"

SKIP_KERNEL_REBUILD=0
SKIP_KERNEL_6=0
SKIP_KERNEL_7=0
SKIP_KERNEL_7L=0
SKIP_KERNEL_8=0
SKIP_RECOVERY_REBUILD=0
UPDATE_TS=0

for i in $*; do
    # Update raspberrypi/firmware master HEAD version in package/rpi-firmware/rpi-firmware.mk to latest
    if [ $i = "update-firmware" ]; then
        update_github_package_version rpi-firmware raspberrypi/firmware stable
    fi

    # Update raspberrypi/userland master HEAD version in package/rpi-userland/rpi-userland.mk to latest
    if [ $i = "update-userland" ]; then
        update_github_package_version rpi-userland raspberrypi/userland master
    fi

    # Update raspberrypi/linux rpi-$KERNEL.y HEAD version in buildroot/.config to latest
    if [ $i = "update-kernel" ]; then
        update_github_kernel_version raspberrypi/linux rpi-$KERNEL.y
    fi

    # Update language TS files
    if [ $i = "update-ts" ]; then
        UPDATE_TS=1
    fi

    # Option to build just recovery without completely rebuilding both kernels
    if [ $i = "skip-kernel-rebuild" ]; then
        SKIP_KERNEL_REBUILD=1
    fi
    # Option to build just recovery without completely rebuilding both kernels
    if [ $i = "skip-kernel-6" ]; then
        SKIP_KERNEL_6=1
    fi

    # Option to build just recovery without completely rebuilding both kernels
    if [ $i = "skip-kernel-7" ]; then
        SKIP_KERNEL_7=1
    fi

    # Option to build just recovery without completely rebuilding both kernels
    if [ $i = "skip-kernel-7l" ]; then
        SKIP_KERNEL_7L=1
    fi

    # Option to build just recovery without completely rebuilding both kernels
    if [ $i = "skip-kernel-8" ]; then
        SKIP_KERNEL_8=1
    fi

    # Option to build just recovery without completely rebuilding both kernels
    if [ $i = "skip-recovery-rebuild" ]; then
        SKIP_RECOVERY_REBUILD=1
    fi

    # Early-exit (in case we want to just update config files without doing a build)
    if [ $i = "nobuild" ]; then
        exit
    fi

    if [ $i = "help" ]; then
        echo "Options:"
	echo "update-firmware"
	echo "update-userland"
	echo "update-kernel"
	echo "update-ts"
	echo "skip-kernel-rebuild"
	echo "skip-kernel-6"
	echo "skip-kernel-7"
	echo "skip-kernel-7l"
	echo "skip-recovery-rebuild"
	echo "nobuild"
	exit
    fi
done

############################ BUILD 64-bit 6.1 KERNELS ######################################


cd ${BUILDROOT_NEW} #buildroot-2023.02

# Create output dir and copy files
FINAL_OUTPUT_DIR="../$NOOBS_OUTPUT_DIR"
mkdir -p "$FINAL_OUTPUT_DIR"
mkdir -p "$FINAL_OUTPUT_DIR/os"
cp -r ../sdcontent/* "$FINAL_OUTPUT_DIR"

if [ $SKIP_KERNEL_REBUILD -ne 1 ]; then

    if [ $SKIP_KERNEL_8 -ne 1 ]; then
        # Rebuild kernel for ARMv8
        select_kernelconfig armv8
        #select_bits64 1
        make linux-reconfigure
        make rpi-firmware
        # copy ARMv8 kernel
        cp "$IMAGES_DIR/Image"                         "$FINAL_OUTPUT_DIR/kernel8.img"
        for f in "$IMAGES_DIR/*.dtb";                   do cp $f "$FINAL_OUTPUT_DIR"; done
    else
        echo "Warning: kernel armv8 in '$NOOBS_OUTPUT_DIR' directory hasn't been updated"
    fi

else
    echo "Warning: 64-bit kernels in '$NOOBS_OUTPUT_DIR' directory haven't been updated"
fi

############################ COPY 6.1 OVERLAYS ######################################

cp -a  "$IMAGES_DIR/rpi-firmware/overlays" "$FINAL_OUTPUT_DIR/overlays6"


############################ BUILD 32-bit 5.10 KERNELS ######################################

# Let buildroot build everything in rootfs
cd ..
cd ${BUILDROOT_OLD} #buildroot


if [ $SKIP_KERNEL_REBUILD -ne 1 ]; then
    if [ $SKIP_KERNEL_7L -ne 1 ]; then
        # Rebuild kernel for ARMv7L
        select_kernelconfig armv7l
        #select_bits64 0
        make linux-reconfigure
        make rpi-firmware
        # copy ARMv7L kernel
        cp "$IMAGES_DIR/zImage"                         "$FINAL_OUTPUT_DIR/kernel7l.img"
        for f in "$IMAGES_DIR/*.dtb";                   do cp $f "$FINAL_OUTPUT_DIR"; done
    else
        echo "Warning: kernel armv7l in '$NOOBS_OUTPUT_DIR' directory hasn't been updated"
    fi

    if [ $SKIP_KERNEL_7 -ne 1 ]; then
        # Rebuild kernel for ARMv7
        select_kernelconfig armv7
        #select_bits64 0
        make linux-reconfigure
        make rpi-firmware
        # copy ARMv7 kernel
        cp "$IMAGES_DIR/zImage"                         "$FINAL_OUTPUT_DIR/kernel7.img"
        for f in "$IMAGES_DIR/*.dtb";                   do cp $f "$FINAL_OUTPUT_DIR"; done
    else
        echo "Warning: kernel armv7 in '$NOOBS_OUTPUT_DIR' directory hasn't been updated"
    fi

    if [ $SKIP_KERNEL_6 -ne 1 ]; then
        # Rebuild kernel for ARMv6
        select_kernelconfig armv6
        #select_bits64 0
        make linux-reconfigure
        make rpi-firmware
        # copy ARMv6 kernel
        cp "$IMAGES_DIR/zImage"                         "$FINAL_OUTPUT_DIR/kernel.img"
        for f in "$IMAGES_DIR/*.dtb";                   do cp $f "$FINAL_OUTPUT_DIR"; done
    else
        echo "Warning: kernel armv6 in '$NOOBS_OUTPUT_DIR' directory hasn't been updated"
    fi

else
    echo "Warning: 32-bit kernels in '$NOOBS_OUTPUT_DIR' directory haven't been updated"
fi

############################ BUILD 32-bit APPLICATION  ######################################


if [ $SKIP_RECOVERY_REBUILD -ne 1 ]; then
    # Delete buildroot build directory to force rebuild
    if [ -e "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR/recovery-$(get_package_version recovery)" || true
    fi
fi

make

# copy any updated translation files
if [ $UPDATE_TS -eq 1 ]; then
    cp $BUILD_DIR/recovery-$(get_package_version recovery)/*.ts ../recovery
fi

# copy rootfs
cp "$IMAGES_DIR/rootfs.squashfs" "$FINAL_OUTPUT_DIR/pinn.rfs"
#cp "$IMAGES_DIR/rootfs.cpio.lzo" "$FINAL_OUTPUT_DIR/pinn.rfs"



#Copy the firmware from the new kernels
#cd ..
#cd ${BUILDROOT_NEW} #buildroot
#pwd

# Ensure that final output dir contains files necessary to boot
cp "$IMAGES_DIR/rpi-firmware/start.elf" "$FINAL_OUTPUT_DIR/start.elf"
cp "$IMAGES_DIR/rpi-firmware/start4.elf" "$FINAL_OUTPUT_DIR/start4.elf"
cp "$IMAGES_DIR/rpi-firmware/fixup.dat" "$FINAL_OUTPUT_DIR/fixup.dat"
cp "$IMAGES_DIR/rpi-firmware/fixup4.dat" "$FINAL_OUTPUT_DIR/fixup4.dat"
cp "$IMAGES_DIR/rpi-firmware/config.txt" "$FINAL_OUTPUT_DIR"
cp "$IMAGES_DIR/rpi-firmware/bootcode.bin" "$FINAL_OUTPUT_DIR"
cp -a  "$IMAGES_DIR/overlays" "$FINAL_OUTPUT_DIR"
cp "$IMAGES_DIR/cmdline.txt" "$FINAL_OUTPUT_DIR"
cp "$IMAGES_DIR/config.txt" "$FINAL_OUTPUT_DIR"
touch "$FINAL_OUTPUT_DIR/RECOVERY_FILES_DO_NOT_EDIT"

# Create build-date timestamp file containing Git HEAD info for build
BUILD_INFO="$FINAL_OUTPUT_DIR/BUILD-DATA"
echo "Build-date: $(date +"%Y-%m-%d")" > "$BUILD_INFO"
echo "PINN Version: $(sed -n 's|.*VERSION_NUMBER.*\"\(.*\)\"|v\1|p' ../recovery/config.h)" >> "$BUILD_INFO"
echo "PINN Git HEAD @ $(git rev-parse --verify HEAD)" >> "$BUILD_INFO"
echo "rpi-userland Git master @ $(get_package_version rpi-userland)" >> "$BUILD_INFO"
echo "rpi-firmware Git master @ $(get_package_version6 rpi-firmware)" >> "$BUILD_INFO"
echo "rpi-linux Git rpi-$KERNEL.y @ $(get_kernel_version)" >> "$BUILD_INFO"

cd ..

#clear
echo "Build complete. Copy files in '$NOOBS_OUTPUT_DIR' directory onto a clean FAT formatted SD card to use."
