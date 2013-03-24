
import logging
import uavlink
import uavlink.uavobjects
import serial
import time
import sys
#sys.path.append("../Modules")
#import ObjectPersistance

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


stats = objMgr.getObjByName("ObjectPersistence")
stats.get()


while(True):
    continue
    stats.get()
    time.sleep(0.5)


    