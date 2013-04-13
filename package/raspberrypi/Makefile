#this is pretty gross but works for now

build_package: clean
	mkdir ../../build/raspberrypi
	cp -R ../../server ../../build/raspberrypi
	cp -R ../../build/uavobject-synthetics/pythonflight ../../build/raspberrypi
	cp -R ../../flight/python ../../build/raspberrypi
	rm -Rf ../../build/raspberrypi/pythonflight/build
	cp install ../../build/raspberrypi
clean:
	rm -Rf ../../build/raspberrypi