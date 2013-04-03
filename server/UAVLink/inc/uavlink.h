/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       uavlink.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file of the UAVLink library
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

#ifndef UAVLINK_H
#define UAVLINK_H

#include <stdbool.h>
#include <stdint.h>

// Public types
typedef int32_t (*UAVLinkOutputStream)(uint8_t* data, int32_t length);

typedef struct {
    uint32_t txBytes;
    uint32_t rxBytes;
    uint32_t txObjectBytes;
    uint32_t rxObjectBytes;
    uint32_t rxObjects;
    uint32_t rxStreamBytes;
    uint32_t rxStreamPackets;
    uint32_t txStreamBytes;
    uint32_t txStreamPackets;
    uint32_t txObjects;
    uint32_t txErrors;
    uint32_t rxErrors;
} UAVLinkStats;

typedef void* UAVLinkConnection;
typedef void (*uavLinkStreamForwarder)(uint32_t id, uint8_t * buf, uint16_t buf_len);

typedef enum {UAVLINK_STATE_ERROR=0, UAVLINK_STATE_SYNC, UAVLINK_STATE_TYPE, UAVLINK_STATE_SIZE, UAVLINK_STATE_OBJID, UAVLINK_STATE_STREAMID, UAVLINK_STATE_INSTID, UAVLINK_STATE_DATA, UAVLINK_STATE_CS, UAVLINK_STATE_STREAM_COMPLETE, UAVLINK_STATE_COMPLETE} UAVLinkRxState;

// Public functions
UAVLinkConnection UAVLinkInitialize(UAVLinkOutputStream outputStream);

int32_t UAVLinkSetOutputStream(UAVLinkConnection connection, UAVLinkOutputStream outputStream);
UAVLinkOutputStream UAVLinkGetOutputStream(UAVLinkConnection connection);

int32_t UAVLinkSendPacket(UAVLinkConnection connectionHandle, uint32_t objId, uint8_t type, uint8_t *buf, uint16_t data_length);
int32_t UAVLinkSendStream(UAVLinkConnection connection, uint8_t Id, uint8_t *buf, uint8_t length);

int32_t UAVLinkSendAck(UAVLinkConnection connectionHandle, uint32_t objId);
int32_t UAVLinkSendNack(UAVLinkConnection connectionHandle, uint32_t objId);

UAVLinkRxState UAVLinkProcessInputStream(UAVLinkConnection connection, uint8_t rxbyte);
bool UAVLinkGetResponse(UAVLinkConnection connectionHandle, uint8_t *buf, uint32_t max_len);

void UAVLinkGetStats(UAVLinkConnection connection, UAVLinkStats *stats);
void UAVLinkResetStats(UAVLinkConnection connection);


int32_t UAVLinkSetStreamForwarder(UAVLinkConnection connectionHandle, uavLinkStreamForwarder forwarder);

#endif // UAVLINK_H
/**
 * @}
 * @}
 */
