##
##############################################################################
#
# @file       objectManager.py
# @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
# @brief      Base classes for python UAVObject
#   
# @see        The GNU Public License (GPL) Version 3
#
#############################################################################/
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#



import logging
import sys
import os
import inspect

from openpilot.uavtalk.uavobject import *



class objManagerTimeoutException(Exception): 
    pass 

class ObjManager(object):
    
    def __init__(self):
        self.objs = {}
        
    def addObj(self, obj):
        obj.objMan = self
        self.objs[obj.objId] = obj
        
    def getObj(self, objId):
        try:
            return self.objs[objId]
        except KeyError:
            return None
        
    def getObjByName(self, name):
        for objId, obj in self.objs.items():
            if obj.name == name:
                return obj
        return None
        
    def importDefinitions(self, uavObjDefPath=None):
        # when the uavObjDefPath is nor defined, assume it is installed together with this module
        if uavObjDefPath == None:
            currModPath = os.path.dirname(sys.modules[__name__].__file__)
            uavObjDefPath = os.path.join(currModPath, "..", "uavobjects")
        
        logging.info("Importing UAVObject definitions from %s" % uavObjDefPath)
        sys.path.append(uavObjDefPath)
        for fileName in os.listdir(uavObjDefPath):
            if fileName[-3:] == ".py":
                logging.debug("Importing from file %s", fileName)
                module = __import__(fileName.replace(".py",""))
                for name in dir(module):
                    klass = getattr(module, name)
                    obj = getattr(module, name)
                    if inspect.isclass(obj):
                        if name != "UAVObject"  and name != "UAVMetaDataObject"  and name != "UAVDataObject"  and issubclass(klass, UAVObject):
                            logging.debug("Importing class %s", name)
                            obj = klass()
                            obj.name = name
                            setattr(self, name, obj)
                            self.addObj(obj)
                            metaObj = UAVMetaDataObject(obj.getMetaObjId())
                            obj.metadata = metaObj
                            metaObj.name = "Meta[%s]" % name
                            self.addObj(metaObj)
    
    def regObjectObserver(self, obj, observerObj, observerMethod):
        o = Observer(observerObj, observerMethod)
        obj.observers.append(o)
        
    def requestObjUpdate(self, obj, requestor=None):
        logging.debug("Requesting %s" % obj)
        logging.debug("Need to forward request to relevant interfaces")
        #self.uavTalk.sendObjReq(obj)
        raise
    
    def waitObjUpdate(self, obj, timeout=.5, requestor=None):
        logging.debug("Waiting for %s " % obj)
        cnt = obj.updateCnt
        obj.updateEvent.acquire()
        obj.updateEvent.wait(timeout)
        obj.updateEvent.release()
        timeout = (cnt == obj.updateCnt)
        logging.debug("-> Waiting for %s Done. " % (obj))
        if timeout:
            s = "Timeout waiting for %s" % obj
            logging.debug(s)
            raise TimeoutException(s)
                    
    def objUpdate(self, obj, rxData, updater):
        obj.deserialize(rxData)
        self.objUpdated(obj)
        
    def objUpdated(self, obj):
        logging.debug("objUpdated %s, triggering observers and events" % obj)
        obj.updateCnt += 1
        for observer in obj.observers:
            observer.call(obj)
        obj.updateEvent.acquire()
        obj.updateEvent.notifyAll()
        obj.updateEvent.release()
        

        

        

        

     
    
                
