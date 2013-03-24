
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

#open a serial port to the embedded micro
ser = serial.Serial("COM11",baudrate=57600)

#create a stream server to allow telemetry to connect
uavtalk_stream_server = uavlink.streamServer("", 8079)
conn = uavlink.uavLinkConnection(None,ser.read,ser.write)
uavtalk_stream_server.register_rx_handler(lambda data: conn.sendStream(1,data,timeout=1,retries=3))
conn.register_rxStream_callback(1,lambda data: uavtalk_stream_server.write(data) )

# start the connection to the embedded micro
conn.start()
     
# create an object manager and connect it through conn to the embedded microcontroller
objMgr = uavlink.objManager(conn)

# create a server to allow uavlink connections our object manager
uavlink_server = uavlink.uavLinkServer(objMgr,"",8075)

#wait until we are getting responses on the *UavlinkStats objects before continueing
CompUavlinkStats = None
while CompUavlinkStats == None:
    CompUavlinkStats = objMgr.getObjByName("CompUavlinkStats")
FlightUavlinkStats = None
while FlightUavlinkStats == None:
    FlightUavlinkStats = objMgr.getObjByName("FlightUavlinkStats")    

# Do the handshake and update the CompUavlinkStats forever
period = 0.5
while(True):
    CompUavlinkStats.set()
    time.sleep(period)
    
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
            #if both are disconnected clear stats and start the handshake
            CompUavlinkStats.TxDataRate = 0
            CompUavlinkStats.RxDataRate = 0
            CompUavlinkStats.TxFailures = 0
            CompUavlinkStats.RxFailures = 0
            CompUavlinkStats.TxRetries = 0
            CompUavlinkStats.Status = "HANDSHAKEREQ"
    
    connStats = conn.stats.get()
    conn.stats.clear()
    # filtering on the rates
    CompUavlinkStats.TxDataRate = 0.25*connStats['TxBytes']//period + 0.75*CompUavlinkStats.TxDataRate
    CompUavlinkStats.RxDataRate = 0.25*connStats['RxBytes']//period + 0.75*CompUavlinkStats.RxDataRate
    CompUavlinkStats.TxFailures += connStats['TxFailures']
    CompUavlinkStats.RxFailures += connStats['RxFailures']
    CompUavlinkStats.TxRetries += connStats['TxRetries']



    