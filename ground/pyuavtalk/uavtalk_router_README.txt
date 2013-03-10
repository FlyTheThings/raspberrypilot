python modules (flight computer) maintain input and output objects seperately in their object manager, they only reply to a request for an output object, they never send input objects if requested (readonly) 

flight software must nack request for objects it does not have 

each connection maintains own meta data update rate/type of updates???????
multiple connection levels ground/pythonflight/stm32
on receiving object ack 
	1st forward to everyone and collect ack/nack from everyone and return or of ack/nacks only after all have replied
	after that only forword objects acks to those who have acked
on recieving an object request
	1st time forward to everyone, python replies are prioritzed higher than autopilot replies for response, since they only respond for objects they own
	after that if a python level reply for that object has been received only request from python




	
what happens if flight receives and obj_ack and does not have the object? nothing now requires minor modification to protocol



Router connection centric model
	One core thread, another for each connection
	Standard Data:
	The one core thread simply receives a packet from on connection thread and forwards it to each other thread according to the following rules:
		General Routing Rules for obj and obj_ack packets:
			packets from telemetry threads are forwarded to all non-telemetry threads except source of packet
			packets from auto pilot thread are forward to all threads except source of packet
			packets from flight computer threads are forward to all threads except source of packet
		Special Exeption To these rules:
			Once an obj or obj_ack has been received from a flightcomputer connection it is added to the exclusive owner list
		when an obj_ack is recieved an item in a dictionary is created
			the dictionary entry is cleared on any subsequent obj act
			objectid = {requestiong connection,responces_required, responces recieved, an ack} = {requesting connection,num_connections,0,false}
				in the above if the request originates from a telemetry connection responses_required is the number of autopilot and 
			the obj ack is then forwarded to all threads according to rules
		ack/nack resposnses are held if there objid is in the dictionary of pending acks
			once all required replies are in the reply is forwared to the requestor
		disconnection:
			the disconnection of a telemetry connection is a non event 
	All communication from these threads are into one input queue for core, the source connection and the packet are included
	Meta data:
	upon receiving a meta data update, the core thread updates its copy of the ports metadata and 
	

Connection Thread
	runs uavtalk on a connection
	each connection has a type to identify it to the core, types are telemetry, flightcomputer, autopilot 
	maintains its own meta data list for known objects:
	From uavTalk:
		upon seeing a new object object_ack or ack, local meta data is set to manual and the meta data is requested
		upon receiving a meta data object or object ack the local meta data is updated 
			and if an object_ack an ack is issued
			the meta data update is sent to the core
			the meta data used depends on the connection type
				telemetry connections use flight data
				autopilot connections use gcsdata
				flightcomputer connections use the most agressive of the two
		upon receiving an object, object ack, object req, or nack it is forwarded to the core
		upon receiving a nack, the object is added to the excluded list, except for telemetry?
	From Core:
		upon receiving a meta data object or meta data object ack it is sent out to uavtalk connection
		upon receiving a ack or nack sent if out to uavtalk connection
		upon receiving an object it is checked against excluded lists, if not excluded it is sent to the uavtalk connection















******************************* Alternative version, probably better ****************************************************************


The uavtalk protocol was designed to be a two way, end point to end point protocol for communicating uavobjects
There are 5 types of packets objects (obj), objects with ackwoledgement requests (obj_ack), object requests (obj_req), acknowldege (ack), and negative acknowledge (nack), each of these packets contains a start byte, a length, an object_id a checksum and possibly the object data (only obj and obj_ack contain data).  There is also  an optional instance number for multi instance uavobjects that appears in the data field, however as object_id's are currently generated it is not possible to decode from the object_id if an object has multiple instances, without a database of all object_ids.  A limitation of the proposed architure will be that all instances of the same object should be routed the same way. With this exception it is fully possible to process packets blind, without any understanding of their internal data.

There are currently 3 supported "transactions" in the protocol.  
1) objects: obj packets may be sent un-requrested either due to an internal change at that endpoint or periodically.  this is a one way communication without a response from the reciever.  This is by far the most common type of communication in the system.
2) object request: one end point sends an obj_req packet, the response from the other end point is on obj packet. If the object does not exist the response is a nack
3) object with acknowledge:  one end point sends an object with a request for an acknowledgement of its reciept. the response from the receiver is to send an ack, currently if it knows about the object, otherwise there is no response. A nack may be more appropriate but this is not how it currently works

On top of this communication is governed by metadata about each object.  This metadata is also a uavobject that is communicated in the same way as an object.  All meta data is governed by one metadata meta object.  I beleive it is safe to ignore this as it is effectively constant and never transmitted.  This metadata determines, amoung other things, when objects are supposed to be sent out over uavtalk.  It is important to us because it is easy to imagine a case where routing between a telemetry station, the autopilot and a flying computer the autopilot may provide data at high rate to the flying computer, however this data should not be blindly copied at high rate to the telemetry.


Keeping in mind that uavtalk was designed with only a flight endpoint and gcs endpoing in mind the meta data for each object is as follows in the autopilot code:
	 *
	 * The object metadata flags are packed into a single 8 bit integer.
	 * The bits in the flag field are defined as:
	 *
 	*   Bit(s)  Name                     Meaning
	 *   ------  ----                     -------
	 *      0    access                   Defines the access level for the local transactions (readonly=1 and readwrite=0)
	 *      1    gcsAccess                Defines the access level for the local GCS transactions (readonly=1 and readwrite=0), not used in the flight s/w
 	*      2    telemetryAcked           Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
	 *      3    gcsTelemetryAcked        Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
	 *    4-5    telemetryUpdateMode      Update mode used by the telemetry module (UAVObjUpdateMode)
	 *    6-7    gcsTelemetryUpdateMode   Update mode used by the GCS (UAVObjUpdateMode)
	 */
	typedef struct {
		uint8_t flags; /** Defines flags for update and logging modes and whether an update should be ACK'd (bits defined above) */
		uint16_t telemetryUpdatePeriod; /** Update period used by the telemetry module (only if telemetry mode is PERIODIC) */
		uint16_t gcsTelemetryUpdatePeriod; /** Update period used by the GCS (only if telemetry mode is PERIODIC) */
		uint16_t loggingUpdatePeriod; /** Update period used by the logging module (only if logging mode is PERIODIC) */
	} __attribute__((packed)) UAVObjMetadata;
	/**
	 * Object update mode, used by multiple modules (e.g. telemetry and logger)
	 */
	typedef enum {
		UPDATEMODE_MANUAL = 0, /** Manually update object, by calling the updated() function */
		UPDATEMODE_PERIODIC = 1, /** Automatically update object at periodic intervals */
		UPDATEMODE_ONCHANGE = 2, /** Only update object when its data changes */
		UPDATEMODE_THROTTLED = 3 /** Object is updated on change, but not more often than the interval time */
	} UAVObjUpdateMode;


Of this meta data the loggingUpdatePeriod, telemtryacked, gcstelemtryacked can be ignored as they effect internal behavoir of the endpoint irrelevant to routing the packets.  Also The data about update mode and period can be used to determine how often packets should be forwarded to each link. The gcsTelemtryUpdateMode and gcsTelemetryUpdatePeriod only effect how often gcs sends out the few objects it does, we can probably safely ignore it as we are only working on the telemetry side link.
To better understand how to use these in routing thinking about the simple case of a router between the flight side and the ground side.  The sides could easily be thought of as an upstream side (flight) and  a downstream side (gcs).  Any updating of the meta data will come from the downstream side (gcs), this is because it is what manages the telemetry.  There is no reason the autopilot (flight) would change its meta data on its own.  The gcs can change data rates or update modes for various reasons (such as HITL simulation, in this case it also changes access). Changes from the ground about access must always fully propagate to the autopilot to facilitate simulation and debugging.  However update mode and rate changes must be propigated more carefully if there are more than two endpoints in the system communicating at different rates.

While the protocol is symetrical the data flow is not.  The router should not be designed to facilitate all possible network topologies, this would have numerous addition complications.  However there is one very likely topology that will allow assumptions to be made. The topology the router will support is up to one upstream connection to the autopilot, zero to multiple downstream connections (either to gcs's or to the upstream connection of another router), and zero to multiple midstream connections.  Data should not be sent where it is not needed.  Each connection should maintain its own meta data about object data rates and limit bandwidth over that channel to the requested rates.  Also object requests should only propigate to where they are needed, same with acks and nacks.  Downstream connections should not cross talk, each downstream connection the midstream connections and upstream connection should appear to be one consistant collection of uavobjects obeying there metadata and responding to requests.

the problem becomes integrating the midstream connections with the upstream connection to make them both appear as one upstream connection to the downstream connections.  to do this it is nessasary to know which objects go upstream and which go to which midstream connection. it may be possible to do this with traffic analysis over time however this becomes complicated.  A simple solution is a small extensino to the uavtalk protocol.  A new type of packet that only midstream connections will speak, a register object packet (obj_reg).  This is a way a midstream endpoint can tell the router it will be responsible for that type of object and not to send downstream packets about that object upstream but instead to simply intercept them and send them to the appropriate midstream connection.  This requires that multiple midstream endpoints not register for the same object. this should be handled by the system design just as the gcs and flight endpoints dont try to write to the same object at the same time.

One other issue is the default object meta data is generated containing the telemetry update type and rate.  Since this is intended to be telemetry update type/rate back to the ground station update type/rates to the midstream connections will have to be setup manually by that midstream connection.  There is no clean way to define how quickly the midstream connections should talk to eachother or to the upstream connection.  This will complicating the creationg of midstream endpoints minimizes changes to the rest of the system.
s
One other benifit to this type of system is that it can be used to conserve bandwidth for multiple downstream connections over a limited link.  If multiple gcs or other telmetry stations are desired it would be inefficient for both of them to connect to the flying router.  A better configuration would be to create a second router on the ground connect its upstream connection to a downstream connection on the flying router and then connect all gcs's to the router on the ground.  This way all the obj's being sent by the upstream connection on the flight router will only need to cross the bandwidth limited link to the ground once and the ground router would relay them to all the gcs's.




the model for the router is:
	One core thread, another for each connection
	The one core thread simply receives a packet from on connection thread and forwards it to each other thread according to the following rules:
		General Routing Rules for obj and obj_req packets:
			packets from telemetry are forwared to a flightcomputer if in the owner database, otherwise to autopilot
			packets from auto pilot thread are forwarded to all connections, unless a flight computer owns that object
			packets from flight computer threads are forward to all other threads except source of packet
		when an obj_ack is recieved 
			if there is not a pending obj_ack for that object or if the current one has timed out
				the obj id and requesting connection and time are stored
			the ack is then forwarded to the requesting connection
			if there is an untimed out obj_ack in the dictionary the new obj_ack is dropped
		ack nack responses are checked against the ack dictionary and forwared to the requesting connection
			unrequested ack/nack that are not in the dictionary are dropped
		when receiving a metadata input
			the core metadata database of each object for each connection is updated
			the objects owner (default autopilot, flight computer if registered) written to the most agressive other midstream or downstream rate
		ogj_reg packets
			the sender is entered into the obj owner database
		disconnection:
			the entry from the object owner database is removed
			relevant entries in ack/nack database are removed
			entries from the meta data database are removed

	All data communication from these threads are into one input queue for core, the source connection and the packet are included

	

Connection Thread
	runs uavtalk on a connection
	each connection has a type to identify it to the core, types are telemetry, flightcomputer, autopilot 
	maintains its own meta data list for known objects:
	From uavTalk:
		upon seeing a new object object_ack or ack, local meta data is set to manual and the meta data is requested
		upon receiving a meta data object or meta data object ack the local meta data is updated
			the data is sent to the core 
			and if an object_ack an ack is issued
			the local meta data is updated
		upon receiving an object, object ack, object req, or nack it is forwarded to the core
	From Core:
		upon receiving a meta data object or meta data object ack it is sent out to uavtalk connection
		upon receiving a ack or nack sent if out to uavtalk connection
		upon receiving an object it is sent to the uavtalk connection if the meta data requirements are satisfied



	