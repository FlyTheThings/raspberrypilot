#!/usr/bin/env sh
cd python
python setup.py clean
python setup.py install
cd ..
cd pythonflight
python setup.py clean
python setup.py install
cd ..
cd server
make
cd ..
cp -f build/uavlink_server/uavlink_server /usr/local/bin/uavlink_server
cp -f python/raspberrypilot/service.py /usr/local/bin/raspberrypilot
chmod 755 /usr/local/bin/raspberrypilot
cp -f uavlinkserver_init /etc/init.d/uavlinkserver
chmod 755 /etc/init.d/uavlinkserver
update-rc.d uavlinkserver defaults
cp -f raspberrypilot_init /etc/init.d/raspberrypilot
chmod 755 /etc/init.d/raspberrypilot
update-rc.d raspberrypilot defaults
rm -Rf ./build
rm -Rf pythonflight/build
rm -Rf python/build
