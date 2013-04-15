#this is pretty gross but works for now

build_package: clean
	mkdir ../../build/raspberrypi
	cp -R ../../server ../../build/raspberrypi
	cp -R ../../build/uavobject-synthetics/pythonflight ../../build/raspberrypi
	cp -R ../../flight/python ../../build/raspberrypi
	rm -Rf ../../build/raspberrypi/pythonflight/build
	perl -p -e \'s/\r$//\'  < raspberrypilot_init > ../../build/raspberrypi/raspberrypilot_init
	perl -p -e \'s/\r$//\'  < uavlinkserver_init > ../../build/raspberrypi/uavlinkserver_init
	perl -p -e \'s/\r$//\'  < install.sh > ../../build/raspberrypi/install.sh
clean:
	rm -Rf ../../build/raspberrypi