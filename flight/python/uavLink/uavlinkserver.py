##
##############################################################################
#
# @file       uavlinkserver.py
# @author     Raspberry Pilot, http://code.google.com/p/raspberrypilot Copyright (C) 2013.
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


import SocketServer
import threading
import uavlink
import logging
import socket

#the streamServer handles multiple connections from GCS (or any serial sream)
class uavLinkServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    """" This server servs a uavlink connection on the network port"""
    daemon_threads = True
    allow_reuse_address = True
    def __init__(self,objMgr,host,port):
        SocketServer.TCPServer.__init__(self,(host,port),uavLinkServerHandler)
        self.handlers_list_lock = threading.RLock()
        self.handlers = []
        self.objMgr = objMgr
        #start it as a new thread
        t = threading.Thread(target=self.serve_forever)
        t.start()
    def register_handler(self,handler):
        self.handlers_list_lock.acquire()
        self.handlers.append(handler)
        self.handlers_list_lock.release()
    def deregister_handler(self,handler):
        self.handlers_list_lock.acquire()
        self.handlers.remove(handler)
        self.handlers_list_lock.release()
    def server_bind(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
        SocketServer.TCPServer.server_bind(self) 


class uavLinkServerHandler(SocketServer.StreamRequestHandler):
    """This is a StreamRequestHAndler for the streamServer"""
    timeout = 30
    def setup(self):
        SocketServer.StreamRequestHandler.setup(self)
        self.server.register_handler(self)
        self.objMgr = self.server.objMgr
        self.conn = uavlink.uavLinkConnection(self.objMgr,self.rfile.read,self.wfile.write)
    def handle(self):
        # the connection is already threaded so this thread does nothing
        self.conn.start()
        self.conn.disconnectEvent.wait()
    def finish(self):
        self.conn.close()
        self.server.deregister_handler(self)
        SocketServer.StreamRequestHandler.finish(self)
    def handle_timeout(self):
        self.server.deregister_handler(self)  
        self.finish()