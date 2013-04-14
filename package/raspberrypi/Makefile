#this is pretty gross but works for now

build_package: clean
	mkdir ../../build/raspberrypi
	cp -R ../../server ../../build/raspberrypi
	cp -R ../../build/uavobject-synthetics/pythonflight ../../build/raspberrypi
	cp -R ../../flight/python ../../build/raspberrypi
	rm -Rf ../../build/raspberrypi/pythonflight/build
	tr -d '\15\32'  < raspberrypilot_init > ../../build/raspberrypi/raspberrypilot_init
	tr -d '\15\32'  < uavlinkserver_init > ../../build/raspberrypi/uavlinkserver_init
	tr -d '\15\32'  < install > ../../build/raspberrypi/install
clean:
	rm -Rf ../../build/raspberrypi