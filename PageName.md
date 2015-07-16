# Setting up and Building Software #

Generally working from a Linux machine makes life easier.

**Building a Linux machine that can build raspberrypilot**
  * `sudo apt-get update`
  * `sudo apt-get install build-essential`
  * `sudo apt-get install git`
  * `apt-get install libsdl1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev`
  * `sudo apt-get install libusb-dev`
  * `sudo apt-get install libudev-dev`
  * `sudo apt-get install libphonon-dev`
  * git clone raspberry pilot by following Option 2: "Stay authenticated with .netrc instructions" at:

http://code.google.com/p/raspberrypilot/source/checkout

  * create a .netrc file in your home directory with nano, vi or gedit and add to it:
> > _machine code.google.com login **gmail\_name@gmail.com** password **google\_generated\_password**_

  * logout and back in to read .netrc
  * make a directory such as _/projects_ if you wish: `mkdir projects`
  * `cd projects`
  * `git clone https://code.google.com/p/raspberrypilot/`
  * `sudo apt-get install curl`
  * `sudo apt-get install gcc-arm-none-eabi`
  * `make arm_sdk_install`
  * `sudo apt-get install qtcreator`
  * `make -j4 gcs`

**Get openocd to flash firmware**

  * `git clone git://openocd.git.sourceforge.net/gitroot/openocd/openocd`
  * `cd openocd`
  * `git checkout v0.6.1`
  * `sudo apt-get install libtool`
  * `sudo apt-get install automake`
  * `sudo apt-get install autoconf`
  * `sudo apt-get install texinfo`

  * `./install`
  * `./configure --enable-maintainer-mode --enable-stlink`
  * `make`
  * `sudo make install`

next we need to fix permissions on the stlink usb-device so we can use it without sudo:

  * `sudo useradd -G plugdev YOURUSERNAME`
  * `sudo gedit /etc/udev/rules.d/10-stlink.rules`

and add the following line:

ATTRS{idProduct}=="3748", ATTRS{idVendor}=="0483", MODE="666", GROUP="plugdev"

also stlink is based on buggy usb-storage class device, the linux usb-storage driver must be told to ignore it to turn it off temporarily do:

  * `modprobe -r usb-storage && modprobe usb-storage quirks=483:3748:i`

to do it permanently

  * `sudo gedit /etc/modprobe.d/stlink.conf` (creating the file)

add "options usb-storage quirks=483:3748:i"

to switch to qt4 if you downloaded qt5:
export QTCHOOSER\_RUNTOOL=qtconfig
export QT\_SELECT=4

**Running GCS once it's built**

Run GCS from this batch file, it will run the binary:


> _..../rapberrypilot/build/openpilotgcs\_release/bin/openpilotgcs_

**How to add uavobjects:**
  1. Add xml file
  1. Edit ground/openpilotgcs/src/plugins/uavobjects.pro to include reference to the c and h files
  1. Rebuild gcs	(don't edit/delete objects without rebuilding gcs)

**Disable the pi's the console serial port in preparation for Zach's server:**
  * `cd /etc`
  * `sudo nano inittab`
  * Comment out the line at the bottom
  * #T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100
  * `cd /boot`
  * `sudo nano cmdline.txt`
  * Delete any references to ttyAMA0,115200 such as "console=ttyAMA0,115200" and/or "kgdboc=ttyAMA0,115200"

**Put Zach's server onto the pi:**
  * `cd package/raspberrypi`
  * `make`
  * `scp -r ./{_your_path_}/raspberrypilot/build/raspberrypi pi@raspberrypi:`
  * `ssh pi@raspberrypi *password*`
  * `cd raspberrypi`
  * `chmod a+x install.sh`
  * `sudo ./install.sh`
  * Reboot the pi, it should be running uav\_link\_server and python:
  * $ `ps aux | grep uav`

**pi        2289  1.3  0.0   2124   392 ?        S    08:23   0:32 /usr/local/bin/uavlink\_server**

$ `ps aux | grep py`

**pi        2241  0.4  1.4  19460  7144 ?        Sl   08:23   0:09 python /usr/local/bin/raspberrypilot
nobody    2283  0.0  0.1   2012   636 ?        Ss   08:23   0:00 /usr/sbin/thd --daemon --triggers /etc/triggerhappy/triggers.d/ --socket /v**

**To connect a raspberrypi by wifi:**
  * `ssh pi@network.raspberrypi.name`
  * `cd /etc/network`
  * `sudo nano interfaces`

```
auto lo
iface lo inet loopback
iface eth0 inet dhcp
auto wlan0
allow-hotplug wlan0
iface wlan0 inet dhcp
  wpa-ssid "your_network"
  wpa-psk "your_network_password"
#allow-hotplug wlan0
#iface wlan0 inet manual
#wpa-roam /etc/wpa_supplicant/wpa_supplicant.conf
#iface default inet dhcp
```

Raspberrypilot Build Instructions:
|make all\_clean|Clean up everything including GCS|
|:--------------|:--------------------------------|
|make -j4 uavobjects| Build New object?               |
|make -j4 raspberrypilot|Build firmware?                  |
|make -j4 gcs   |Build Ground Control Station (takes a while)|
|make fw\_raspberrypilot\_clean|Clean up the firmware for rebuild|
|make fw\_raspberrypilot|Build firware but don't send     |
|make bl\_raspberrypilot\_program|Build and send bootloader to raspberrypilot target|
|make fw\_raspberrypilot\_program|Build and send firmware to raspberrypilot target|
|make all       |Build uavobjects, all\_ground, all\_flight|


Convert a logfile to matlab format:
make uavobjects\_matlab

This will automatically generate a log parser under your build output directory (./build/uavobjects-synthetics/matlab/OPLogConvert.m).  This file is specifically generated to decode the logs for exactly this version of firmware.
Converting the log file from a raw binary to a .mat file

In Octave, run this:

addpath('/path/to/op/source/build/uavobjects-synthetics/matlab');

OPLogConvert ('/path/to/op/log/file/OP-2011-12-03\_12-15-46.opl');