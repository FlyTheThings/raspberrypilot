How the python uavlink works:

There are 6 main components:
	uavlinkprotocol
	objmanager
	uavlinkconnection
	uavlinkserver
	streamserver
	uavobject
	
uavlinkprotocol - 
contains the byte stream parser, the crc, and methods to send packets of all types, two callbacks 
may be registered, one to send data, one on receiving a packet

uavlinkconnection - 
this is a threaded connection that handles talking uavtalk on an actual interface.  It is threaded 
with two threads, tx and rx.  When created it is given a functions to read and write.  It may be given an 
object manager or None.  If it has an object manager it will pass received object packets to it.  A rxStream 
callback may registerd with it, when receiving a stream packet or the correct ID this callback will be called.
It also makes use of a uavlinkconnectiontransaction object to keep track of pending transactions, it may have 
up to a limited number of these. It presents functions fo transacting uavobjects and streams.

objmanager - 
Unlike the object manager on the microcontroller and in the gcs this one does not store any object 
data, it requests it on the fly from a uavlinkconnection.  It searches uavlink.uavobjects for objects and allows 
users to request them by name or id.  When an object is requested from the object manager, it forwards the request 
to its uavlink connection.  It returns an instance of the correct type of object with itself registered as the 
object's manager.

uavobject - 
This is the base class all the generated uavobjects inherit from.  It contains a number of uavobjectfield 
objects and faciliates serialization/deserialization for forming uavlink packets.  It also has a get and set methods, 
get requests an update from the object manager and updates the local data, set writes the local data to the object 
manager.

streamserver -
The stream servers purpose is to act as a bridge between TCP/IP and encapsulated serial streams over uavlink.  It opens a 
port where connections effectively become the de-encapsulated serial stream.  It's primary purpose is to support a uavtalk 
session with the gcs.  On the raspberry pilot microcontroller streamID 1 is telemetry.  This link is designed to support 
multiple connections. Due to serial point to point nature arbitration is needed. The arbitration is simply the first to 
connect can send, everyone can receive (not what the first writes though).  If the first disconnects, the second connected 
gains write access.

uavlinkserver -
This is a server to talk uavlink to multiple tcp/ip connections.  Each TCP/IP connection gets a uavlinkconnection instance 
with the servers object manager.  This allows multiple network connections to the the same object manager.

 
On the raspberry pi there will be proxy with a uavlinkconnection over serial to the microcontroller.  This connection will not 
have an object manager. One objectmanager will be created to use this connection. One serialserver will be set up to bridge 
telemetry to the uavlink connection to the microcontroller.  One uavlinkserver will be setup to allow multiple uavlink connections
to the object manager.
