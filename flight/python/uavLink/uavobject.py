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


class uavObjectField:
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
    def serialize(self, ser):
        if self.numElements == 1:
            ser += map(ord, self.struct.pack(self.value))
        else:
            ser += map(ord, apply(self.struct.pack, self.value))
    def deserialize(self, data):
        # DOTO: FIXME: This is getting very messy
        values = list(self.struct.unpack("".join(map(chr,data[:self.rawSize]))))
        if self.numElements == 1:
            self.value = values[0]
        else:
            self.value = values
        return self.rawSize

          
class uavObject:
    def __init__(self, objId, name=None):
        self.objId = objId
        self.instId = 0
        self.fields = []
        if name:
            self.name = name
        else:
            self.name = self.__class__.__name__
    def addField(self, field):
        self.fields.append(field)
    def getSerialisedSize(self):
        size = 0
        for field in self.fields:
            size += field.getRawSize()
        return size
    def unpackData(self, data):
        p = 0
        for field in self.fields:
            p += field.deserialize(data[p:])
    def getPackedData(self):
        ser = []
        for field in self.fields:
            field.serialize(ser)
        return ser
    def __str__(self):
        if self.name != None:
            if self.isMetaData():
                return "%s" % self.name
            else:
                return "%s" % self.name
        else:    
            if self.isMetaData():
                return "UAVMetaObj of %08x" % (self.objId-1)
            else:
                return "UAVObj %08x" % self.objId
    def getInstanceId(self):
        return self.instId
        "return the instance id if multinstance or none"



