#this is pretty gross but works for now

build_package: clean
	mkdir ../../build/raspberrypi
	cp -R ../../server ../../build/raspberrypi
	cp -R ../../build/uavobject-synthetics/pythonflight ../../build/raspberrypi
	cp -R ../../flight/python ../../build/raspberrypi
	rm -Rf ../../build/raspberrypi/pythonflight/build
	awk '{ sub("\r", ""); print }'  < raspberrypilot_init > ../../build/raspberrypi/raspberrypilot_init
	awk '{ sub("\r", ""); print }'  < uavlinkserver_init > ../../build/raspberrypi/uavlinkserver_init
	awk '{ sub("\r", ""); print }'  < install.sh > ../../build/raspberrypi/install.sh
clean:
	rm -Rf ../../build/raspberrypi