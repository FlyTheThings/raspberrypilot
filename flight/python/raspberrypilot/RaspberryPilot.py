import threading
import inspect
import time
from raspberrypilot import uavlink

class raspberryPilotModule(threading.Thread):
    def __init__(self,objMgr,printing = True):
        threading.Thread.__init__(self)
        self.objMgr = objMgr
    def set_core(self,core):
        self.core = core
    def is_running():
        return self.core.is_running()

    
class RaspberryPilot():
    def __init__(self,serv_addr,serv_port):
        '''arguments are addr and port of uavlink_udp server'''
        self.serv_addr = serv_addr
        self.serv_port = serv_port
        self.import_raspberry_pilot_moddules()
        self.running_modules = []
    def is_running(self):
        #when running is false the child threads should die
        return self.running
    def import_raspberry_pilot_moddules(self):
        import raspberrypilot.modules
        self.raspberrypilot_modules = {}
        for name,mod in inspect.getmembers(raspberrypilot.modules):
            if inspect.ismodule(mod):
                for name,obj in inspect.getmembers(mod):
                    if inspect.isclass(obj):
                        if issubclass(obj,raspberrypilot.raspberryPilotModule):
                            self.raspberrypilot_modules[name] = obj
    def get_raspberry_pilot_modules(self):
        return self.raspberrypilot_modules.values()
    def start_all(self):
        self.running = True
        for raspberry_module in self.get_raspberry_pilot_modules():
            #each module has its own connection and objectmanager
            #that way each thread is completely independed and one cannot lock out other threads
            conn = uavlink.uavLinkConnection_UDP(addr=self.serv_addr,port=self.serv_port)
            objMgr = uavlink.objManager(conn)
            mod = raspberry_module(objMgr)
            mod.set_core(self)
            mod.start()
            self.running_modules.append(mod)

if __name__ == "__main__":
    rp = RaspberryPilot("192.168.1.115",32001)
    rp.start_all()
    try:
        while(True):
            time.sleep(100)
    except  KeyboardInterrupt:
        exit()
            
    
    