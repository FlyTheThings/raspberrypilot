
import logging
import uavlink
import time
import socket 

# Log everything, and send it to stderr.
logging.basicConfig(level=logging.DEBUG)


# open a socket to the uavlink sever
sock = socket.socket()
sock.connect(("127.0.0.1", 8075))
# setup a connection on that socket
conn = uavlink.uavLinkConnection(None,sock.recv,sock.send)
conn.start()
# create an object manager for that connection
objMgr = uavlink.objManager(conn)


stats = objMgr.getObjByName("I2CStats")
stats.read()
print stats.nacks

while(True):
    stats.nacks += 1
    stats.nacks %= 100
    print stats.nacks
    for i in range(20):
        stats.write()
    