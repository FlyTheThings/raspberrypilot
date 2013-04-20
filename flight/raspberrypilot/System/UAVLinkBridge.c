/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup UavlinkbridgeModule Uavlinkbridge Module
 * @brief Main uavlinkbridge module
 * Starts three tasks (RX, TX, and priority TX) that watch event queues
 * and handle all the uavlinkbridge of the UAVobjects
 * @{ 
 *
 * @file       uavlinkbridge.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Uavlinkbridge module, handles uavlinkbridge and UAVObject updates
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

#include "openpilot.h"
#include "UAVLinkBridge.h"
#include "flightuavlinkstats.h"
#include "compuavlinkstats.h"
#include "hwsettings.h"

// Private constants
#define MAX_QUEUE_SIZE   TELEM_QUEUE_SIZE
#define STACK_SIZE_BYTES PIOS_TELEM_STACK_SIZE
#define TASK_PRIORITY_RX (tskIDLE_PRIORITY + 2)
#define TASK_PRIORITY_TX (tskIDLE_PRIORITY + 2)
#define TASK_PRIORITY_TXPRI (tskIDLE_PRIORITY + 2)
#define REQ_TIMEOUT_MS 250
#define MAX_RETRIES 2
#define STATS_UPDATE_PERIOD_MS 4000
#define CONNECTION_TIMEOUT_MS 8000
#define BRIDGE_TELEM_TX_BUF_LEN 300
// Private types

// Private variables
static uint32_t uavlinkbridgePort;
static uint8_t * bridge_telem_tx_buffer;

static xQueueHandle priorityQueue;
static xTaskHandle uavlinkbridgeTxPriTaskHandle;
static void uavlinkbridgeTxPriTask(void *parameters);


static xTaskHandle uavlinkbridgeTelemTxTaskHandle;
static xTaskHandle uavlinkbridgeRxTaskHandle;
static uint32_t txErrors;
static uint32_t txRetries;
static uint32_t timeOfLastObjectUpdate;
static UAVLinkConnection uavLinkCon;

// Private functions
static void uavlinkbridgeTelemTxTask(void *parameters);
static void uavlinkbridgeRxTask(void *parameters);
static int32_t transmitData(uint8_t * data, int32_t length);
//static int32_t setUpdatePeriod(UAVObjHandle obj, int32_t updatePeriodMs);
static void processObjEvent(UAVObjEvent * ev);
static void updateUavlinkStats();
static void compUavlinkBridgeStatsUpdated();
static void forwardStream(uint32_t id, uint8_t * buf, uint16_t buf_len);



/**
 * Initialise the uavlinkbridge module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t UavlinkbridgeStart(void)
{
	// Update uavlinkbridge settings
	uavlinkbridgePort = PIOS_COM_UAVLINK;

	// Listen to objects of interest
	CompUavlinkStatsConnectQueue(priorityQueue);
	FlightUavlinkStatsConnectQueue(priorityQueue);
    
	// Start uavlinkbridge tasks
	xTaskCreate(uavlinkbridgeTelemTxTask, (signed char *)"LinkTelTx", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY_TX, &uavlinkbridgeTelemTxTaskHandle);
	xTaskCreate(uavlinkbridgeRxTask, (signed char *)"LinkRx", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY_RX, &uavlinkbridgeRxTaskHandle);
	//TaskMonitorAdd(TASKINFO_RUNNING_TELEMETRYTX, uavlinkbridgeTxTaskHandle);
	//TaskMonitorAdd(TASKINFO_RUNNING_TELEMETRYRX, uavlinkbridgeRxTaskHandle);

	xTaskCreate(uavlinkbridgeTxPriTask, (signed char *)"LinkPriTx", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY_TXPRI, &uavlinkbridgeTxPriTaskHandle);
	//TaskMonitorAdd(TASKINFO_RUNNING_TELEMETRYTXPRI, uavlinkbridgeTxPriTaskHandle);


	return 0;
}

/**
 * Initialise the uavlinkbridge module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t UavlinkbridgeInitialize(void)
{
	FlightUavlinkStatsInitialize();
	CompUavlinkStatsInitialize();

	// initialize serial streams
	bridge_telem_tx_buffer =  pvPortMalloc(BRIDGE_TELEM_TX_BUF_LEN);
	PIOS_Assert(bridge_telem_tx_buffer);

	// Initialize vars
	timeOfLastObjectUpdate = 0;

	// Create object queue
	priorityQueue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));
    
	// Initialise UAVLink
	uavLinkCon = UAVLinkInitialize(&transmitData);

	UAVLinkSetStreamForwarder(uavLinkCon, forwardStream);
    
	// Create periodic event that will be used to update the uavlinkbridge stats
	txErrors = 0;
	txRetries = 0;
	UAVObjEvent ev;
	memset(&ev, 0, sizeof(UAVObjEvent));
	EventPeriodicQueueCreate(&ev, priorityQueue, STATS_UPDATE_PERIOD_MS);

	UavlinkbridgeStart();
	return 0;
}


/* UavlinkBridge is structured after the telemetry module however it does not start as a module, it starts
 * in pios_board_init
 */
//MODULE_INITCALL(UavlinkbridgeInitialize, UavlinkbridgeStart)


/**
 * Processes queue events
*/
static void processObjEvent(UAVObjEvent * ev)
{
	UAVObjMetadata metadata;
	UAVObjUpdateMode updateMode;
	FlightUavlinkStatsData flightStats;
	int32_t retries;
	int32_t success;

	if (ev->obj == 0) {
		updateUavlinkStats();
	} else if (ev->obj == CompUavlinkStatsHandle()) {
		compUavlinkBridgeStatsUpdated();
	} else {
		// Only process event if connected to computer or if object FlightUavlinkStats is updated
		FlightUavlinkStatsGet(&flightStats);
		// Get object metadata
		UAVObjGetMetadata(ev->obj, &metadata);
		updateMode = UAVObjGetTelemetryUpdateMode(&metadata);
		if (flightStats.Status == FLIGHTUAVLINKSTATS_STATUS_CONNECTED || ev->obj == FlightUavlinkStatsHandle()) {
			// Act on event
			retries = 0;
			success = -1;
			if (ev->event == EV_UPDATED || ev->event == EV_UPDATED_MANUAL || ((ev->event == EV_UPDATED_PERIODIC) && (updateMode != UPDATEMODE_THROTTLED))) {
				// Send update to computer (with retries)
				while (retries < MAX_RETRIES && success == -1) {
					//
					success = UAVLinkSendObject(uavLinkCon, ev->obj, ev->instId, UAVObjGetTelemetryAcked(&metadata), REQ_TIMEOUT_MS);	// call blocks until ack is received or timeout
					++retries;
				}
				// Update stats
				txRetries += (retries - 1);
				if (success == -1) {
					++txErrors;
				}
			}
		}

	}
}


/**
 * uavlinkbridgeTelemTxTask forwards telemetry transmissions out over uavlink
 */
static void uavlinkbridgeTelemTxTask(void *parameters)
{
	//this task handles serial forwarding only
	while (1) {
		uint32_t rx_bytes;
		rx_bytes = PIOS_COM_ReceiveBuffer(PIOS_COM_TELEM_LOOP, bridge_telem_tx_buffer, BRIDGE_TELEM_TX_BUF_LEN, 500);
		if (rx_bytes > 0) {
			streamStream(uavLinkCon, TELEM_STREAM_ID,  rx_bytes, bridge_telem_tx_buffer);
		}
	}
}

/**
 * Uavlinkbridge transmit task high priority
 */
static void uavlinkbridgeTxPriTask(void *parameters)
{
	UAVObjEvent ev;

	// Loop forever
	while (1) {
		// Wait for queue message
		if (xQueueReceive(priorityQueue, &ev, portMAX_DELAY) == pdTRUE) {
			// Process event
			processObjEvent(&ev);
		}
	}
}

/**
 * Uavlinkbridge Rx task. Processes queue events and periodic updates.
 */
static void uavlinkbridgeRxTask(void *parameters)
{
	uint32_t inputPort;

	// Task loop
	while (1) {
#if defined(PIOS_INCLUDE_USB)
		// Determine input port (USB takes priority over uavlinkbridge port)
		if (PIOS_USB_CheckAvailable(0) && PIOS_COM_TELEM_USB) {
			inputPort = PIOS_COM_TELEM_USB;
		} else
#endif /* PIOS_INCLUDE_USB */
		{
			inputPort = uavlinkbridgePort;
		}

		if (inputPort) {
			// Block until data are available
			uint8_t serial_data[1];
			uint16_t bytes_to_process;

			bytes_to_process = PIOS_COM_ReceiveBuffer(inputPort, serial_data, sizeof(serial_data), 500);
			if (bytes_to_process > 0) {
				for (uint8_t i = 0; i < bytes_to_process; i++) {
					UAVLinkProcessInputStream(uavLinkCon,serial_data[i]);
				}
			}
		} else {
			vTaskDelay(5);
		}
	}
}


/**
 * Forwards an encapsulated stream from uavlink to its destination
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static void forwardStream(uint32_t id, uint8_t * buf, uint16_t buf_len) {
	switch (id) {
		case(TELEM_STREAM_ID):
				PIOS_COM_SendBuffer(PIOS_COM_TELEM_LOOP, buf, buf_len);
			break;
		default:
			break;
	}

}


/**
 * Transmit data buffer to the modem or USB port.
 * \param[in] data Data buffer to send
 * \param[in] length Length of buffer
 * \return -1 on failure
 * \return number of bytes transmitted on success
 */
static int32_t transmitData(uint8_t * data, int32_t length)
{
	uint32_t outputPort;

	// Determine input port (USB takes priority over uavlinkbridge port)
#if defined(PIOS_INCLUDE_USB)
	if (PIOS_USB_CheckAvailable(0) && PIOS_COM_TELEM_USB) {
		outputPort = PIOS_COM_TELEM_USB;
	} else
#endif /* PIOS_INCLUDE_USB */
	{
		outputPort = uavlinkbridgePort;
	}

	if (outputPort) {
		return PIOS_COM_SendBuffer(outputPort, data, length);
	} else {
		return -1;
	}
}


/**
 * Called each time the computer uavlinkbridge stats object is updated.
 * Trigger a flight uavlinkbridge stats update if a connection is not
 * yet established.
 */
static void compUavlinkBridgeStatsUpdated()
{
	FlightUavlinkStatsData flightStats;
	CompUavlinkStatsData compStats;
	FlightUavlinkStatsGet(&flightStats);
	CompUavlinkStatsGet(&compStats);
	if (flightStats.Status != FLIGHTUAVLINKSTATS_STATUS_CONNECTED || compStats.Status != COMPUAVLINKSTATS_STATUS_CONNECTED) {
		updateUavlinkStats();
	}
}

/**
 * Update uavlinkbridge statistics and handle connection handshake
 */
static void updateUavlinkStats()
{
	UAVLinkStats ulinkStats;
	FlightUavlinkStatsData flightStats;
	CompUavlinkStatsData compStats;
	uint8_t forceUpdate;
	uint8_t connectionTimeout;
	uint32_t timeNow;

	// Get stats
	UAVLinkGetStats(uavLinkCon, &ulinkStats);
	UAVLinkResetStats(uavLinkCon);

	// Get object data
	FlightUavlinkStatsGet(&flightStats);
	CompUavlinkStatsGet(&compStats);

	// Update stats object
	if (flightStats.Status == FLIGHTUAVLINKSTATS_STATUS_CONNECTED) {
		flightStats.RxDataRate = (float)ulinkStats.rxBytes / ((float)STATS_UPDATE_PERIOD_MS / 1000.0);
		flightStats.TxDataRate = (float)ulinkStats.txBytes / ((float)STATS_UPDATE_PERIOD_MS / 1000.0);
		flightStats.RxFailures += ulinkStats.rxErrors;
		flightStats.TxFailures += txErrors;
		flightStats.TxRetries += txRetries;
		txErrors = 0;
		txRetries = 0;
	} else {
		flightStats.RxDataRate = 0;
		flightStats.TxDataRate = 0;
		flightStats.RxFailures = 0;
		flightStats.TxFailures = 0;
		flightStats.TxRetries = 0;
		txErrors = 0;
		txRetries = 0;
	}

	// Check for connection timeout
	timeNow = xTaskGetTickCount() * portTICK_RATE_MS;
	if (ulinkStats.rxObjects > 0) {
		timeOfLastObjectUpdate = timeNow;
	}
	if ((timeNow - timeOfLastObjectUpdate) > CONNECTION_TIMEOUT_MS) {
		connectionTimeout = 1;
	} else {
		connectionTimeout = 0;
	}

	// Update connection state
	forceUpdate = 1;
	if (flightStats.Status == FLIGHTUAVLINKSTATS_STATUS_DISCONNECTED) {
		// Wait for connection request
		if (compStats.Status == COMPUAVLINKSTATS_STATUS_HANDSHAKEREQ) {
			flightStats.Status = FLIGHTUAVLINKSTATS_STATUS_HANDSHAKEACK;
		}
	} else if (flightStats.Status == FLIGHTUAVLINKSTATS_STATUS_HANDSHAKEACK) {
		// Wait for connection
		if (compStats.Status == COMPUAVLINKSTATS_STATUS_CONNECTED) {
			flightStats.Status = FLIGHTUAVLINKSTATS_STATUS_CONNECTED;
		} else if (compStats.Status == COMPUAVLINKSTATS_STATUS_DISCONNECTED) {
			flightStats.Status = FLIGHTUAVLINKSTATS_STATUS_DISCONNECTED;
		}
	} else if (flightStats.Status == FLIGHTUAVLINKSTATS_STATUS_CONNECTED) {
		if (compStats.Status != COMPUAVLINKSTATS_STATUS_CONNECTED || connectionTimeout) {
			flightStats.Status = FLIGHTUAVLINKSTATS_STATUS_DISCONNECTED;
		} else {
			forceUpdate = 0;
		}
	} else {
		flightStats.Status = FLIGHTUAVLINKSTATS_STATUS_DISCONNECTED;
	}

	/* todo add uavlinke to system alarms
	// Update the uavlinkbridge alarm
	if (flightStats.Status == FLIGHTUAVLINKSTATS_STATUS_CONNECTED) {
		AlarmsClear(SYSTEMALARMS_ALARM_UAVLINK);
	} else {
		AlarmsSet(SYSTEMALARMS_ALARM_UAVLINK, SYSTEMALARMS_ALARM_ERROR);
	}
	*/

	// Update object
	FlightUavlinkStatsSet(&flightStats);

	// Force uavlinkbridge update if not connected
	if (forceUpdate) {
		FlightUavlinkStatsUpdated();
	}
}



/**
  * @}
  * @}
  */
