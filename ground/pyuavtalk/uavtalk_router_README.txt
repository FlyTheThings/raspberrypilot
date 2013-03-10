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
		packets from telemetry threads are forwarded to all non-telemetry threads except source of packet
		packets from auto pilot thread are forward to all threads except source of packet
		packets from flight computer threads are forward to all threads except source of packet
		when an obj ack is recieved an item in a dictionary is created
			the dictionary entry is cleared on any subsequent obj act
			objectid = {requestiong connection,responces_required, responces recieved, an ack} = {requesting connection,num_connections,0,false}
			the obj ack is then forwarded to all threads according to rules
		ack/nack resposnses are held if there objid is in the dictionary of pending acks
			once all required replies are in the reply is forwared to the requestor 
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
	