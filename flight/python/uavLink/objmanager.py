import uavlink
import inspect
import logging

# the object manager
# this is a routing object manager for now, ie it holds no copies of the data
# it assumes the data is always on the other side of its connection conn
class objManager():
    def __init__(self,conn=None):
        #conn is an unstream connection
        self.conn = conn
        self.objDefs = {}
        self.objNames = {}
        self.importObjDefs()
        self.retries = 3
    def _addObjDef(self, name, objDef):
        self.objDefs[objDef.OBJID] = objDef
        self.objNames[name] = objDef
    def importObjDefs(self):
        import uavlink.uavobjects
        for name,obj in inspect.getmembers(uavlink.uavobjects):
            if inspect.isclass(obj):
                if issubclass(obj,uavlink.uavObject):
                    self._addObjDef(name,obj)

    def receive(self,rxObjId,rxData):
        """ receive an id and data, return it as an uavobject"""
        # blindly send up the object
        self.conn.transSingleObjectAck(rxObjId,rxData)
        obj = self.getObjByID(rxObjId)
        obj.unpackData(rxData)
        return obj
    def unpack(self,rxObjId,rxData):
        """Returns on object from ID and packed data. Returns None if object is unkown"""
        pass
    def getObjByID (self,id,instance=0,read=True):
        if id in self.objDefs:
            obj = self.objDefs[id]()
            obj.setObjManager(self)
            obj.instance = instance
            if read:
                self.getObj(obj)
            return obj
        elif (id-1 in self.objDefs) and instance==0:
            #its a meta object
            obj = uavlink.uavMetaObject(id)
            obj.setObjManager(self)
            obj.updateName()
            if read:
                self.getObj(obj)
            return obj
        else:
            return None
    def getObjByName(self,name,instance=0,read=True):
        if name in self.objNames:
            return self.getObjByID( self.objNames[name].OBJID,instance,read)
        return None
    def getAllObjsIDs(self):
        return list(self.objDefs.keys())
    def getAllMetaObjsIDs(self):
        return map(lambda x : x+1, list(self.objDefs.keys()))
    def getObj(self,obj):
        attempt = self.retries
        while attempt:
            if obj.isSingleInstance():
                data = self.conn.transSingleObjectReq(obj.OBJID)
            else:
                data = self.conn.transInstanceObjectReq(obj.OBJID,obj.instance)
            if data:
                obj.unpackData(data)
                return True
            elif data == False:
                return False
            logging.warning("Retry getObj")
            attempt -= 1
    def setObj(self,obj):
        data = obj.getPackedData()
        attempt = self.retries
        while attempt:
            if obj.isSingleInstance():
                res = self.conn.transSingleObjectAck(obj.OBJID,data)
            else:
                res = self.conn.transInstanceObjectAck(obj.OBJID,obj.instance,data)
            if res:
                return True
            logging.warning("Retry setObj")
            attempt -= 1
        