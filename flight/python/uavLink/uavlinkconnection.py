import threading
import Queue
import uavlink
import socket
import logging

# transaction class used by uavLinkConnection
class uavLinkConnectionTransaction():
    def __init__(self,conn,Id,transType,timeout=0.5):
        self.transDoneEvent = threading.Event()
        self.conn = conn
        self.protocol = conn.protocol
        self.id = Id
        self.transType = transType
        self.transTimeout = timeout
        self.rxType = None
        self.rxData = None
        self.rxAck = False
        self.reply = False
        self.conn.register_transaction(self)
    def process(self,rxId,rxType,rxData):
        #print "trans data %s len: %s" % (rxData.encode('hex'),len(rxData))
        if rxId != self.id:
            return
        if (self.transType == self.protocol.TYPE_OBJ_REQ) & (rxType == self.protocol.TYPE_OBJ):
            self.rxType = rxType
            self.rxData = rxData
            self.reply = True
            self.transDoneEvent.set()
        elif (self.transType == self.protocol.TYPE_OBJ_REQ) & (rxType == self.protocol.TYPE_NACK):
            self.rxType = rxType
            self.rxData = None
            self.rxAck = False
            self.reply = True
            self.transDoneEvent.set()
        elif (self.transType == self.protocol.TYPE_OBJ_ACK) & (rxType == self.protocol.TYPE_ACK):
            #The response to an OBJ_ACK is always an ack to no need to check for a NACK for a OBJ_ACK
            self.rxType = rxType
            self.rxData = None
            self.rxAck = True
            self.reply = True
            self.transDoneEvent.set()
        elif (self.transType == self.protocol.TYPE_STREAM) & (rxType == self.protocol.TYPE_ACK):
            #The response to an OBJ_ACK is always an ack to no need to check for a NACK for a OBJ_ACK
            self.rxType = rxType
            self.rxData = None
            self.rxAck = True
            self.reply = True
            self.transDoneEvent.set()
    def getResponse(self):
        #Waits for the current transaction to complete
        result = self.transDoneEvent.wait(self.transTimeout)
        self.conn.deregister_transaction(self)
        if not result:
            logging.warning( "transaction timeout type: %d id: %d" % (self.transType,self.id) )
        return self.reply
    def getData(self):
        if self.reply:
            return self.rxData
    def getAck(self):
        if self.reply:
            return self.rxAck

    

class uavLinkConnection_rx(threading.Thread):
    """private class used only by uavLinkConnection to implement the receive thread"""
    def __init__(self,uavLinkConnection):
        threading.Thread.__init__(self)
        self.conn = uavLinkConnection
    def run(self):
        while(self.conn.connected):
            try:
                byte = self.conn.read(1)
            except socket.error as e:
                self.conn.close()
            if isinstance(byte,str) and byte != "":
                byte = ord(byte) 
                self.conn.protocol.rxByte(byte)
                self.conn.stats.rxBytes(1)


class uavLinkConnection_tx(threading.Thread):
    """private class used only by uavLinkConnection to implement the transmit thread""" 
    def __init__(self,uavLinkConnection):
        threading.Thread.__init__(self)
        self.conn = uavLinkConnection
    def run(self):
        while(self.conn.connected):
            data = self.conn.txQueue.get()
            self.conn.stats.txBytes(len(data))
            try:
                self.conn.write(data)
            except socket.error as e:
                self.conn.close()
                
class uavLinkConnectionStats():
    def __init__(self):
        self.stats = {
            "TxFailures" : 0,
            "RxFailures" : 0,
            "TxRetries" : 0,
            "TxBytes" : 0,
            "RxBytes" : 0
            }
        self.lock = threading.RLock()
    def rxBytes(self,num):
        self.lock.acquire()
        self.stats['RxBytes'] += num
        self.lock.release()
    def txFailure(self):
        self.lock.acquire()
        self.stats['TxFailures'] += 1
        self.lock.release()
    def rxFailure(self):
        self.lock.acquire()
        self.stats['RxFailures'] += 1
        self.lock.release()
    def txBytes(self,num):
        self.lock.acquire()
        self.stats['TxBytes'] += num
        self.lock.release()
    def clear(self):
        self.lock.acquire()
        self.stats = {
            "TxFailures" : 0,
            "RxFailures" : 0,
            "TxRetries" : 0,
            "TxBytes" : 0,
            "RxBytes" : 0
            }
        self.lock.release()
    def get(self):
        self.lock.acquire()
        stats = {}
        stats.update(self.stats)
        self.lock.release()
        return stats
   
class uavLinkConnection():
    """ this class's responsibility is to manage a uavLink connection.  It uses the uavLinkProtocol to process and encode packets.
    This connection is multithreaded and handles the ack/nak and obj/req transactions. 
    """
    def __init__(self,objMgr,read,write):
        self.objMgr = objMgr
        self.stats = uavLinkConnectionStats()
        self.protocol = uavlink.uavLinkProtocol()
        self.protocol.register_uavLinkTx_callback(self.tx)
        self.protocol.register_uavLinkRxPacket_callback(self.rxPacket)
        self.protocol.register_uavLinkRxError_callback(self.stats.rxFailure)
        self.tx_thread = uavLinkConnection_tx(self)
        self.rx_thread = uavLinkConnection_rx(self)
        self.trans_list_lock = threading.RLock()
        self.txQueue = Queue.Queue()
        self.disconnectEvent = threading.Event()
        self.trans = []
        self.max_trans = 5
        self.trans_available = threading.Event()
        self.write = write
        self.read = read
        self.rxSerialStreams = {}
        self.connected = False
        
    def register_rxStream_callback(self,streamId,callback):
        self.rxSerialStreams[streamId] = callback
    def register_transaction(self,tran):
        self.trans_list_lock.acquire()
        if len(self.trans) > self.max_trans:
            self.trans_list_lock.release()
            self.trans_available.wait()
            self.trans_list_lock.acquire()
        self.trans.append(tran)
        self.trans_list_lock.release()
    def deregister_transaction(self,tran):
        self.trans_list_lock.acquire()
        self.trans.remove(tran)
        if len(self.trans) < self.max_trans:
            self.trans_available.set()
        elif len(self.trans) >= self.max_trans:
            self.trans_available.clear()
        self.trans_list_lock.release()
    def start(self):
        self.connected = True
        self.tx_thread.start()
        self.rx_thread.start()
    def close(self):
        if self.connected:
            logging.warning("closing uavlink connection")
        self.disconnectEvent.set()
        self.connected = False
    def tx(self,data):
        self.txQueue.put(data)
    def rxPacket(self,rxId,rxType,rxData ):
        #print "id: %d type: %d datalen: %d" % (rxId,rxType,len(rxData))
        #process the incoming object for effect on transactions
        self.trans_list_lock.acquire()
        for tran in self.trans:
            tran.process(rxId,rxType,rxData)
        self.trans_list_lock.release()
        #if there is an object manager send objects to it
        if rxType == self.protocol.TYPE_OBJ:
            if self.objMgr:
                #receive the object into the object manager
                self.objMgr.receive(rxId,rxData)
        elif rxType == self.protocol.TYPE_OBJ_ACK:
            if self.objMgr:
                #receive the object into the object manager and ack?
                obj = self.objMgr.receive(rxId,rxData)
                self.protocol.sendAck(rxId,obj.getInstance())
            else:
                self.protocol.sendAck(rxId)
        elif rxType == self.protocol.TYPE_OBJ_REQ:
            #get the object from the object manager, 
            #send an obj if it exists nack otherwise
            if len(rxData) == 2:
                objInstance = ord(rxData[0]) + (ord(rxData[1]) << 8)
                if self.objMgr:
                    raise "UNIMPLEMENTED"
                    obj = self.objMgr.getObjInstanceByID(rxId,objInstance)
                    self.protocol.sendInstanceObject(rxId,objInstance,obj.getPackedData())
            if len(rxData) == 0:
                obj = self.objMgr.getObjByID(rxId)
                obj.get()
                self.protocol.sendSingleObject(rxId,obj.getPackedData())
            else:
                logging.warning("Received Obj Request with data length other than 2 or 0")
        elif rxType == self.protocol.TYPE_STREAM:
            self.protocol.sendAck(rxId)
            if rxId in self.rxSerialStreams:
                self.rxSerialStreams[rxId](rxData)
            else:
                logging.warning("Recieved Unregistered StreamId: %d", rxId)
    def sendStream(self,ID,data,timeout=0.5,retries=3):
        while retries:
            ack = self.transSendStreamAck(ID,data,timeout)
            if ack:
                return ack
            logging.warning( "retrying serial, len %s" % len(data) )
            retries -= 1
    def sendSingleObject(self,ObjId,data):
        """Sends a single object, no return value"""
        self.protocol.sendSingleObject(ObjId,data)
    def sendInstanceObject(self,ObjId,Instance,data): 
        """Sends a single object, no return value"""
        self.protocol.sendInstanceObject(ObjId,Instance,data) 
    def transSendStreamAck(self,ID,data,timeout=0.5):
        """Sends a stream of data to streamID, returns True for ACK or None if timedout""" 
        tran = uavLinkConnectionTransaction(self,ID,self.protocol.TYPE_STREAM,timeout=timeout)
        self.protocol.sendStream(ID,data)
        if (tran.getResponse()):
            return tran.getAck()
        else:
            self.stats.txFailure()
            return None
    def transSingleObjectReq(self,ObjId):
        """Sends an OBJ_REQ for objId, returns that object or None if failes""" 
        tran = uavLinkConnectionTransaction(self,ObjId,self.protocol.TYPE_OBJ_REQ)
        self.protocol.sendSingleObjectReq(ObjId)
        if (tran.getResponse()):
            rxData = tran.getData()
            return rxData
        else:
            self.stats.txFailure()
            return None
    def transInstanceObjectReq(self,ObjId,Instance):
        """Sends an OBJ_REQ for objId,Instance returns that object or None if failes""" 
        tran = uavLinkConnectionTransaction(self,ObjId,self.protocol.TYPE_OBJ_REQ)
        self.protocol.sendInstanceObjectReq(ObjId,Instance)
        if (tran.getResponse()):
            rxData = tran.getData()
            return rxData[2:]
        else:
            self.stats.txFailure()
            return None
    def transSingleObjectAck(self,ObjId,data):
        """Sends an OBJ_ACK for objId, returns True for ACK or None if timedout""" 
        tran = uavLinkConnectionTransaction(self,ObjId,self.protocol.TYPE_OBJ_ACK)
        self.protocol.sendSingleObjectAck(ObjId,data)
        if (tran.getResponse()):
            return tran.getAck()
        else:
            self.stats.txFailure()
            return None
    def transInstanceObjectAck(self,ObjId,Instance,data):
        """Sends an OBJ_ACK for objId,Instance, returns True for ACK or None if timedout""" 
        tran = uavLinkConnectionTransaction(self,ObjId,self.protocol.TYPE_OBJ_ACK)
        self.protocol.sendInstanceObjectAck(ObjId,data)
        if (tran.getResponse()):
            return  tran.getAck()
        else:
            self.conn.txFailure()
            return None