from uavlinkprotocol import *
from uavobject import *
from uavlinkserver import *
from streamserver import *
from objmanager import *
from uavlinkconnection import *
import logging

# This is a helper function to clean up repeat code in flight modules
def connectObjMgr(host="127.0.0.1",port=8075):
    # open a socket to the uavlink sever
    sock = socket.socket()
    sock.connect((host, port))
    # setup a connection on that socket
    conn = uavlink.uavLinkConnection(None,sock.recv,sock.send)
    conn.start()
    # create an object manager for that connection
    objMgr = uavlink.objManager(conn)
    return objMgr
