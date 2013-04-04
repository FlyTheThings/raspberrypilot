/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 *
 * @file       uavlink.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      UAVLink library is a modified version of UAVTalk.  It is intended to 
 *             link an onboard flight computer to the openpilot board.  It is also designed 
 *             to transport serial streams, such as UAVTalk over it.
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *  Modified Version of uavlink, this is unthreaded and intended to build in C on linux
 *  It is inteded to run singe threaded so all freertos locks have been removed
 *  It is also modified to be "blind" it handles everything with no knowledge of uavobject definitions
 *  this means it cant know about instances at all
 *
 * Further assumpts, uavlink is asymetric
 *  flight computer sends obj_ack obj and obj_req to flight uc
 *  flight uc replies to these
 *  flight uc or computer can send streams at any time
 *  flight uc does not request objects from flight computer
 *
 */
#include "uavlink.h"
#include "uavlink_priv.h"


// Private functions
static int32_t sendNack(UAVLinkConnectionData *connection, uint32_t objId);
static int32_t receivePacket(UAVLinkConnectionData *connection, uint8_t type, uint32_t rxId, uint16_t instId, uint8_t* data, int32_t length);
static void updateAck(UAVLinkConnectionData *connection, uint32_t rxId);
static int32_t sendAck(UAVLinkConnectionData *connection, uint32_t Id);
static int32_t sendStreamPacket(UAVLinkConnection connectionHandle, uint8_t Id, uint8_t length, uint8_t *buf);
UAVLinkRxState UAVLinkProcessInputStreamQuiet(UAVLinkConnection connection, uint8_t rxbyte);


/**
 * Initialize the UAVLink library
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] outputStream Function pointer that is called to send a data buffer
 * \return 0 Success
 * \return -1 Failure
 */
UAVLinkConnection UAVLinkInitialize(UAVLinkOutputStream outputStream)
{
	// allocate object
	UAVLinkConnectionData * connection = malloc(sizeof(UAVLinkConnectionData));
	if (!connection) return 0;
	connection->canari = UAVLINK_CANARI;
	connection->iproc.rxPacketLength = 0;
	connection->iproc.state = UAVLINK_STATE_SYNC;
	connection->outStream = outputStream;
	connection->streamForwarder = 0;
	// allocate buffers
	connection->rxBuffer = malloc(512);       // stores the data in the packet
	if (!connection->rxBuffer) return 0;
	connection->txBuffer = malloc(512);
	if (!connection->txBuffer) return 0;
	connection->rxPacketBuffer = malloc(512); // stores the entire packet
	if (!connection->rxPacketBuffer) return 0;
	UAVLinkResetStats( (UAVLinkConnection) connection );
	return (UAVLinkConnection) connection;
}

/**
 * Set the communication output stream
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] outputStream Function pointer that is called to send a data buffer
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVLinkSetOutputStream(UAVLinkConnection connectionHandle, UAVLinkOutputStream outputStream)
{

	UAVLinkConnectionData *connection;
    CHECKCONHANDLE(connectionHandle,connection,return -1);

	// set output stream
	connection->outStream = outputStream;
	
	return 0;

}

/**
 * Get current output stream
 * \param[in] connection UAVLinkConnection to be used
 * @return UAVTarlkOutputStream the output stream used
 */
UAVLinkOutputStream UAVLinkGetOutputStream(UAVLinkConnection connectionHandle)
{
	UAVLinkConnectionData *connection;
    CHECKCONHANDLE(connectionHandle,connection,return NULL);
	return connection->outStream;
}


/**
 * Set the stream forwarder
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] the forwarder to use
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVLinkSetStreamForwarder(UAVLinkConnection connectionHandle, uavLinkStreamForwarder forwarder)
{

	UAVLinkConnectionData *connection;
    CHECKCONHANDLE(connectionHandle,connection,return -1);

	// set output stream
	connection->streamForwarder = forwarder;


	return 0;

}


/**
 * Get communication statistics counters
 * \param[in] connection UAVLinkConnection to be used
 * @param[out] statsOut Statistics counters
 */
void UAVLinkGetStats(UAVLinkConnection connectionHandle, UAVLinkStats* statsOut)
{
	UAVLinkConnectionData *connection;
    CHECKCONHANDLE(connectionHandle,connection,return );

	// Copy stats
	memcpy(statsOut, &connection->stats, sizeof(UAVLinkStats));
	
}

/**
 * Reset the statistics counters.
 * \param[in] connection UAVLinkConnection to be used
 */
void UAVLinkResetStats(UAVLinkConnection connectionHandle)
{
	UAVLinkConnectionData *connection;
    CHECKCONHANDLE(connectionHandle,connection,return);

	// Lock
	//xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
	
	// Clear stats
	memset(&connection->stats, 0, sizeof(UAVLinkStats));
	
	// Release lock
	//xSemaphoreGiveRecursive(connection->lock);
}



/**
 * Execute a transaction while forwarding a stream.
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] objId of object
 *  * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] type Transaction type
 * 			  UAVLINK_TYPE_OBJ: send object,
 * 			  UAVLINK_TYPE_OBJ_REQ: request object update
 * 			  UAVLINK_TYPE_OBJ_ACK: send object with an ack
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVLinkSendStream(UAVLinkConnection connectionHandle, uint8_t Id, uint8_t *buf, uint8_t length)
{
	UAVLinkConnectionData *connection;
	CHECKCONHANDLE(connectionHandle,connection,return -1);

	// Send stream and wait for ack
	connection->resp = 0;
	connection->respId = Id;
	sendStreamPacket(connection, Id, length, buf);
	return 0;
}

/**
 * Process an byte from the telemetry stream.
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] rxbyte Received byte
 * \return UAVLinkRxState
 */
UAVLinkRxState UAVLinkProcessInputStreamQuiet(UAVLinkConnection connectionHandle, uint8_t rxbyte)
{
	UAVLinkConnectionData *connection;
    CHECKCONHANDLE(connectionHandle,connection,return -1);

	UAVLinkInputProcessor *iproc = &connection->iproc;
	++connection->stats.rxBytes;

	if (iproc->state == UAVLINK_STATE_ERROR || iproc->state == UAVLINK_STATE_COMPLETE || iproc->state == UAVLINK_STATE_STREAM_COMPLETE)
		iproc->state = UAVLINK_STATE_SYNC;
	
	if (iproc->rxPacketLength < 512)
		iproc->rxPacketLength++;   // update packet byte count
	
	connection->rxPacketBuffer[iproc->rxPacketLength] = rxbyte;

	// Receive state machine
	switch (iproc->state)
	{
		case UAVLINK_STATE_SYNC:
			if (rxbyte != UAVLINK_SYNC_VAL)
				break;
			
			// Initialize and update the CRC
			iproc->cs = CRC_updateByte(0, rxbyte);
			
			iproc->rxPacketLength = 1;
			connection->rxPacketBuffer[0] = rxbyte;
			
			iproc->state = UAVLINK_STATE_TYPE;
			break;
			
		case UAVLINK_STATE_TYPE:
			
			// update the CRC
			iproc->cs = CRC_updateByte(iproc->cs, rxbyte);
			
			if ((rxbyte & UAVLINK_TYPE_MASK) != UAVLINK_TYPE_VER)
			{
				iproc->state = UAVLINK_STATE_ERROR;
				break;
			}
			
			iproc->type = rxbyte;
			
			iproc->packet_size = 0;
			
			iproc->state = UAVLINK_STATE_SIZE;
			iproc->rxCount = 0;
			break;
			
		case UAVLINK_STATE_SIZE:
			// update the CRC
			iproc->cs = CRC_updateByte(iproc->cs, rxbyte);
			
			if (iproc->rxCount == 0)
			{
				iproc->packet_size += rxbyte;
				iproc->rxCount++;
				break;
			}
			
			iproc->packet_size += rxbyte << 8;
			
			if (iproc->packet_size < UAVLINK_MIN_PACKET_SIZE || iproc->packet_size > UAVLINK_MAX_PACKET_LENGTH)
			{   // incorrect packet size
				iproc->state = UAVLINK_STATE_ERROR;
				break;
			}
			
			iproc->rxCount = 0;
			iproc->rxId = 0;
			if (iproc->type == UAVLINK_TYPE_STREAM)
			{
				iproc->state = UAVLINK_STATE_STREAMID;
			} else {
				iproc->state = UAVLINK_STATE_OBJID;
			}
			break;
			
		case UAVLINK_STATE_STREAMID:
			// update the CRC
			iproc->cs = CRC_updateByte(iproc->cs, rxbyte);
			iproc->rxId = rxbyte;
			iproc->length = iproc->packet_size - iproc->rxPacketLength;
			iproc->state = UAVLINK_STATE_DATA;
			iproc->rxCount = 0;
			break;
		case UAVLINK_STATE_OBJID:
			// update the CRC
			iproc->cs = CRC_updateByte(iproc->cs, rxbyte);
			iproc->rxId += rxbyte << (8*(iproc->rxCount++));

			if (iproc->rxCount < 4)
				break;
			
			// Determine data length
			if (iproc->type == UAVLINK_TYPE_OBJ_REQ || iproc->type == UAVLINK_TYPE_ACK || iproc->type == UAVLINK_TYPE_NACK)
			{
				iproc->length = 0;
				iproc->instanceLength = 0;
			}
			else
			{
				// We don't know if it's a multi-instance object, so just assume it's 0.
				iproc->instanceLength = 0;
				iproc->length = iproc->packet_size - iproc->rxPacketLength;
			}
			
			// Check length and determine next state
			if (iproc->length >= UAVLINK_MAX_PAYLOAD_LENGTH)
			{
				connection->stats.rxErrors++;
				iproc->state = UAVLINK_STATE_ERROR;
				break;
			}
			
			if (iproc->type == UAVLINK_TYPE_NACK)
			{
				// If this is a NACK, we skip to Checksum
				iproc->state = UAVLINK_STATE_CS;
			}
			else
			{
				// If there is a payload get it, otherwise receive checksum
				if (iproc->length > 0)
					iproc->state = UAVLINK_STATE_DATA;
				else
					iproc->state = UAVLINK_STATE_CS;
			}
			iproc->rxCount = 0;
			
			break;
		case UAVLINK_STATE_DATA:
			
			// update the CRC
			iproc->cs = CRC_updateByte(iproc->cs, rxbyte);
			
			connection->rxBuffer[iproc->rxCount++] = rxbyte;
			if (iproc->rxCount < iproc->length)
				break;
			
			iproc->state = UAVLINK_STATE_CS;
			iproc->rxCount = 0;
			break;
			
		case UAVLINK_STATE_CS:
			
			// the CRC byte
			if (rxbyte != iproc->cs)
			{   // packet error - faulty CRC
				connection->stats.rxErrors++;
				iproc->state = UAVLINK_STATE_ERROR;
				break;
			}
			if (iproc->type == UAVLINK_TYPE_STREAM) 
			{
				connection->stats.rxStreamBytes += iproc->length;
				connection->stats.rxStreamPackets++;
				iproc->state = UAVLINK_STATE_COMPLETE;
			} else {
				if (iproc->rxPacketLength != (iproc->packet_size + 1))
				{   // packet error - mismatched packet size
					connection->stats.rxErrors++;
					iproc->state = UAVLINK_STATE_ERROR;
					break;
				} else {
					connection->stats.rxObjectBytes += iproc->length;
					connection->stats.rxObjects++;
					iproc->state = UAVLINK_STATE_COMPLETE;
				}
			}
			break;
		default:
			connection->stats.rxErrors++;
			iproc->state = UAVLINK_STATE_ERROR;
	}
	// Done
	return iproc->state;
}



/**
 * Process an byte from the telemetry stream.
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] rxbyte Received byte
 * \return UAVLinkRxState
 */
UAVLinkRxState UAVLinkProcessInputStream(UAVLinkConnection connectionHandle, uint8_t rxbyte)
{
	UAVLinkRxState state = UAVLinkProcessInputStreamQuiet(connectionHandle, rxbyte);

	if (state == UAVLINK_STATE_COMPLETE)
	{
		UAVLinkConnectionData *connection;
		CHECKCONHANDLE(connectionHandle,connection,return -1);
		UAVLinkInputProcessor *iproc = &connection->iproc;

		//xSemaphoreTakeRecursive(connection->lock, portMAX_DELAY);
		receivePacket(connection, iproc->type, iproc->rxId, iproc->instId, connection->rxBuffer, iproc->length);
		//xSemaphoreGiveRecursive(connection->lock);
	}

	return state;
}

/**
 * Send a ACK through the telemetry link.
 * \param[in] connectionHandle UAVLinkConnection to be used
 * \param[in] objId Object ID to send a NACK for
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVLinkSendAck(UAVLinkConnection connectionHandle, uint32_t objId)
{
	UAVLinkConnectionData *connection;
	CHECKCONHANDLE(connectionHandle,connection,return -1);

	return sendAck(connection, objId);
}



/**
 * Send a NACK through the telemetry link.
 * \param[in] connectionHandle UAVLinkConnection to be used
 * \param[in] objId Object ID to send a NACK for
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVLinkSendNack(UAVLinkConnection connectionHandle, uint32_t objId)
{
	UAVLinkConnectionData *connection;
	CHECKCONHANDLE(connectionHandle,connection,return -1);

	return sendNack(connection, objId);
}

/**
 * Receive an object. This function process objects received through the telemetry stream.
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] type Type of received message (UAVLINK_TYPE_OBJ, UAVLINK_TYPE_OBJ_REQ, UAVLINK_TYPE_OBJ_ACK, UAVLINK_TYPE_ACK, UAVLINK_TYPE_NACK)
 * \param[in] objId ID of the object to work on
 * \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES for all instances.
 * \param[in] data Data buffer
 * \param[in] length Buffer length
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t receivePacket(UAVLinkConnectionData *connection, uint8_t type, uint32_t objId, uint16_t instId, uint8_t* data, int32_t length)
{
	int32_t ret = 0;

	
	// Process message type
	switch (type) {
		case UAVLINK_TYPE_OBJ:
			// Check if an ack is pending
			updateAck(connection, objId);
			break;
	case UAVLINK_TYPE_OBJ_ACK:
			// Always Transmit ACK
			sendAck(connection, objId);
			break;
	case UAVLINK_TYPE_STREAM:
			// Transmit ACK
			sendAck(connection,objId);
			if (connection->streamForwarder) {
				(connection->streamForwarder)(objId,data,length);
			}
			break;
	case UAVLINK_TYPE_OBJ_REQ:
			// The in uavlink the autopilot board never requests an object from the computer (this code)
			sendNack(connection, objId);
			break;
		case UAVLINK_TYPE_NACK:
			// Do nothing
			break;
		case UAVLINK_TYPE_ACK:
			// Check if an ack is pending
			updateAck(connection, objId);
			break;
		default:
			ret = -1;
	}
	// Done
	return ret;
}

/**
 * Check if an ack is pending on an object and give response semaphore
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] obj Object
 */
static void updateAck(UAVLinkConnectionData *connection, uint32_t rxId)
{
	if (connection->respId == rxId)
	{
		//xSemaphoreGive(connection->respSema);
		connection->resp = 1;
		connection->respId = 0;
	}
}

// returns if a response was received, if one was write packet to buf and update len to the length of the packet
// uavlink connection to use
// buffer where packet is to be copied
// length of the buffer, set to length copied
bool UAVLinkGetResponsePacket(UAVLinkConnection connectionHandle, uint8_t *buf, uint16_t *len) {
	UAVLinkConnectionData *connection;
	CHECKCONHANDLE(connectionHandle,connection,return -1);
	
	if (connection->resp) {
		connection->resp = 0;
		// use the smaller of the max_len and the received size
		*len = *len < connection->iproc.rxPacketLength ? *len : connection->iproc.rxPacketLength  ;
		memcpy(buf,connection->rxPacketBuffer,*len);
		return 1;
	} else {
	  len = 0;
	  return 0;
	}
}


/**
 * Send a packet through the telemetry link.
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] objId Object handle to send
 * \param[in] instId The instance ID (can NOT be UAVOBJ_ALL_INSTANCES, use sendObject() instead)
 * \param[in] type Transaction type
 * \return 0 Success
 * \return -1 Failure
 */
int32_t UAVLinkSendPacket(UAVLinkConnection connectionHandle, uint32_t objId, uint8_t type, uint8_t *buf, uint16_t data_length)
{
	UAVLinkConnectionData *connection;
	CHECKCONHANDLE(connectionHandle,connection,return -1);
	int32_t length;
	int32_t dataOffset;

	if (!connection->outStream) return -1;

	connection->txBuffer[0] = UAVLINK_SYNC_VAL;  // sync byte
	connection->txBuffer[1] = type;
	// data length inserted here below
	connection->txBuffer[4] = (uint8_t)(objId & 0xFF);
	connection->txBuffer[5] = (uint8_t)((objId >> 8) & 0xFF);
	connection->txBuffer[6] = (uint8_t)((objId >> 16) & 0xFF);
	connection->txBuffer[7] = (uint8_t)((objId >> 24) & 0xFF);
	
	// Don't know anything about instance id here, so dont use it
	dataOffset = 8;

	
	// Determine data length
	if (type == UAVLINK_TYPE_OBJ_REQ || type == UAVLINK_TYPE_ACK)
	{
		length = 0;
	}
	else
	{
		length = data_length;
	}
	
	// Check length
	if (length >= UAVLINK_MAX_PAYLOAD_LENGTH)
	{
		return -1;
	}
	
	// Copy data (if any)
	if (length > 0)
	{
		memcpy(&connection->txBuffer[dataOffset], buf, length);
	}
	

	// Store the packet length
	connection->txBuffer[2] = (uint8_t)((dataOffset+length) & 0xFF);
	connection->txBuffer[3] = (uint8_t)(((dataOffset+length) >> 8) & 0xFF);
	
	// Calculate checksum
	connection->txBuffer[dataOffset+length] = CRC_updateCRC(0, connection->txBuffer, dataOffset+length);

	uint16_t tx_msg_len = dataOffset+length+UAVLINK_CHECKSUM_LENGTH;
	int32_t rc = (*connection->outStream)(connection->txBuffer, tx_msg_len);

	if (rc == tx_msg_len) {
		// Update stats
		++connection->stats.txObjects;
		connection->stats.txBytes += tx_msg_len;
		connection->stats.txObjectBytes += length;
	}
	
	// store the response information if a response is needed
	connection->resp =  0;
	connection->respId =  objId;


	// Done
	return 0;
}


/**
 * Send an object through the telemetry link.
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] obj Object handle to send
 * \param[in] instId The instance ID (can NOT be UAVOBJ_ALL_INSTANCES, use sendObject() instead)
 * \param[in] type Transaction type
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t sendStreamPacket(UAVLinkConnection connectionHandle, uint8_t Id, uint8_t length, uint8_t *buf)
{
	UAVLinkConnectionData *connection;
	CHECKCONHANDLE(connectionHandle,connection,return -1);

	uint8_t dataOffset;
	dataOffset = 5;

	if (!connection->outStream) return -1;

	connection->txBuffer[0] = UAVLINK_SYNC_VAL;  // sync byte
	connection->txBuffer[1] = UAVLINK_TYPE_STREAM;
	// data length inserted here below
	connection->txBuffer[4] = Id;

	// Check length
	if ((length >= UAVLINK_MAX_PAYLOAD_LENGTH) | (length == 0))
	{
		return -1;
	}

	// Copy data
	memcpy(&connection->txBuffer[dataOffset],buf,length);

	// Store the packet length
	connection->txBuffer[2] = (uint8_t)((dataOffset+length) & 0xFF);
	connection->txBuffer[3] = (uint8_t)(((dataOffset+length) >> 8) & 0xFF);

	// Calculate checksum
	connection->txBuffer[dataOffset+length] = CRC_updateCRC(0, connection->txBuffer, dataOffset+length);

	uint16_t tx_msg_len = dataOffset+length+UAVLINK_CHECKSUM_LENGTH;
	int32_t rc = (*connection->outStream)(connection->txBuffer, tx_msg_len);

	if (rc == tx_msg_len) {
		// Update stats
		++connection->stats.txStreamPackets;
		connection->stats.txBytes += tx_msg_len;
		connection->stats.txStreamBytes += length;
	}

	// Done
	return 0;
}

/**
 * Send a NACK through the telemetry link.
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] objId Object ID to send a NACK for
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t sendNack(UAVLinkConnectionData *connection, uint32_t objId)
{
	int32_t dataOffset;

	if (!connection->outStream) return -1;

	connection->txBuffer[0] = UAVLINK_SYNC_VAL;  // sync byte
	connection->txBuffer[1] = UAVLINK_TYPE_NACK;
	// data length inserted here below
	connection->txBuffer[4] = (uint8_t)(objId & 0xFF);
	connection->txBuffer[5] = (uint8_t)((objId >> 8) & 0xFF);
	connection->txBuffer[6] = (uint8_t)((objId >> 16) & 0xFF);
	connection->txBuffer[7] = (uint8_t)((objId >> 24) & 0xFF);

	dataOffset = 8;

	// Store the packet length
	connection->txBuffer[2] = (uint8_t)((dataOffset) & 0xFF);
	connection->txBuffer[3] = (uint8_t)(((dataOffset) >> 8) & 0xFF);

	// Calculate checksum
	connection->txBuffer[dataOffset] = CRC_updateCRC(0, connection->txBuffer, dataOffset);

	uint16_t tx_msg_len = dataOffset+UAVLINK_CHECKSUM_LENGTH;
	int32_t rc = (*connection->outStream)(connection->txBuffer, tx_msg_len);

	if (rc == tx_msg_len) {
		// Update stats
		connection->stats.txBytes += tx_msg_len;
	}

	// Done
	return 0;
}

/**
 * Send a ACK through the telemetry link.
 * \param[in] connection UAVLinkConnection to be used
 * \param[in] Id of the stream for ACK
 * \return 0 Success
 * \return -1 Failure
 * This is needed because the regular send ack expects to be passed an object to ack
 */
static int32_t sendAck(UAVLinkConnectionData *connection, uint32_t Id)
{
	int32_t dataOffset;

	if (!connection->outStream) return -1;

	connection->txBuffer[0] = UAVLINK_SYNC_VAL;  // sync byte
	connection->txBuffer[1] = UAVLINK_TYPE_ACK;
	// data length inserted here below
	connection->txBuffer[4] = (uint8_t)(Id & 0xFF);
	connection->txBuffer[5] = (uint8_t)((Id >> 8) & 0xFF);
	connection->txBuffer[6] = (uint8_t)((Id >> 16) & 0xFF);
	connection->txBuffer[7] = (uint8_t)((Id >> 24) & 0xFF);

	dataOffset = 8;

	// Store the packet length
	connection->txBuffer[2] = (uint8_t)((dataOffset) & 0xFF);
	connection->txBuffer[3] = (uint8_t)(((dataOffset) >> 8) & 0xFF);

	// Calculate checksum
	connection->txBuffer[dataOffset] = CRC_updateCRC(0, connection->txBuffer, dataOffset);

	uint16_t tx_msg_len = dataOffset+UAVLINK_CHECKSUM_LENGTH;
	int32_t rc = (*connection->outStream)(connection->txBuffer, tx_msg_len);

	if (rc == tx_msg_len) {
		// Update stats
		connection->stats.txBytes += tx_msg_len;
	}

	// Done
	return 0;
}

/**
 * @}
 * @}
 */
