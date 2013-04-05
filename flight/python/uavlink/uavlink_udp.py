import threading
import Queue
import uavlink
import socket
import logging
import time
import select


class uavLinkConnection_UDP():
    """ this class's responsibility is to turn object transactions into udp packets"""
    def __init__(self,addr="127.0.0.1",port=32001):
        self.VERSION_MASK = 0xF8
        self.TYPE_MASK  = 0x07
        self.TYPE_OBJ = 0x00
        self.TYPE_OBJ_REQ = 0x01
        self.TYPE_OBJ_ACK = 0x02
        self.TYPE_ACK = 0x03
        self.TYPE_NACK = 0x04
        self.TYPE_STREAM = 0x05
        self.VERSION = 0x20
        self.serv_addr = addr
        self.serv_port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
    def _tx(self,data):
        self.socket.sendto(data,(self.serv_addr,self.serv_port))
    def _rx(self,timeout=0.05):
        ready = select.select([self.socket],[],[],timeout)
        if len(ready[0]):
            (data,dfrom) = self.socket.recvfrom(4096)
            return data
    def extract_response(self,data):
        rxType = ord(data[0]) & self.TYPE_MASK
        rxId = ord(data[1]) + (ord(data[2]) << 8) + (ord(data[3]) << 16) + (ord(data[4]) << 24)
        if len(data) > 5:
            data = data[5:]
        else:
            data = []
        return (rxType,rxId,data)
    def get_response(self,type,id,timeout=0.05):
        '''returns the received data or None'''
        timeout_time = time.time() + timeout
        while (time.time() < timeout_time):
            timeout = timeout_time - time.time()
            if timeout <= 0:
                break
            data = self._rx(timeout)
            if data:
                (rxType,rxId,data) = self.extract_response(data)
                if rxType == type and rxId == id:
                    return data
        return None
    def sendSingleObject(self,objId,data):
        """Sends a single object, no return value"""
        toSend = chr(self.TYPE_OBJ | self.VERSION)
        toSend += chr( (objId >> 0) & 0xFF)
        toSend += chr( (objId >> 8) & 0xFF)
        toSend += chr( (objId >> 16) & 0xFF)
        toSend += chr( (objId >> 24) & 0xFF)
        toSend += data
        self._tx(toSend)
    def sendInstanceObject(self,objId,Instance,data): 
        """Sends a instance object, no return value"""
        raise Exception("Unimplemented")
    def transSingleObjectReq(self,objId):
        """Sends an OBJ_REQ for objId, returns that object or None if failes""" 
        toSend = chr(self.TYPE_OBJ_REQ | self.VERSION)
        toSend += chr( (objId >> 0) & 0xFF)
        toSend += chr( (objId >> 8) & 0xFF)
        toSend += chr( (objId >> 16) & 0xFF)
        toSend += chr( (objId >> 24) & 0xFF)
        self._tx(toSend)
        return self.get_response(self.TYPE_OBJ,objId)
    def transInstanceObjectReq(self,objId,Instance):
        """Sends an OBJ_REQ for objId,Instance returns that object or False for no ACK""" 
        raise Exception("Unimplemented")
    def transSingleObjectAck(self,objId,data):
        """Sends an OBJ_ACK for objId, returns True for ACK or None if timedout""" 
        toSend = chr(self.TYPE_OBJ_ACK | self.VERSION)
        toSend += chr( (objId >> 0) & 0xFF)
        toSend += chr( (objId >> 8) & 0xFF)
        toSend += chr( (objId >> 16) & 0xFF)
        toSend += chr( (objId >> 24) & 0xFF)
        toSend += data
        self._tx(toSend)
        response = self.get_response(self.TYPE_ACK,objId)
        if response != None:
            return True
        else:
            return False
    def transInstanceObjectAck(self,objId,Instance,data):
        """Sends an OBJ_ACK for objId,Instance, returns True for ACK or None if timedout""" 
        raise Exception("Unimplemented")