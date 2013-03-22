
import logging
import uavlink
import uavlink.uavobjects
import serial
import time


# Log everything, and send it to stderr.
logging.basicConfig(level=logging.DEBUG)



ser = serial.Serial("COM11",baudrate=57600)

uavtalk_server = uavlink.streamServer(("", 8079), uavlink.streamServerHandler)
uavtalk_server.register_rx_handler(lambda data: conn.sendSerial(1,data))

conn = uavlink.uavLinkConnection(None,ser.read,ser.write)
conn.register_rxStream_callback(1,lambda data: uavtalk_server.write(data) )
conn.start()

objMgr = uavlink.objManager(conn)
stats = objMgr.getObjByName("I2CStats")
stats.read()
print stats.nacks

while(True):
    stats.nacks += 1
    print stats.nacks
    for i in range(20):
        stats.write()
        time.sleep(0.02)
    