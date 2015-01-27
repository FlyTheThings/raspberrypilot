#!/usr/bin/env python
import time
import raspberrypilot
from raspberrypilot import rp_core

rp = rp_core.RaspberryPilot_Server("127.0.0.1",32001)
rp.start_all()
try:
    while(True):
        time.sleep(1000)
except  KeyboardInterrupt:
    exit()

