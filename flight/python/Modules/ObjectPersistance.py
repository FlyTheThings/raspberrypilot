import uavlink
import time
import socket 

def UAVObjLoad(obj, InstanceID):
    "load an obj from storage"
    print "load an obj from storage"
    pass
    
def UAVObjLoadSettings():
    "load all settings from storage"
    pass
def UAVObjLoadMetaobjects():
    "load all meta objects from storage"
    print "load all meta objects from storage"
    pass
    
def UAVObjSave(obj, InstanceID):
    "saves a single object"
    print "saves a single object"
    pass
    
def UAVObjSaveSettings():
    "save all settings"
    print "save all settings"
    pass

def UAVObjSaveMetaobjects():
    "save all meta objects"
    print "save all meta objects"
    pass
    
def UAVObjDelete(obj, InstanceID):
    print "Delete Object"
    pass
    
def UAVObjDeleteAll():
    print "delete all"
    pass
    
    
# should probably wrap this into some helper function
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

objMgr = connectObjMgr(host="127.0.0.1",port=8075)
objper = objMgr.getObjByName("ObjectPersistence")
    
    
while True:
    # Get object data
    time.sleep(0.5)
    print "loop"
    objper.get()
    print objper.Operation
    
    
    # Execute action
    retval = -1
    if (objper.Operation == "LOAD") :
        if (objper.Selection == "SINGLEOBJECT") :
            # Get selected object
            obj =  objMgr.getObjByID(objper.ObjectID)
            if not obj:
                print "can't load %s" % objper.ObjectID
                continue
            # Load selected instance
            retval = UAVObjLoad(obj, objper.InstanceID)
        if (objper.Selection == "ALLSETTINGS" or objper.Selection == "ALLOBJECTS"):
            retval = UAVObjLoadSettings()
        if (objper.Selection == "ALLMETAOBJECTS" or objper.Selection == "ALLOBJECTS"):
            retval = UAVObjLoadMetaobjects()
    elif (objper.Operation == "SAVE"):
        if (objper.Selection == "SINGLEOBJECT"):
            # Get selected object
            obj = objMgr.getObjByID(objper.ObjectID)
            if not obj:
                continue
            # Save selected instance
            retval = UAVObjSave(obj, objper.InstanceID)
            # Verify saving worked
            if (retval == 0):
                retval = UAVObjLoad(obj, objper.InstanceID)
        if (objper.Selection == "ALLSETTINGS" or objper.Selection == "ALLOBJECTS"):
            retval = UAVObjSaveSettings()
        if (objper.Selection == "ALLMETAOBJECTS" or objper.Selection == "ALLOBJECTS"):
            retval = UAVObjSaveMetaobjects()
    elif (objper.Operation == "DELETE"):
        if (objper.Selection == "SINGLEOBJECT"):
            # Get selected object
            obj = objMgr.getObjByID(objper.ObjectID)
            if not obj:
                continue
            # Delete selected instance
            retval = UAVObjDelete(obj, objper.InstanceID)
        elif (objper.Selection == "ALLSETTINGS" or objper.Selection == "ALLOBJECTS") :
            retval = UAVObjDeleteSettings()
        elif (objper.Selection == "ALLMETAOBJECTS" or objper.Selection == "ALLOBJECTS") :
            retval = UAVObjDeleteMetaobjects()
    elif (objper.Operation == "FULLERASE") :
        retval = UAVObjDeleteAll()

    #Update the operation in the object
    if retval == 0:
        objper.Operation = "COMPLETED"
        objper.set()
    else:
        objper.Operation = "ERROR"
        objper.set()
