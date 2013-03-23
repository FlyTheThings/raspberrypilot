
import logging
import uavlink
import uavlink.uavobjects
import serial
import time


# Log everything, and send it to stderr.
logging.basicConfig(level=logging.DEBUG)



ser = serial.Serial("COM11",baudrate=57600)

uavtalk_stream_server = uavlink.streamServer("", 8079)
conn = uavlink.uavLinkConnection(None,ser.read,ser.write)

uavtalk_stream_server.register_rx_handler(lambda data: conn.sendStream(1,data,timeout=1,retries=3))
conn.register_rxStream_callback(1,lambda data: uavtalk_stream_server.write(data) )

conn.start()
     
objMgr = uavlink.objManager(conn)
uavlink_server = uavlink.uavLinkServer(objMgr,"",8075)


stats = objMgr.getObjByName("I2CStats")
stats.read()
print stats.nacks

while(True):
    time.sleep(1)
    
"""
    stats.nacks += 1
    stats.nacks %= 100
    print stats.nacks
    for i in range(20):
        stats.write()
        time.sleep(0.02)
"""
    