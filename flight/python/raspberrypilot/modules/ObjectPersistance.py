import time
import socket 
import sqlite3

from raspberrypilot import uavlink
from raspberrypilot import rp_core

def UAVObjLoad(cur, obj):
    cur.execute("SELECT data FROM uavObjects WHERE objId=? and instance=?" , (obj.OBJID,obj.instance))
    data = cur.fetchone()
    if data == None:
        return False
    obj.unpackData(data[0].decode('hex'))
    obj.set()
    return True
    
def UAVObjLoadSettings(cur,objMgr):
    for (objId,instance,data) in cur.execute("SELECT objId,instance,data FROM uavObjects WHERE isSetting=1"):
        obj =  objMgr.getObjByID(objId,instance,read = False)
        obj.unpackData(data.decode('hex'))
        obj.set()
    return True
def UAVObjLoadMetaobjects(cur,objMgr):
    for (objId,instance,data) in cur.execute("SELECT objId,instance,data FROM uavObjects isMeta=1"):
        obj =  objMgr.getObjByID(objId,instance,read = False)
        obj.unpackData(data.decode('hex'))
        obj.set()
    return True

def UAVObjLoadAllobjects(cur,objMgr):
    for (objId,instance,data) in cur.execute("SELECT objId,instance,data FROM uavObjects"):
        obj =  objMgr.getObjByID(objId,instance,read = False)
        obj.unpackData(data.decode('hex'))
        obj.set()
    return True
    
def UAVObjSave(cur,obj):
    "saves a single object"
    # delete the old one if it existed
    cur.execute("DELETE FROM uavObjects WHERE objId=? and instance=?",(obj.OBJID,obj.instance))
    values = (obj.OBJID,obj.instance,obj.name,obj.isMeta,obj.isSetting,obj.getPackedData().encode('hex'))
    cur.execute("INSERT INTO uavObjects(objId,instance,name,isMeta,isSetting,data) values (?,?,?,?,?,?)" , values)
    return True
    
def UAVObjSaveSettings(cur,objMgr):
    ids = objMgr.getAllObjsIDs()
    for objId in ids:
        obj = objMgr.getObjByID(objId,read = False)
        if obj.isSetting == True:
            if obj.isSingleInst:
                if obj.get():
                    UAVObjSave(cur,obj)
            else:
                inst = 0
                while True:
                    obj.instance = inst
                    if obj.get():
                        UAVObjSave(cur,obj)
                        inst += 1
                    else:
                        break
    return True
    
def UAVObjSaveMetaobjects(cur,objMgr):
    ids = objMgr.getAllMetaObjsIDs()
    for objId in ids:
        obj = objMgr.getObjByID(objId,read = False)
        #the following line should always be true
        if obj.isMeta == True:
            if obj.isSingleInst:
                obj.get()
                UAVObjSave(cur,obj)
    return True
    
def UAVObjDelete(cur,obj):
    cur.execute("DELETE FROM uavObjects WHERE objId=? and instance=?",(obj.OBJID,obj.instance))
    return True
    
def UAVObjDeleteAll(cur):
    cur.execute("DELETE * FROM uavObjects")
    return True
    
class objectPersistance(rp_core.raspberryPilotModule):
    def run(self):
        #conn = uavlink.uavLinkConnection_UDP(addr="192.168.1.115",port=32001)
        #objMgr = uavlink.objManager(conn)
        objper = self.objMgr.getObjByName("ObjectPersistence")
        while not self.objMgr.getObj(objper):
            print("Unable to get objper")
            pass
        con = sqlite3.connect('uavobjects.sqlite')
        cur = con.cursor()
        #check if the uavObjects table exists
        cur.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='uavObjects'")
        if not cur.fetchone():
            cur.execute("CREATE TABLE uavObjects(objId INT, instance INT, name TEXT, isMeta INT, isSetting INT, data TEXT)")

        retval = None
        sleep_time = 0.001
        while True:
            # Get object data
            if retval == None:
                sleep_time *= 2
                if sleep_time > 0.5:
                    sleep_time = 0.5 
                time.sleep(sleep_time)
            else:
                sleep_time = 0.001
                time.sleep(sleep_time)
            objper.get()
            
            # Execute action each action returns true for success, false for failure
            retval = None
            if (objper.Operation == "LOAD") :
                if (objper.Selection == "SINGLEOBJECT") :
                    # Get selected object
                    obj =  self.objMgr.getObjByID(objper.ObjectID,objper.InstanceID)
                    if not obj:
                        print("can't load %s" % objper.ObjectID)
                        retval = False
                    else:
                        # Load selected instance
                        retval = UAVObjLoad(cur,obj)
                elif objper.Selection == "ALLSETTINGS":
                    retval = UAVObjLoadSettings(cur,self.objMgr)
                elif objper.Selection == "ALLMETAOBJECTS":
                    retval = UAVObjLoadMetaobjects(cur,self.objMgr)
                elif objper.Selection == "ALLOBJECTS":
                    retval = UAVObjLoadAllobjects(cur,self.objMgr)
            elif (objper.Operation == "SAVE"):
                if (objper.Selection == "SINGLEOBJECT"):
                    # Get selected object
                    obj = self.objMgr.getObjByID(objper.ObjectID,objper.InstanceID)
                    if not obj:
                        retval = False
                    else:
                        # Save selected instance
                        retval = UAVObjSave(cur,obj)
                        # Verify saving worked
                        if retval:
                            retval = UAVObjLoad(cur,obj)
                if (objper.Selection == "ALLSETTINGS" or objper.Selection == "ALLOBJECTS"):
                    retval = UAVObjSaveSettings(cur,self.objMgr)
                if (objper.Selection == "ALLMETAOBJECTS" or objper.Selection == "ALLOBJECTS"):
                    retval = UAVObjSaveMetaobjects(cur,self.objMgr)
            elif (objper.Operation == "DELETE"):
                if (objper.Selection == "SINGLEOBJECT"):
                    # Get selected object
                    obj = self.objMgr.getObjByID(objper.ObjectID)
                    if not obj:
                        retval = False
                    else:
                        # Delete selected instance
                        retval = UAVObjDelete(obj, objper.InstanceID)
                elif (objper.Selection == "ALLSETTINGS" or objper.Selection == "ALLOBJECTS") :
                    retval = UAVObjDeleteSettings(cur)
                elif (objper.Selection == "ALLMETAOBJECTS" or objper.Selection == "ALLOBJECTS") :
                    retval = UAVObjDeleteMetaobjects(cur)
            elif (objper.Operation == "FULLERASE") :
                retval = UAVObjDeleteAll(cur)
            
            con.commit() 
            #Update the operation in the object
            if retval == True:
                objper.Operation = "COMPLETED"
                objper.set()
            elif retval == False:
                objper.Operation = "ERROR"
                objper.set()
            else:
                pass

if __name__ == "__main__":
    conn = uavlink.uavLinkConnection_UDP(addr="192.168.1.116",port=32001)
    objMgr = uavlink.objManager(conn)
    op = objectPersistance(objMgr)
    op.run()
