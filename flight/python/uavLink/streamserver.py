import SocketServer
import threading
import Queue

#the streamServer handles multiple connections from GCS (or any serial sream)
class streamServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    """"There are two ways to get data out of this stream server, if a rx_handler is registered 
    then the class builds upt to read_len bytes or waits read_timeout then calls the handler 
    with the buffer.  The other builds up the buffer, the buffer can be read by the read function.  
    The read function will return read_len bytes or timeout after read_timeout."""
    daemon_threads = True
    allow_reuse_address = True
    def __init__(self,host, port):
        SocketServer.TCPServer.__init__(self,(host,port),streamServerHandler)
        self.handlers_list_lock = threading.RLock()
        self.rx_buffer_lock = threading.RLock()
        self.readEvent = threading.Event()
        self.handlers = []
        self.rx_buf = ""
        self.read_timeout = 0.1 # the read will block for at most this amount of time
        self.read_len = 75      # the read will return if more than this many bytes are available
        self.rx_handler = None
        #start it as a new thread
        t = threading.Thread(target=self.serve_forever)
        t.start()
    def call_rx_handler(self):
        if self.rx_handler == None:
            return
        self.rx_timer.cancel()
        self.rx_buffer_lock.acquire()
        if len(self.rx_buf):
            self.rx_handler(self.rx_buf)
        self.rx_buf = ""
        self.rx_buffer_lock.release()
        self.rx_timer = threading.Timer(self.read_timeout,self.call_rx_handler)
        self.rx_timer.start()
    def register_rx_handler(self,rx_handler):
        self.rx_handler = rx_handler
        self.rx_timer = threading.Timer(self.read_timeout,self.call_rx_handler)
        self.rx_timer.start()
    def register_handler(self,handler):
        self.handlers_list_lock.acquire()
        self.handlers.append(handler)
        self.handlers_list_lock.release()
    def deregister_handler(self,handler):
        self.handlers_list_lock.acquire()
        self.handlers.remove(handler)
        self.handlers_list_lock.release()
    def write(self,data):
        self.handlers_list_lock.acquire()
        for handler in self.handlers:
            handler.write(data)
        self.handlers_list_lock.release()
    def rx_data(self,data,source):
        "This function takes in data from the handlers and combines it"
        self.rx_buffer_lock.acquire()
        if source == self.handlers[0]:
            self.rx_buf += data
        if len(self.rx_buf) > self.read_len:
            if self.rx_handler:
                self.call_rx_handler()
            else:
                self.readEvent.set()
        self.rx_buffer_lock.release()
    def read(self):
        self.readEvent.wait(self.read_timeout)
        self.rx_buffer_lock.acquire()
        buf = self.rx_buf
        self.rx_buf = ""
        self.readEvent.clear()
        self.rx_buffer_lock.release()
        return buf

class streamServerHandler(SocketServer.StreamRequestHandler):
    """This is a StreamRequestHAndler for the streamServer"""
    timeout = 30
    def setup(self):
        SocketServer.StreamRequestHandler.setup(self)
        self.rxQueue = Queue.Queue()
        self.server.register_handler(self)
    def write(self,data):
        self.wfile.write(data)
    def handle(self):
        # the txThread will handle writing to this socket 
        # this thread's (the handler) purpose is to read from it
        # this is a thread because streamServer has the threadingMixIn
        while(True):
            data = self.rfile.read(1)
            self.server.rx_data(data,self)
    def finish(self):
        self.server.deregister_handler(self)
        SocketServer.StreamRequestHandler.finish(self)
    def handle_timeout(self):
        self.server.deregister_handler(self) 
        SocketServer.StreamRequestHandler.finish(self)        
        
