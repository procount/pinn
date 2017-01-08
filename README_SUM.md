# PINN (PINN is not NOOBS)
#### An enhanced Operating System installer for the Raspberry Pi

PINN is a version of the NOOBS Operating System Installer for the Raspberry Pi with the following additional features:

## Change History

### V2.1.2

- **Default Boot**    - A default OS can be set in a multi-boot setup for quicker boots.
- **Bootmenutimout**  - The timeout of the boot selection dialog can be changed.
- **SHIFT key**       - The PINN interface can be reached using the Left mouse button as well as shift.

### V2.1.1

- **CEC Support**     - Added CEC support to control PINN with a TV remote

### V2.1

- **IoTpHAT support** - Updated to match NOOBS v2.1 for IoTpHAT wifi

### V2.0

- **Network Drivers** - Added the popular RTL8188eu wifi driver (for HubPiWi and others)
- **New Firmware**    - Updated Firmware and Kernel to match NOOBS 2.0

### V1.9.5 

- **Self-Update**     - Notification and download of new releases

#### Bugfixes

- **Network Drivers** - Fixed the popular RTL8188cus wifi driver.
- **RPI3 ACT_LED**    - Now displays correctly on SD card access


### V1.9.4

- **Network Drivers** - A number of additional wifi and ethernet drivers have been added.

#### Bugfixes

- **Dialog Box**      - The initialisation dialog box is removed under network errors


### V1.9.3

- **Clone SD Card**   - Copy the SD card to another card in a USB reader (BETA)
- **Rescue shell**    - Exiting the rescue shell now enters the PINN recovery program instead of another shell.
- **VNCSHARE**        - Sets up PINN to use VNC at the same time as an attached screen

#### Bugfixes

- **https**           - Fixed the ability to use https protocol (which was preventing Arch from installing sometimes).
- **wifi config**     - A user supplied wpa_supplicant.conf file is now ALWAYS copied to /settings and then renamed to wpa_supplicant.conf.bak so that it does not continually overwrite any manual changes made through the GUI.


### V1.9.2 

- **Password**        - Change or reset your password for each OS.
- **DSI/HDMI**        - Auto-switching of DSI/HDMI screen with HDMI taking priority 

#### BugFixes

- **NoobsConfig**     - Config filenames now now have all spaces converted to underscores 
- **Multiple Os**     - Multiple installed OSes can now be selected again. 
- **DHCP**            - ClientID is now used instead of DUID to request an IP address 


### V1.9.1 (based on NOOBS v1.9)

- **USB support**     - store your OS installations on USB stick to avoid wasting SD card space and avoid downloading each time.
- **Alternative Src** - Install OSes from an alternative website, or local webserver
- **ARCH support**    - Install the Arch Linux OS directly from the Arch website
- **VGA666 support**  - Alow PINN to be used with Gert's VGA666 adaptor
- **NOOBSCONFIG support** - Allow customisation of an OS installation as it installs
- **SSH support**     - SSH remotely into you PINN installation.
- **Progress**        - View your installation progress via alternative means


Documentation on how to use PINN can be found **[here](README_PINN.md)**, which includes all of the original NOOBS documentation.

If you are already familiar with NOOBS' features, you may prefer to read the reduced documenation that just contains **[PINN's new features](changes.md)** alone.

The latest version of **[PINN-lite](http://sourceforge.net/projects/pinn/files/pinn-lite.zip)** can be downloaded from [sourceforge](http://www.sourceforge.net/projects/pinn).

Source code is available from [github](https://github.com/procount/pinn)

see  [github](https://github.com/procount/pinn) for full information
