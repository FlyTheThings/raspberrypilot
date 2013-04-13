/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       uavlink.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Private include file of the UAVLink library
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

#ifndef UAVLINK_PRIV_H
#define UAVLINK_PRIV_H

//#include "uavobjectsinit.h"
#include <stdint.h>
#include <stdbool.h>
#include <crc.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Private types and constants
typedef struct {
	uint8_t  sync;
	uint8_t	 type;
	uint16_t size;
	uint32_t objId;
} uavlink_min_header;
#define UAVLINK_MIN_PACKET_SIZE       5

typedef struct {
	uint8_t  sync;
	uint8_t	 type;
	uint16_t size;
	uint32_t objId;
	uint16_t instId;
} uavlink_max_header;
#define UAVLINK_MAX_HEADER_LENGTH       sizeof(uavlink_max_header)

typedef uint8_t uavlink_checksum;
#define UAVLINK_CHECKSUM_LENGTH	        sizeof(uavlink_checksum)
#define UAVLINK_MAX_PAYLOAD_LENGTH      255
#define UAVLINK_MIN_PACKET_LENGTH	UAVLINK_MAX_HEADER_LENGTH + UAVLINK_CHECKSUM_LENGTH
#define UAVLINK_MAX_PACKET_LENGTH       UAVLINK_MIN_PACKET_LENGTH + UAVLINK_MAX_PAYLOAD_LENGTH

typedef struct {
    uint32_t objId;
    uint8_t type;
    uint16_t packet_size;
    uint32_t rxId;
    uint16_t instId;
    uint32_t length;
    uint8_t instanceLength;
    uint8_t cs;
    int32_t rxCount;
    UAVLinkRxState state;
    uint16_t rxPacketLength;
} UAVLinkInputProcessor;

typedef struct {
    uint8_t canari;
    UAVLinkOutputStream outStream;
    //xSemaphoreHandle lock;
    //xSemaphoreHandle transLock;
    //xSemaphoreHandle respSema;
    uint32_t resp;
    uint32_t respId;
    UAVLinkStats stats;
    UAVLinkInputProcessor iproc;
    uint8_t *rxBuffer;
    uint32_t txSize;
    uint8_t *txBuffer;
    uint8_t *rxPacketBuffer;
    uavLinkStreamForwarder streamForwarder;
} UAVLinkConnectionData;

#define UAVLINK_CANARI         0xCA
#define UAVLINK_WAITFOREVER     -1
#define UAVLINK_NOWAIT          0
#define UAVLINK_SYNC_VAL       0x3E
#define UAVLINK_TYPE_MASK      0xF8
#define UAVLINK_TYPE_VER       0x20
#define UAVLINK_TYPE_OBJ       (UAVLINK_TYPE_VER | 0x00)
#define UAVLINK_TYPE_OBJ_REQ   (UAVLINK_TYPE_VER | 0x01)
#define UAVLINK_TYPE_OBJ_ACK   (UAVLINK_TYPE_VER | 0x02)
#define UAVLINK_TYPE_ACK       (UAVLINK_TYPE_VER | 0x03)
#define UAVLINK_TYPE_NACK      (UAVLINK_TYPE_VER | 0x04)
#define UAVLINK_TYPE_STREAM    (UAVLINK_TYPE_VER | 0x05)

//macros
#define CHECKCONHANDLE(handle,variable,failcommand) \
	variable = (UAVLinkConnectionData*) handle; \
	if (variable == NULL || variable->canari != UAVLINK_CANARI) { \
		failcommand; \
	}

#endif // UAVLINK__PRIV_H
/**
 * @}
 * @}
 */
