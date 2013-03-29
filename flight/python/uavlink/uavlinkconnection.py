import threading
import Queue
import uavlink
import socket
import logging
import time

class uavLinkConnectionTransactionPool(threading.Thread):
    def __init__(self,conn,num_transactions,timeout=0.5):
        threading.Thread.__init__(self)
        self.conn = conn
        self.protocol = conn.protocol
        self.num_transactions = num_transactions
        self.trans_available = num_transactions
        self.timeout = timeout
        self.max_wait = 0.05
        self.trans_pool = []
        for t in range(self.num_transactions):
            self.trans_pool.append(uavLinkConnectionTransaction(self))
        self.trans_pool_lock = threading.Lock()
        self.trans_available_lock = threading.Lock()
    def get_timeout(self):
        return self.timeout
    def process(self,rxId,rxType,rxData):
        self.trans_available_lock.acquire()
        for trans in self.trans_pool:
            #they only return a timeout if they are active
            if trans.get_timeout():
                trans.process(rxId,rxType,rxData)
        self.trans_available_lock.release()
    def acquire(self):
        print "request new trans"
        print "try aquire avalable_lock"
        self.trans_available_lock.acquire()
        print "got avalable_lock"
        print "try aquire pool_lock"
        self.trans_pool_lock.acquire()
        print "got pool_lock"
        self.trans_available -= 1
        print "%d trans available" % self.trans_available
        if self.trans_available:
            self.trans_available_lock.release()
        #find which of the transactions is unlocked
        available_trans = None
        for trans in self.trans_pool:
            if trans.get_lock():
                available_trans = trans
                break
        if available_trans == None:
            raise Exception("Something is wrong with uavLinkConnectionTransationPool")
        self.trans_pool_lock.release()
        print "release pool lock"
        return available_trans
    def release(self,transaction):
        print "release trans"
        self.trans_pool_lock.acquire()
        if transaction not in self.trans_pool:
            raise Exception("Transaction not valid in this pool")
        transaction.release_lock()
        if self.trans_available == 0:
            self.trans_available_lock.release()    
        self.trans_available += 1
        self.trans_pool_lock.release()
        print "released"
    def run(self):
        while self.conn.connected:
            #did any transactions in the pool timeout
            for trans in self.trans_pool:
                if time.time() <= trans.get_timeout():
                    trans.send_timeout()
            #find the minum wait time
            next_timeout = time.time() + self.max_wait
            for trans in self.trans_pool:
                timeout = trans.get_timeout()
                if timeout:
                    if timeout < next_timeout:
                        next_timout = timeout
            sleep_time = next_timeout - time.time()
            if sleep_time > 0:
                time.sleep(sleep_time)
            else:
                print "missed thread"
            
        
        
        
# transaction class used by uavLinkConnection
#  known issues - this doesn't handle multi instance objects completely correctly
class uavLinkConnectionTransaction():
    ''' The strange use of locks in this thread is to avoid using a higher level threading object, these using polling waits in python'''
    def __init__(self,pool):
        self.transDoneEvent = threading.Event()
        self.pool = pool
        self.protocol = pool.protocol
        self.timeout = None
        self.lock = threading.Lock()
        self.done = threading.Lock()
        self.done_lock = threading.Lock()
    def get_timeout(self):
        return self.timeout
    def get_lock(self):
        return self.lock.acquire(False)
    def release_lock(self):
        return self.lock.release()
    def config(self,Id,transType,inst=None):
        self.id = Id
        self.transType = transType
        self.inst = inst
        self.rxType = None
        self.rxData = None
        self.rxAck = False
        self.reply = False
    def start(self):
        self.timeout = time.time() + self.pool.get_timeout()
        self.done.acquire()
    def send_timeout(self):
        self.done_lock.acquire()
        if not self.done.acquire(False):
            logging.warning( "transaction timeout type: %d id: %d" % (self.transType,self.id) )
            self.done.release()
        self.timeout = None
        self.done_lock.release()
    def process(self,rxId,rxType,rxData):
        #print "trans data %s len: %s" % (rxData.encode('hex'),len(rxData))
        if rxId != self.id:
            return 
        if (self.transType == self.protocol.TYPE_OBJ_REQ) & (rxType == self.protocol.TYPE_OBJ):
            self.done_lock.acquire()
            if not self.done.acquire(False):
                self.rxType = rxType
                self.rxData = rxData
                self.rxAck = True
                self.reply = True
                self.timeout = None
            self.done.release()
            self.done_lock.release()
        elif (self.transType == self.protocol.TYPE_OBJ_REQ) & (rxType == self.protocol.TYPE_NACK):
            self.done_lock.acquire()
            if not self.done.acquire(False):
                self.rxType = rxType
                self.rxData = None
                self.rxAck = False
                self.reply = True
                self.timeout = None
            self.done.release()
            self.done_lock.release()
        elif (self.transType == self.protocol.TYPE_OBJ_ACK) & (rxType == self.protocol.TYPE_ACK):
            #The response to an OBJ_ACK is always an ack to no need to check for a NACK for a OBJ_ACK
            self.done_lock.acquire()
            if not self.done.acquire(False):
                self.rxType = rxType
                self.rxData = None
                self.rxAck = True
                self.reply = True
                self.timeout = None
            self.done.release()
            self.done_lock.release()
        elif (self.transType == self.protocol.TYPE_STREAM) & (rxType == self.protocol.TYPE_ACK):
            #The response to an OBJ_ACK is always an ack to no need to check for a NACK for a OBJ_ACK
            self.done_lock.acquire()
            if not self.done.acquire(False):
                self.rxType = rxType
                self.rxData = None
                self.rxAck = True
                self.reply = True
                self.timeout = None
            self.done.acquire(False)
            self.done.release()
            self.done_lock.release()
    def getResponse(self):
        #Waits for the current transaction to complete
        self.done.acquire()
        final_data = (self.reply,self.rxAck,self.rxData) 
        self.done.release()
        self.pool.release(self)
        return final_data


    

class uavLinkConnection_rx(threading.Thread):
    """private class used only by uavLinkConnection to implement the receive thread"""
    def __init__(self,uavLinkConnection):
        threading.Thread.__init__(self)
        self.conn = uavLinkConnection
    def run(self):
        while(self.conn.connected):
            try:
                rxBytes = self.conn.read()
            except socket.error as e:
                self.conn.close()
                return
            if isinstance(str(rxBytes),str) and rxBytes != "":
                rxBytes = map(ord,rxBytes)
                for rxByte in rxBytes:
                    self.conn.protocol.rxByte(rxByte)
                self.conn.stats.rxBytes(len(rxBytes))


class uavLinkConnection_tx(threading.Thread):
    """private class used only by uavLinkConnection to implement the transmit thread""" 
    def __init__(self,uavLinkConnection):
        threading.Thread.__init__(self)
        self.conn = uavLinkConnection
    def run(self):
        while(self.conn.connected):
            data = self.conn.txQueue.get()
            while not self.conn.txQueue.empty():
                data = self.conn.txQueue.get()
            self.conn.stats.txBytes(len(data))
            try:
                self.conn.write(data)
            except socket.error as e:
                self.conn.close()
                return
                
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
        self.disconnectEvent = threading.Event()
        self.txQueue = Queue.Queue(1)
        self.write = write
        self.read = read
        self.rxSerialStreams = {}
        self.connected = False
        self.tx_lock = threading.Lock()
        self.pool = uavLinkConnectionTransactionPool(self,num_transactions=5,timeout=0.5)
    def register_rxStream_callback(self,streamId,callback):
        self.rxSerialStreams[streamId] = callback
    def start(self):
        self.connected = True
        self.tx_thread.start()
        self.rx_thread.start()
        self.pool.start()
    def close(self):
        if self.connected:
            logging.warning("closing uavlink connection")
        self.disconnectEvent.set()
        self.connected = False
    def tx(self,data):
        #self.tx_lock.acquire()
        print "start qtx"
        self.txQueue.put(data)
        print "stop qtx"
        #try:
        #    self.write(data)
        #except socket.error as e:
        #    self.close()
        #    return
        #self.tx_lock.release()
    def rxPacket(self,rxId,rxType,rxData ):
        #print "id: %d type: %d datalen: %d" % (rxId,rxType,len(rxData))
        #process the incoming object for effect on transactions
        self.pool.process(rxId,rxType,rxData)
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
        tran = self.pool.acquire()
        tran.config(ID,self.protocol.TYPE_STREAM)
        self.protocol.sendStream(ID,data)
        tran.start()
        (reply,rxAck,rxData)  = tran.getResponse()
        if reply:
            return rxAck
        else:
            self.stats.txFailure()
            return None
    def transSingleObjectReq(self,ObjId):
        """Sends an OBJ_REQ for objId, returns that object or None if failes""" 
        tran = self.pool.acquire()
        tran.config(ObjId,self.protocol.TYPE_OBJ_REQ)
        self.protocol.sendSingleObjectReq(ObjId)
        tran.start()
        (reply,rxAck,rxData)  = tran.getResponse()
        if reply:
            if rxAck:
                return rxData
            else:
                return False
        else:
            self.stats.txFailure()
            return None
    def transInstanceObjectReq(self,ObjId,Instance):
        """Sends an OBJ_REQ for objId,Instance returns that object or None if failes""" 
        tran = self.pool.acquire()
        tran.config(ObjId,self.protocol.TYPE_OBJ_REQ)
        self.protocol.sendInstanceObjectReq(ObjId,Instance)
        tran.start()
        (reply,rxAck,rxData)  = tran.getResponse()
        if reply:
            if rxAck:
                return rxData[2:]
            else:
                return False
        else:
            self.stats.txFailure()
            return None
    def transSingleObjectAck(self,ObjId,data):
        """Sends an OBJ_ACK for objId, returns True for ACK or None if timedout""" 
        tran = self.pool.acquire()
        tran.config(ObjId,self.protocol.TYPE_OBJ_ACK)
        self.protocol.sendSingleObjectAck(ObjId,data)
        tran.start()
        (reply,rxAck,rxData)  = tran.getResponse()
        if reply:
            return rxAck
        else:
            self.stats.txFailure()
            return None
    def transInstanceObjectAck(self,ObjId,Instance,data):
        """Sends an OBJ_ACK for objId,Instance, returns True for ACK or None if timedout""" 
        tran = self.pool.acquire()
        tran.config(ObjId,self.protocol.TYPE_OBJ_ACK)
        self.protocol.sendInstanceObjectAck(ObjId,data)
        tran.start()
        (reply,rxAck,rxData)  = tran.getResponse()
        if reply:
            return rxAck
        else:
            self.conn.txFailure()
            return None