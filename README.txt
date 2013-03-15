Working on Ground:

When opening the .pro file in ground in Qt Creator go to project build settings and change the build directory to be ../build/ground

You now need python and git in your path

Todo:

Add uavlinkbridge task monitor
Add uavlinkbridge system alarm

Bad Changers to get it running:
dumped the boot loader, for now
really stripped out board config as it now is running on a stm32fdiscovery board



how to add uavobjects:
add xml file
edit ground/openpilotgcs/src/plugins/uavobjects.pro to include reference the c and h files
rebuild gcs
(don't edit/delete objects without rebuilding gcs)
