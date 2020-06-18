################################################################################
#
# host-openjdk-bin
#
################################################################################

HOST_OPENJDK_BIN_VERSION_MAJOR = 13.0.2
HOST_OPENJDK_BIN_VERSION_MINOR = 8
HOST_OPENJDK_BIN_VERSION = $(HOST_OPENJDK_BIN_VERSION_MAJOR)_$(HOST_OPENJDK_BIN_VERSION_MINOR)
HOST_OPENJDK_BIN_SOURCE = OpenJDK13U-jdk_x64_linux_hotspot_$(HOST_OPENJDK_BIN_VERSION).tar.gz
HOST_OPENJDK_BIN_SITE = https://github.com/AdoptOpenJDK/openjdk13-binaries/releases/download/jdk-$(HOST_OPENJDK_BIN_VERSION_MAJOR)%2B$(HOST_OPENJDK_BIN_VERSION_MINOR)
HOST_OPENJDK_BIN_LICENSE = GPL-2.0+ with exception
HOST_OPENJDK_BIN_LICENSE_FILES = legal/java.prefs/LICENSE legal/java.prefs/ASSEMBLY_EXCEPTION

# unpack200 has an invalid RPATH and relies on libzlib. When
# host-libzlib is installed on the system, the error "ERROR: package
# host-libzlib installs executables without proper RPATH: will occur.
# Because unpack200 is a deprecated tool, removing it to fix this
# issue is safe.
define HOST_OPENJDK_BIN_INSTALL_CMDS
	mkdir -p $(HOST_DIR)/bin
	cp -dpfr $(@D)/bin/* $(HOST_DIR)/bin/
	mkdir -p $(HOST_DIR)/lib
	cp -dpfr $(@D)/lib/* $(HOST_DIR)/lib/
	$(RM) -f $(HOST_DIR)/bin/unpack200
endef

$(eval $(host-generic-package))
