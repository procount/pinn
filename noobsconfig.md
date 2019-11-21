How to customise an OS installation
===================================

Allows you to customise the installation of an OS installed by PINN.

## Introduction
After PINN installs the standard boot and root images to the SD card, this modification allows a custom set of files to be copied, decompressed and extracted onto any partition of the distro being installed. Therefore, several variants of the same OS can be provided from the same installation files, thus saving space.

### Flavours
First, a brief explanantion about `flavours`.

Early versions of NOOBS introduced the concept of flavours to install the same version of Raspbian in 3 different ways: to boot to the command line, desktop or straight into Scratch, without the need to store the installation files for 3 separate versions of the same OS. These days, this functionality is provided by raspi-config and no other distro made use of the functionality of flavours. So PINN enhanced this functionality to provide more comprehensive yet simple customisation features.

In this documentation I use `flavour` to represent the "os name" plus a "flavour" suffix. Typically a `flavour` name is the os name appended with a dash and the flavour suffix. See `flavours.json` if it exists for your distro.

### Customisation Files

Three types of customisation files can be used. They must all have the same base filename of `(flavour)_(partitionName)` and differ in the three extensions that may be used:
* `(flavour)_(partitionName).tar`
* `(flavour)_(partitionName).tar.xz`
* `(flavour)_(partitionName).txt`

(Any spaces in the flavour or partitionName should be replaced by underscores)  
The above files can be used individually or in combination. If more than one is found they are processed in the above order. They should be placed in the os/(distro) folder.  
The .tar file will be un-tarred to the root of the appropriate partition.  
The .tar.xz file is assumed to be a compressed tar file and is treated in the same way as a .tar file after decompression.  
The .txt file is a control file that PINN will process to customise the OS. In its simplest form it provides a list of files to be installed over the OS, each on a separate line. Additionally, it can include controls to change folders, specify additional .tar files to install, or even include other .txt control files. In this way, many customisations can be provided individually and a master control file canbe used to selectively install a subset of them. This are described in more detail in Advanced Customisations below.

<!--
### Repository Organisation
* helpers - contains helper scripts to help create custom tarballs
* Examples - contains example customisations
-->

## Detailed Description

### What can it be used for?
This method is for simple customisations of a distro. It behaves a bit like applying a patch file to the distro after it has been installed. As such it can replace or add files to the distro, but not delete existing files.  
Some examples of its use are therefore:
* setting up wifi so that it works "out of the box"
* installing script files to install other packages (either manually or on first boot - like raspi-config)
* provide standard configurations for config.txt (instead of requiring raspi-config to be executed on first execution)
* install a standard set of "lesson support materials" into the `/home/pi` folder.

### Benefits
* ideal for small customisations
* does not require the creation of a full customised OS
* independent of PINN distributions
* the same customisations can be quickly and easily applied to a subsequent update of an OS distribution.

### What it doesn't do
Currently it does not execute user-defined scripts, it simply copies/replaces existing files.
Whilst it could be used to copy entire packages onto the distro, this use is not recommended and it is suggested to follow the existing instructions to create a customised OS if the modifications are extensive.

### Rationale
I like to use the latest version of PINN whenever it is released, ensuring I get all the latest updates and distro versions.
However, whenever I update to a new version, or reinstall a distro from PINN, I end up having to apply the same customisations and installations over and over again.
I've now learnt to create scripts for these common operations and a custom tarball that I can use to overwrite files like `/etc/network/interfaces` etc.
But it's still an extra step to perform after PINN has finished. So I thought about how this could be applied automatically.

The ability to patch using a flavour would be ideal for schools and others, it could save building full custom images and avoids having to rebuild it for every new release of Raspbian. It would also allow Zero-config setups, for instance you'd be able to configure PINN to auto-install a distro which has direct wifi networking enabled out of the box.
 
## How does the PINNconfig customisation work?

After PINN has installed a distro, it runs the `partition_setup.sh` script for that distro. Then it looks for files in the distro folder with a basename of (flavour)_(partitionName) and an extension of either `.tar`, `.tar.xz` or `.txt`.

Note that `(flavour)` and `(partitionName)` must exactly match (in characters and case) the names provided in the `*.json` files that describe each OS in order for them to be recognised. Any spaces should be replaced with underscores.
So, for the standard v1.3.4 installation of PINN, the following base filenames were valid:
<pre>
Arch_boot
Arch_root
Data_Partition_data
OpenELEC_Storage
OpenELEC_System
Pidora_boot
Pidora_rootfs
Raspbian_boot
Raspbian_root
Raspbian_-_Boot_to_Scratch_boot
Raspbian_-_Boot_to_Scratch_root
RaspBMC_boot
RaspBMC_root
</pre>

If a file is found with the appropriate base filename above and one of the following extensions, then the following actions are carried out:
A filename ending in `.tar` is untarred onto the appropriate partition.
A filename ending in `.tar.xz` is decompressed, and untarred onto the appropriate partition.

So to add/replace some custom files on your distro, you just need to create a tar ball with the correct name, optionally compress it using XZ, drop it into the distro folder and install the distro as usual - simples! Some helper scripts are provided to help create these tarballs (see the Helpers folder).

(A filename with the appropriate basename ending in `.txt` is processed according to the Advanced Customisation section described below.)

If you add any other distros, flavours or OS partition names, you need to name the custom files according to the filename format above.

### How to Create a Custom Tarball

These instructions assume you already have a working installation of a distro on the RPi and you want to capture the customisations you have made for future installations of the same distro.

1. Firstly you need to start from a working installation of the distro. I suggest you use PINN to install the distro first.
2. Modify any existing, or create any new files that you want to be included in your customisation.
3. For manageability I suggest creating a simple text file that contains a list of all the new/modified files that you want, with one fullpathname on each line.
For example:

    ```
    $>sudo nano files.txt
    /etc/network/interfaces
    /etc/init.d/rc.local
    ```

4. Now create the tar ball using this text file as input, as follows:

    ```
    $>sudo tar cvf &lt;flavour&gt;_&lt;partitionName&gt;.tar -T files.txt
    $>sudo xz &lt;flavour&gt;_&lt;partitionName&gt;.tar
    ```

    (See the Helpers folder for alternative ways to do this using scripts).  

5. Copy the tarball to the appropriate OS folder.

### Advanced Customisation

Sometimes, you may want to have greater control over your customisations. Maybe you want to:
* segregate them into separate tar files for distinct uses.
* give them better names according to their use.
* have easier control over which ones should be installed.
* share files between different flavours of the same distro.
* provide raw files that are not tarred or compressed, e.g. by direct copying on to the recovery partition using Windows.
For these and other uses, the (flavour)_(partitionName).txt file can be useful.

This text file contains a list of files (one per line) to be installed to the distro AFTER the above `.tar` or `.tar.xz` files have been installed.  In this way some tweaks can be applied to existing .tar files without having to modify/rebuild them.

Any line beginning with an '@' symbol is processed as a control instead of simply being copied (see below)

When directly copying files, you may need to provide additional information in the form of optional fields on the same line.

There are five fields as follows (separated by spaces):

	Filename destination attributes user group

Only the filename field is mandatory and it is the only field used for TAR and XZ files; the others can be omitted and default values will then be applied. If you want to omit a field but follow it with another field, then use a '#' instead to keep the relative placement of the fields.

Blank lines will be ignored.  
Comments can be added by starting the line with a '#' in the first column, or adding the comment after field 5.

Here are some examples to illustrate this:
* Readme.txt
* Readme.txt # 0644
* nc-helpers.tar
* wifi/interfaces /etc/network 0644 root root
* wifi/wpa_supplicant.conf /etc/wpa_supplicant 0600 root root

(The last 2 examples above show how using this direct copy technique, it is possible to specify the '/etc/network/interfaces' and '/etc/wpa_supplicant/wpa_supplicant.conf' files for easy wifi setup from Windows, whilst specifying the correct attributes and avoiding the problem of creating Tar files from Windows.)

#### Current Source Folder

Filenames to be copied are referenced relative to the Current Source Folder, which initially is the folder where the distro is located, or as specified by the configpath argument.

So for example, if a file called readme.txt is needed to be copied from the Raspbian distro folder of /os/Raspbian, it is only necessary to specify the filename `readme.txt` because the /os/Raspbian folder is the Current Source Folder.

If the file is put it in a subfolder then this must be specified.  
e.g.   
To copy the file `/os/Raspbian/folder/readme.txt`, use `folder/readme.txt`  
The folder name is only used to locate the source file. It is not used as part of the destination folder name.

The Current Source Folder may be changed by the use of the @push=, @pop controls and possibly as a consequence of including another control file using @filename.txt. (see the controls section). The Current Source Folder may also be a valid URL on the internet.

If a filename begins with the '/' symbol, then the Current Source Folder is reset back to the initial location and the filename is made relative to that location again.

#### configpath=folder

Flavour customisations are normally found in the same location as the Distro installation files, but if you want to customise the installation of an OS that is stored on the internet, it is often not possible to locate the customisations there. In this case it is possible to specify a new initial Custom Source Folder using the configpath parameter. This can be specified in recovery.cmdline. 

For a local OS, this configpath can be overridden by providing a configpath entry in an OSes os.json file.

Configpath can be specified as a valid URL or an absolute folder reference.

#### Filename
This is the name of the file you want to copy from the recovery partition, relative to the current source folder. This field is mandatory.

If a filename has embedded spaces in it, then it must be enclosed in double quotation marks:
e.g.
* "my wifi/interfaces" /etc/network 0644 root root

The filename may also be a valid URL on the internet.

#### Destination
This is the name of the destination folder where the file is to be stored on the target partition, relative to the root. E.g. `/home/pi`. It must begin with a slash, but have no trailing slash.
This field is optional. If not specified, then the root directory is used.
If you need to include a destination pathname that includes embedded spaces,then enclose this in double quotations.

#### Attributes
This specifies the attributes to be applied to the new file using the chmod command. e.g. `0644`. I guess options such as `+x` would also work but I've not tried.
@filename.tar - untarred 
@filename.tar.xz - untarred
#### User
This specifies the new user of the file e.g. `pi` or `root` etc.

#### Group
This specifies the new group of the file e.g. `root`

### TXT file controls

In addition to the list of files to copy, TXT files can contain controls which are identified by the use of the '@' symbol in column 1 of each line. The following controls are defined:

#### @filename.tar  
#### @filename.tar.xz  

.tar and and .tar.xz files preceded by the '@' symbol are decompressed to the root folder of the target partition, rather than just being copied.  
NOTE: If you are already using a `(flavour)_(partitionName).tar(.xz)` file, you should NOT specify it in this text file as well, otherwise it would be installed twice.

#### @filename.txt
This is used to include another TXT control file and process it.

#### @push=folder

This pushes the Current Source Folder onto a stack and changes the Current Source Folder to the given folder argument. The folder may also be a valid URL.

#### @pop

This restores the previous value of the Current Source Folder from the stack.

#### @/&lt;filename&gt;

This resets the Current Source Folder to it's initial value.

### Selecting a Method of customisation
It can be confusing to decide which type of customisation to use, so here is a short guide.
* TAR files are very convenient when there are many related files to be installed, possibly in different folders, and where file permissions or ownership are important. They capture the contents, ownership and permissions of each file into a single TAR file which is then easily managed. So they are best created directly on an existing distro on the RPi. Not so many compression programs allow you to create a TAR file on Windows (7-zip can), but nevertheless, the file permissions and attributes cannot be set. Plain TAR files are not compressed and append 2 blocks of 512 bytes at the end. So the minimum size of a TAR file is 10kB. This can be quite an overhead if you only want to install a couple of small script files.
* TAR.XZ files are TAR files that have been compressed using the XZ program. They therefore retain the advantages of the TAR file's manageability, but also avoid the TAR file's largish size. It is best to compress them on the RPi, but it can be done on another Linux distro, but not so easily on Windows.
* If you want to create your customisation on Windows, referencing files from the TXT file may be the easiest method. This does not provide any compression, nor does it collect the separate files into one single file for manageability. However, it does provide control of user permissions and ownership. This is also a convenient method if you only want to install 1 or 2 small files. If you are creating script files on Windows, be careful to choose Linux line endings. Some editing programs, like Notepad++ allow you to visualise the line endings and change them from Windows to Linux and vice versa.



