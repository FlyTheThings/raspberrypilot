
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

CompUavlinkStats = None
while CompUavlinkStats == None:
    CompUavlinkStats = objMgr.getObjByName("CompUavlinkStats")
FlightUavlinkStats = None
while FlightUavlinkStats == None:
    FlightUavlinkStats = objMgr.getObjByName("FlightUavlinkStats")    

while(True):
    CompUavlinkStats.set()
    time.sleep(0.5)
    print CompUavlinkStats.Status
    
    if not FlightUavlinkStats.get():
        CompUavlinkStats.Status = "DISCONNECTED"
    if CompUavlinkStats.Status == "CONNECTED":
        if FlightUavlinkStats.Status == "DISCONNECTED":
            CompUavlinkStats.Status = "DISCONNECTED"
            continue
    elif CompUavlinkStats.Status == "HANDSHAKEREQ":
        if FlightUavlinkStats.Status == "HANDSHAKEACK":
            CompUavlinkStats.Status = "CONNECTED"
    # This case won't happen as flight does not begin the handshake
    #elif CompUavlinkStats.Status == "HANDSHAKEACK":
    #    if FlightUavlinkStats.Status == "CONNECTED":
    #        CompUavlinkStats.Status = "CONNECTED"
    elif CompUavlinkStats.Status == "DISCONNECTED":
        if FlightUavlinkStats.Status == "DISCONNECTED":
            CompUavlinkStats.Status = "HANDSHAKEREQ"
    
    



    