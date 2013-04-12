/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup AltitudeModule Altitude Module
 * @brief Communicate with BMP180 and update @ref BaroAltitude "BaroAltitude UAV Object"
 * @{ 
 *
 * @file       altitude.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Altitude module, handles temperature and pressure readings from BMP180
 *
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

/**
 * Output object: BaroAltitude
 *
 * This module will periodically update the value of the BaroAltitude object.
 *
 */

#include "openpilot.h"
#include "hwsettings.h"
#include "altitude.h"
#include "baroaltitude.h"	// object that will be updated by the module
#if defined(PIOS_INCLUDE_HCSR04)
#include "sonaraltitude.h"	// object that will be updated by the module
#endif

// Private constants
#define STACK_SIZE_BYTES 500
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define UPDATE_PERIOD 50

// Private types

// Private variables
static xTaskHandle taskHandle;
static int32_t raw_pressure;
static int32_t raw_temperature;

// IIR Running average interval
#define PRES_IIR_COEFF		0.05
#define TEMP_IIR_COEFF		0.25
#define SPEED_OF_SOUND		340 // m/s at sea level

static bool altitudeEnabled;

// Private functions
static void altitudeTask(void *parameters);

/**
 * Initialize the module, called on startup
 * \returns 0 on success or -1 if initialization failed
 */
int32_t AltitudeStart()
{

	if (altitudeEnabled) {
		BaroAltitudeInitialize();
#if defined(PIOS_INCLUDE_HCSR04)
		SonarAltitudeInitialze();
#endif

		// Start main task
		xTaskCreate(altitudeTask, (signed char *)"Altitude", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
		TaskMonitorAdd(TASKINFO_RUNNING_ALTITUDE, taskHandle);
		return 0;
	}
	return -1;
}

/**
 * Initialize the module, called on startup
 * \returns 0 on success or -1 if initialization failed
 */
int32_t AltitudeInitialize()
{
#ifdef MODULE_Altitude_BUILTIN
	altitudeEnabled = 1;
#else
	HwSettingsInitialize();
	uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];
	HwSettingsOptionalModulesGet(optionalModules);
	if (optionalModules[HWSETTINGS_OPTIONALMODULES_ALTITUDE] == HWSETTINGS_OPTIONALMODULES_ENABLED) {
		altitudeEnabled = 1;
	} else {
		altitudeEnabled = 0;
	}
#endif

	return 0;
}
MODULE_INITCALL(AltitudeInitialize, AltitudeStart)
/**
 * Module thread, should not return.
 */
static void altitudeTask(void *parameters)
{
	BaroAltitudeData data;
	portTickType lastSysTime;
	
#if defined(PIOS_INCLUDE_HCSR04)
	SonarAltitudeData sonardata;
	int32_t value = 0, timeout = 5;
	float coeff = 0.25, height_out = 0, height_in = 0;
	PIOS_HCSR04_Init();
	PIOS_HCSR04_Trigger();
#endif
	PIOS_BMP180_Init();
	
	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1)
	{
#if defined(PIOS_INCLUDE_HCSR04)
		// Compute the current altitude (all pressures in kPa)
		if(PIOS_HCSR04_Completed())
		{
			value = PIOS_HCSR04_Get();
			if((value > 100) && (value < 15000)) //from 3.4cm to 5.1m
			{
				height_in = value * SPEED_OF_SOUND/1000/2; // ÷2 for the return trip!
				height_out = (height_out * (1 - coeff)) + (height_in * coeff);
				sonardata.Altitude = height_out; // m/us
			}
			
			// Update the AltitudeActual UAVObject
			SonarAltitudeSet(&sonardata);
			timeout = 5;
			PIOS_HCSR04_Trigger();
		}
		if(timeout--)
		{
			//retrigger
			timeout = 5;
			PIOS_HCSR04_Trigger();
		}
#endif
		if (!PIOS_BMP180_StartADC(TEMPERATURE)) {
			vTaskDelay( ceil(PIOS_BMP180_Data_Ready_Time_us()/1000) / portTICK_RATE_MS); // vTaskDelay set to 1ms/tick
			// BMP180 raw result in 0.1°C. Convert to °C.
			raw_temperature = PIOS_BMP180_GetTemperature();
			data.Temperature = (float)(raw_temperature) / 10 * TEMP_IIR_COEFF + (data.Temperature * (1 - TEMP_IIR_COEFF));
		}
		if (!PIOS_BMP180_StartADC(PRESSURE)) {
			vTaskDelay( ceil(PIOS_BMP180_Data_Ready_Time_us()/1000) / portTICK_RATE_MS); // vTaskDelay set to 1ms/tick
			// BMP180 raw result in 0.01hPa (= Pa). Convert to KPa.
			raw_pressure = PIOS_BMP180_GetPressure();
			data.Pressure = (float)(raw_pressure) / 1000 * PRES_IIR_COEFF + data.Pressure * (1 - PRES_IIR_COEFF);

			// Compute the current altitude (all pressures in kPa)
			data.Altitude = 44330.0 * (1.0 - powf((data.Pressure / (PIOS_BMP180_P0 / 1000.0)), (1.0 / 5.255)));
		}
		// TODO SM add temperature reading to pressure to get more accurate altitude
		
		// http://en.wikipedia.org/wiki/Density_altitude
		// http://en.wikipedia.org/wiki/Atmospheric_pressure
		// http://www.engineeringtoolbox.com/air-altitude-pressure-d_462.html

		// Update the AltitudeActual UAVObject
		BaroAltitudeSet(&data);

		// Delay until it is time to read the next sample
		vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD / portTICK_RATE_MS);
	}
}

/**
 * @}
 * @}
 */
