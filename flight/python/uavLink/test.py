
import logging
import uavlink.uavobjects


if __name__ == "__main__":
    # Log everything, and send it to stderr.
    logging.basicConfig(level=logging.DEBUG)
    
    objMgr = uavlink.objManager()
    
    exit()
    
    ser = serial.Serial("COM11",baudrate=57600)
    
    uavtalk_server = uavlink.streamServer(("", 8079), streamServerHandler)
    uavtalk_server.register_rx_handler(lambda data: conn.sendSerial(1,data))
    
    conn = uavlink.uavLinkConnection(None,ser.read,ser.write)
    conn.register_rxStream_callback(1,lambda data: uavtalk_server.write(data) )
    conn.start()
    
    
    
    while (True):
        pass