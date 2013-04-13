from distutils.core import setup
import glob



setup(name='raspberrypilot.uavlink',
      version='1.0',
      url="http://code.google.com/p/raspberrypilot",
      description='OpenPilot/RaspberryPilot UavLink (modified uavTalk)',
      packages=['raspberrypilot/uavlink'],
     )

setup(name='raspberrypilot.modules',
      version='0.1',
      url="http://code.google.com/p/raspberrypilot",
      description='OpenPilot/RaspberryPilot Python Modules',
      packages=['raspberrypilot/modules'],
     )
     
setup(name='raspberrypilot',
      version='1.0',
      url="http://code.google.com/p/raspberrypilot",
      description='RaspberryPilot (openpilot)',
      packages=['raspberrypilot'],
     )