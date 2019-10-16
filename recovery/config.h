#ifndef CONFIG_H
#define CONFIG_H

/* Version number displayed in the title bar */
#define VERSION_NUMBER "3.0mcn2"

#define MC 1
#define SOCKSERVER "/tmp/server"
#define SOCKCLIENT "/tmp/client"

#define AGENT "Wget/1.15 (linux-gnu)"

/* Color of the background */
// #define BACKGROUND_COLOR  Qt::white
//#define BACKGROUND_COLOR  QColor(0xde, 0xde, 0xde)
#define BACKGROUND_COLOR  Qt::black

/* Highlight color of installed OS */
#define INSTALLED_OS_BACKGROUND_COLOR  QColor(0xef,0xff,0xef)

/* Places background.png resource file in center of screen */
#define CENTER_BACKGROUND_IMAGE

/* Enable language selection */
#define ENABLE_LANGUAGE_CHOOSER

/* Website launched when launching Arora */
#define HOMEPAGE  "https://www.microcenter.com/site/content/tech-support.aspx"

/* Location to download the list of available distributions from
 * Multiple lists can be specified by space separating the URLs */
#define DEFAULT_REPO_SERVER  "http://raw.githubusercontent.com/procount/pinn-os/master/os/os_retro.json"

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
#define RECOMMENDED_IMAGE "Raspbian Full"

/* RiscOS magic */
#define RISCOS_OFFSET_KEY "riscos_offset"
#define RISCOS_OFFSET (1760)
#define RISCOS_SECTOR_OFFSET (RISCOS_OFFSET * 2048)

#define RISCOS_BLOB_FILENAME  "/mnt/riscos-boot.bin"
#define RISCOS_BLOB_SECTOR_OFFSET  (1)

/* Maximum number of partitions */
#define MAXIMUM_PARTITIONS  32

#define AGENT "Wget/1.15 (linux-gnu)"

#endif // CONFIG_H
