##
##############################################################################
#
# @file       uavobject.py
# @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

"""
  Modified significantly from open pilot

"""

import struct


class uavObjectField():
    class FType:
        INT8 = 0
        INT16 = 1
        INT32 = 2
        UINT8 = 3
        UINT16 = 4
        UINT32 = 5
        FLOAT32 = 6
        ENUM = 7
         
    def __init__(self, ftype, numElements):
        self.ftype = ftype
        self.numElements = numElements
        if self.ftype == uavObjectField.FType.INT8:
            vfmt = "b"
            self.rawSizePerElem = 1
        elif self.ftype == uavObjectField.FType.UINT8 or self.ftype == uavObjectField.FType.ENUM:
            vfmt = "B"
            self.rawSizePerElem = 1
        elif self.ftype == uavObjectField.FType.INT16:
            vfmt = "h"
            self.rawSizePerElem = 2
        elif self.ftype == uavObjectField.FType.UINT16:
            vfmt = "H"
            self.rawSizePerElem = 2
        elif self.ftype == uavObjectField.FType.INT32:
            vfmt = "i"
            self.rawSizePerElem = 4
        elif self.ftype == uavObjectField.FType.UINT32:
            vfmt = "I"
            self.rawSizePerElem = 4
        elif self.ftype == uavObjectField.FType.FLOAT32:
            vfmt = "f"
            self.rawSizePerElem = 4
        else:
            raise ValueError()
        fmt = "<" + vfmt*numElements
        self.struct = struct.Struct(fmt)
        self.fmt = fmt
        if ftype == uavObjectField.FType.FLOAT32:
            baseValue = 0.0
        else:
            baseValue = 0
        if numElements == 1:
            self.value = baseValue
        else:
            self.value = [baseValue]* numElements
        self.rawSize = self.rawSizePerElem * self.numElements
    def getRawSize(self):
        return self.rawSize
    def serialize(self):
        if self.numElements == 1:
            ser = self.struct.pack(self.value)
        else:
            ser = self.struct.pack(*self.value)
        return ser
    def deserialize(self, data):
        # DOTO: FIXME: This is getting very messy
        values = list(self.struct.unpack("".join(data[:self.rawSize])))
        if self.numElements == 1:
            self.value = values[0]
        else:
            self.value = values
        return self.rawSize
    def setValue(self,value):
        #this has been added for error checking later TODO
        self.value = value

          
class uavObject(object):
    def __init__(self, objId):
        self.objId = objId
        self.instId = 0
        self.fields = []
        self.objMgr = None
    def setObjManager(self,objMgr):
        self.objMgr = objMgr
    def addField(self, field):
        self.fields.append(field)
    def getSerialisedSize(self):
        size = 0
        for field in self.fields:
            size += field.getRawSize()
        return size
    def isSingleInstance(self):
        return self.isSingleInst == True
    def unpackData(self, data):
        p = 0
        for field in self.fields:
            p += field.deserialize(data[p:])
    def getPackedData(self):
        ser = ""
        for field in self.fields:
            ser += field.serialize()
        return ser
    def __str__(self):
        if self.name != None:
            return "uavObject: %s" % self.name
        else:    
            return "UAVObj %08x" % self.objId
    def getInstanceId(self):
        return self.instId
        "return the instance id if multinstance or none"
    def read(self):
        if self.objMgr:
            self.objMgr.getObj(self)
    def write(self):
        if self.objMgr:
            self.objMgr.setObj(self)



