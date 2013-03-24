import uavlink

class Crc(object):
    
    crcTable = ( 0x00, 0x07, 0x0e, 0x09, 0x1c,
            0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
            0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46,
            0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb,
            0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd, 0x90,
            0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
            0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5,
            0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0,
            0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93,
            0x94, 0x9d, 0x9a, 0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
            0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59,
            0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74,
            0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1,
            0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4, 0xf9, 0xfe, 0xf7, 0xf0,
            0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3,
            0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56,
            0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10, 0x05,
            0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
            0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78,
            0x7f, 0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25,
            0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae,
            0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f,
            0x8a, 0x8d, 0x84, 0x83, 0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc,
            0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3 )
    def __init__(self):
        self.reset()
    def reset(self, firstValue=None):
        self.crc = 0
        if firstValue != None:
            self.add(firstValue)
    def read(self):
        return self.crc
    def add(self, value):
        if isinstance(value, str):
            value = ord(value)
        self.crc = Crc.crcTable[self.crc ^ (value & 0xff)]
    def addList(self, values):
        for v in values:
            self.add(v)


class uavLinkProtocol():
    """this class's responsibility is to parse a stream of bytes into objects and serial packets events, which are implemented as callbacks.
    It also can take raw data bytes and encapsulate them into uavlink packets.  It knows nothing about what the data means.    
    """
    def __init__(self):
        # constants
        self.SYNC = 0x3E
        self.VERSION_MASK = 0xF8
        self.VERSION = 0x20
        self.TYPE_MASK  = 0x07
        self.TYPE_OBJ = 0x00
        self.TYPE_OBJ_REQ = 0x01
        self.TYPE_OBJ_ACK = 0x02
        self.TYPE_ACK = 0x03
        self.TYPE_NACK = 0x04
        self.TYPE_STREAM = 0x05
        self.STATE_SYNC = 0
        self.STATE_TYPE = 1 
        self.STATE_SIZE = 2
        self.STATE_STREAMID = 4
        self.STATE_OBJID = 5
        self.STATE_INSTID = 6
        self.STATE_DATA = 7
        self.STATE_CS = 8
        self.STREAM_HEADER_LENGTH = 5       # sync(1), type (1), size(2), stream ID(1)
        self.OBJECT_HEADER_LENGTH = 8       # sync(1), type (1), size(2), object ID (4)
        self.OBJECT_INST_HEADER_LENGTH = 10  # sync(1), type (1), size(2), object ID (4), instance ID(2)
        self.MIN_HEADER_LENGTH = self.STREAM_HEADER_LENGTH  
        self.MAX_HEADER_LENGTH = self.OBJECT_INST_HEADER_LENGTH
        self.MAX_PAYLOAD_LENGTH = 255
        self.CHECKSUM_LENGTH = 1
        self.MAX_PACKET_LENGTH = (self.MAX_HEADER_LENGTH + self.MAX_PAYLOAD_LENGTH + self.CHECKSUM_LENGTH)
        # variables
        self.rxState = self.STATE_SYNC
        self.rxCrc = Crc()
        self.rxCnt = 0
        self.rxData = []
        self.uavLinkRxObjectPacket_callback = None
        self.uavLinkRxError_callback = None
        self.uavLinkTx_callback = None
    def register_uavLinkRxPacket_callback(self,uavLinkPacket_callback):
        "A uavLinkObjectPacket_callback takes three arguements, the object_id, the message type, and the objects data"
        self.uavLinkPacket_callback = uavLinkPacket_callback
    def register_uavLinkTx_callback(self,uavLinkTxPacket_callback):
        "A uavLinkSend_callback takes two arguements, the streamId,and the data"
        self.uavLinkTx_callback = uavLinkTxPacket_callback
    def register_uavLinkRxError_callback(self,uavLinkRxError_callback):
        "Functino to call when a rx Error occurs"
        self.uavLinkRxError_callback = uavLinkRxError_callback
    def _tx(self,data):
        if self.uavLinkTx_callback:
            return self.uavLinkTx_callback(data)
        else:
            return data
    def rxByte(self,byte):
        byte = int(byte)
        self.rxCrc.add(byte)
        self.rxCnt +=1
        #SYNC STATE
        if self.rxState == self.STATE_SYNC:
            if byte == self.SYNC:
                self.rxCrc.reset(byte)       
                self.rxState = self.STATE_TYPE
                self.rxCnt = 1
                self.rxData = []
        #TYPE STATE
        elif self.rxState == self.STATE_TYPE:
            if (byte & self.VERSION_MASK != self.VERSION):
                self.rxState == self.STATE_SYNC
                logging.warning("Version Mismatch")
            else:
                self.rxType = byte & self.TYPE_MASK
                self.rxCount = 0
                self.rxSize = 0
                self.rxState = self.STATE_SIZE
        #SIZE STATE
        elif self.rxState == self.STATE_SIZE:
            self.rxSize >>= 8
            self.rxSize |= (byte<<8)
            self.rxCount += 1
            
            if self.rxCount == 2:    
                # Received complete packet size, check for valid packet size
                if (self.rxSize < self.MIN_HEADER_LENGTH) or (self.rxSize > self.MAX_HEADER_LENGTH + self.MAX_PAYLOAD_LENGTH):
                    logging.error("INVALID Packet Size")
                    self.rxState = self.STATE_SYNC
                else:
                    self.rxCount = 0
                    self.rxId = 0
                    if (self.rxType == self.TYPE_STREAM):
                        self.rxState = self.STATE_STREAMID
                    else:
                        self.rxState = self.STATE_OBJID
        # OBJECT ID STATE
        elif self.rxState == self.STATE_OBJID:
            self.rxId >>= 8
            self.rxId |= (byte<<24)
            self.rxCount += 1
            if self.rxCount == 4:    
                self.rxData = ""
                self.rxCount = 0
                self.rxDataSize = self.rxSize - self.rxCnt
                if self.rxDataSize:
                    self.rxState = self.STATE_DATA
                else:
                    self.rxState = self.STATE_CS
        # STREAMID STATE
        elif self.rxState == self.STATE_STREAMID:
            self.rxId = byte
            self.rxState = self.STATE_DATA
            self.rxDataSize = self.rxSize - self.rxCnt
            self.rxData = ""
        # DATA STATE
        elif self.rxState == self.STATE_DATA:
            self.rxData += chr(byte)
            self.rxCount += 1
            if self.rxCount == self.rxDataSize:
                self.rxState = self.STATE_CS
        # CHECKSUM STATE
        elif self.rxState == self.STATE_CS:
            self.rxState = self.STATE_SYNC
            if self.rxCrc.read() != 0:
                logging.error("CRC ERROR")
                if self.uavLinkRxError_callback:
                    self.uavLinkRxError_callback()
            elif self.uavLinkPacket_callback:
                self.uavLinkPacket_callback(self.rxId,self.rxType,self.rxData)
    def sendStream(self,serialId,data):
        """where serialId is the 8bit serial id and data is a string"""
        if data == "":
            return
        if data == None:
            return
        if len(data) > 200:
            print "data too long"
        length = len(data) + self.STREAM_HEADER_LENGTH 
        toSend = chr(self.SYNC)
        toSend += chr(self.TYPE_STREAM | self.VERSION)
        toSend += chr(length & 0xFF)
        toSend += chr( (length >>8) & 0xFF )
        toSend += chr(serialId)
        toSend += data
        crc = Crc()
        crc.addList(toSend)
        toSend += chr(crc.read())
        return self._tx(toSend)
    def _sendObject(self,ObjId,objType,data,instanceId = None):
        if instanceId:
            length = len(data) + self.OBJECT_INST_HEADER_LENGTH
        else:
            length = len(data) + self.OBJECT_HEADER_LENGTH
        toSend = chr(self.SYNC)
        toSend += chr(objType | self.VERSION)
        toSend += chr(length & 0xFF)
        toSend += chr( (length >> 8) & 0xFF)
        toSend += chr( (ObjId >> 0) & 0xFF)
        toSend += chr( (ObjId >> 8) & 0xFF)
        toSend += chr( (ObjId >> 16) & 0xFF)
        toSend += chr( (ObjId >> 24) & 0xFF)
        if instanceId:
            toSend += chr(instanceId)
        toSend += data
        crc = Crc()
        crc.addList(toSend)
        toSend += chr(crc.read())
        return self._tx(toSend)
    def sendSingleObject(self,ObjId,data):
        return self._sendObject(ObjId,self.TYPE_OBJ,data)
    def sendSingleObjectReq(self,ObjId):
        return self._sendObject(ObjId,self.TYPE_OBJ_REQ,"")
    def sendSingleObjectAck(self,ObjId,data):
        return self._sendObject(ObjId,self.TYPE_OBJ_ACK,data)
    def sendInstanceObject(self,ObjId,Instance,data):
        return self._sendObject(ObjId,self.TYPE_OBJ,data,Instance)    
    def sendInstanceObjectReq(self,ObjId,Instance):
        return self._sendObject(ObjId,self.TYPE_OBJ_REQ,"",Instance)
    def sendInstanceObjectAck(self,ObjId,Instance,data):
        return self._sendObject(ObjId,self.TYPE_OBJ_ACK,data,Instance)
    def sendAck(self,ObjId,instanceId=None):
        return self._sendObject(ObjId,self.TYPE_ACK,"",instanceId)
    def sendNack(self,ObjId,instanceId=None):
        return self._sendObject(ObjId,self.TYPE_NACK,"",instanceId)






        
        

