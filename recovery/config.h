#ifndef CONFIG_H
#define CONFIG_H

/* Version number displayed in the title bar */
#define VERSION_NUMBER "2.4.5"

/* Color of the background */
// #define BACKGROUND_COLOR  Qt::white
#define BACKGROUND_COLOR  QColor(0xde, 0xde, 0xde)

/* Highlight color of installed OS */
#define INSTALLED_OS_BACKGROUND_COLOR  QColor(0xef,0xff,0xef)

/* Places background.png resource file in center of screen */
#define CENTER_BACKGROUND_IMAGE

/* Enable language selection */
#define ENABLE_LANGUAGE_CHOOSER

/* Website launched when launching Arora */
#define HOMEPAGE  "http://www.raspberrypi.org/help/"

#define AGENT "Wget/1.15 (linux-gnu)"

/* Location to download the list of available distributions from
 * Multiple lists can be specified by space separating the URLs */
#define DEFAULT_REPO_SERVER  "http://downloads.raspberrypi.org/os_list_v3.json"

/* Location of data for self-updating */
#define BUILD_URL       "http://downloads.sourceforge.net/project/pinn/BUILD-DATA"
#define UPDATE_URL      "http://downloads.sourceforge.net/project/pinn/pinn-lite.zip"
#define README_URL      "http://downloads.sourceforge.net/project/pinn/README.md"
#define GROUP_URL       "http://raw.githubusercontent.com/procount/pinn-os/master/os/overrides.json"
#define BUILD_CURRENT   "/mnt/BUILD-DATA"
#define BUILD_NEW       "/tmp/BUILD-DATA"
#define BUILD_IGNORE    "/settings/BUILD-DATA"
#define UPDATE_NEW      "/tmp/pinn-lite.zip"
#define README_NEW      "/tmp/README.md"
#define GROUP_NEW       "/tmp/overrides.json"
#define GROUP_CURRENT   "/mnt/overrides.json"
#define INFO_URL        "https://github.com/procount/pinn"

/* Size of recovery FAT partition in MB when using reformat drive initialization method. */
#define RESCUE_PARTITION_SIZE  63

/* Files that are currently on the FAT partition are normally saved to memory during
 * repartitioning.
 * If files they are larger than number of MB, try resizing the FAT partition instead */
#define MAXIMUM_BOOTFILES_SIZE  64

/* Partitioning settings */
#define PARTITION_ALIGNMENT  8192
#define PARTITION_GAP  2
/* Allow partitions to be shrunk PARTITION_GAP sectors
   if that prevents having a 4 MiB gap between the next one */
#define SHRINK_PARTITIONS_TO_MINIMIZE_GAPS

#define SETTINGS_PARTNR   5
#define SETTINGS_PARTITION_SIZE  (32 * 2048 - PARTITION_GAP)

/* If the image name matches this exactly, mark it as recommended */
#define RECOMMENDED_IMAGE "Raspbian"

/* RiscOS magic */
#define RISCOS_OFFSET_KEY "riscos_offset"
#define RISCOS_OFFSET (1760)
#define RISCOS_SECTOR_OFFSET (RISCOS_OFFSET * 2048)

#define RISCOS_BLOB_FILENAME  "/mnt/riscos-boot.bin"
#define RISCOS_BLOB_SECTOR_OFFSET  (1)

/* Maximum number of partitions */
#define MAXIMUM_PARTITIONS  62

#define DEFGROUP "General"

#endif // CONFIG_H
