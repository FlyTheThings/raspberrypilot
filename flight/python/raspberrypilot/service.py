#!/usr/bin/env python
import time
import raspberrypilot

rp = raspberrypilot.RaspberryPilot("127.0.0.1",32001)
rp.start_all()
try:
    while(True):
        time.sleep(1000)
except  KeyboardInterrupt:
    exit()
    