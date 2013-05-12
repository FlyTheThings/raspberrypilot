/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup Sensors
 * @brief Acquires sensor data 
 * Specifically updates the the @ref Gyros, @ref Accels, and @ref Magnetometer objects
 * @{
 *
 * @file       sensors.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Module to handle all comms to the AHRS on a periodic basis.
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
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
 * Input objects: None, takes sensor data via pios
 * Output objects: @ref Gyros @ref Accels @ref Magnetometer
 *
 * The module executes in its own thread.
 *
 * UAVObjects are automatically generated by the UAVObjectGenerator from
 * the object definition XML file.
 *
 * Modules have no API, all communication to other modules is done through UAVObjects.
 * However modules may use the API exposed by shared libraries.
 * See the OpenPilot wiki for more details.
 * http://www.openpilot.org/OpenPilot_Application_Architecture
 *
 */
 

/*
 * Sensors on the raspberry pilot board are not mounted in alignement
 *
 * Board 	x	y	z	(x toward GPS, y toward airspeed, z down through board)
 * LM303	-x	y	-z
 * LSM330	-x	y	-z
 * mag3110	y	x	z
 * hmc5883	-x	y	-z
 *
 */
#include "openpilot.h"
#include "homelocation.h"
#include "magnetometer.h"
#include "magbias.h"
#include "accels.h"
#include "gyros.h"
#include "gyrosbias.h"
#include "attitudeactual.h"
#include "attitudesettings.h"
#include "revocalibration.h"
#include "flightstatus.h"
#include "CoordinateConversions.h"

#include <pios_board_info.h>

// Private constants
#define STACK_SIZE_BYTES 1240
#define TASK_PRIORITY (tskIDLE_PRIORITY+3)
#define SENSOR_PERIOD 3

#define F_PI 3.14159265358979323846f
#define PI_MOD(x) (fmodf(x + F_PI, F_PI * 2) - F_PI)
// Private types


// Private functions
static void SensorsTask(void *parameters);
static void settingsUpdatedCb(UAVObjEvent * objEv);
static void magOffsetEstimation(MagnetometerData *mag);

// Private variables
static xTaskHandle sensorsTaskHandle;
RevoCalibrationData cal;

// These values are initialized by settings but can be updated by the attitude algorithm
static bool bias_correct_gyro = true;

static float mag_bias[3] = {0,0,0};
static float mag_scale[3] = {0,0,0};
static float accel_bias[3] = {0,0,0};
static float accel_scale[3] = {0,0,0};

static float R[3][3] = {{0}};
static int8_t rotate = 0;

/**
 * API for sensor fusion algorithms:
 * Configure(xQueueHandle gyro, xQueueHandle accel, xQueueHandle mag, xQueueHandle baro)
 *   Stores all the queues the algorithm will pull data from
 * FinalizeSensors() -- before saving the sensors modifies them based on internal state (gyro bias)
 * Update() -- queries queues and updates the attitude estiamte
 */


/**
 * Initialise the module.  Called before the start function
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t SensorsInitialize(void)
{
	GyrosInitialize();
	GyrosBiasInitialize();
	AccelsInitialize();
	MagnetometerInitialize();
	MagBiasInitialize();
	RevoCalibrationInitialize();
	AttitudeSettingsInitialize();

	rotate = 0;

	RevoCalibrationConnectCallback(&settingsUpdatedCb);
	AttitudeSettingsConnectCallback(&settingsUpdatedCb);

	return 0;
}

/**
 * Start the task.  Expects all objects to be initialized by this point.
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t SensorsStart(void)
{
	// Start main task
	xTaskCreate(SensorsTask, (signed char *)"Sensors", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &sensorsTaskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_SENSORS, sensorsTaskHandle);
	PIOS_WDG_RegisterFlag(PIOS_WDG_SENSORS);

	return 0;
}

MODULE_INITCALL(SensorsInitialize, SensorsStart)

int32_t accel_test;
int32_t gyro_test;
int32_t mag_test1, mag_test2;
int32_t altitude_test;
//int32_t pressure_test;
//int32_t airspeed_test;


/**
 * The sensor task.  This polls the gyros at 333 Hz and pumps that data to
 * stabilization and to the attitude loop
 * 
 */

uint32_t sensor_dt_us;
static void SensorsTask(void *parameters)
{
	portTickType lastSysTime;
	float gyro_scaling = 1;
	float accel_scaling = 1;
	static int32_t timeval;

	AlarmsClear(SYSTEMALARMS_ALARM_SENSORS);

	UAVObjEvent ev;
	settingsUpdatedCb(&ev);



	//should really do the tests sometime
	gyro_test = 0;
	accel_test = 0;
	
#if defined(PIOS_INCLUDE_HMC5883)
	mag_test1 = PIOS_HMC5883_Test();
#else
	mag_test1 = 0;
#endif

#if defined(PIOS_INCLUDE_MAG3110)
	mag_test2 = PIOS_MAG3110_Test();
#else
	mag_test2 = 0;
#endif

#if defined(PIOS_INCLUDE_BMP180)
	altitude_test = PIOS_BMP180_Test();
#else
	altitude_test = 0;
#endif

#if defined(PIOS_INCLUDE_BMP085)
	altitude_test = PIOS_BMP085_Test();
#else
	altitude_test = 0;
#endif


	if(accel_test < 0 || gyro_test < 0 || mag_test1 < 0 || altitude_test < 0 || mag_test2 < 0) {
		AlarmsSet(SYSTEMALARMS_ALARM_SENSORS, SYSTEMALARMS_ALARM_CRITICAL);
		while(1) {
			PIOS_WDG_UpdateFlag(PIOS_WDG_SENSORS);
			vTaskDelay(10);
		}
	}
	
	// Main task loop
	lastSysTime = xTaskGetTickCount();
	bool error = false;
	uint32_t mag_update_time = PIOS_DELAY_GetRaw();
	while (1) {
		// TODO: add timeouts to the sensor reads and set an error if the fail
		sensor_dt_us = PIOS_DELAY_DiffuS(timeval);
		timeval = PIOS_DELAY_GetRaw();

		vTaskDelayUntil(&lastSysTime, SENSOR_PERIOD / portTICK_RATE_MS);
		if (error) {
			PIOS_WDG_UpdateFlag(PIOS_WDG_SENSORS);
			lastSysTime = xTaskGetTickCount();

			AlarmsSet(SYSTEMALARMS_ALARM_SENSORS, SYSTEMALARMS_ALARM_CRITICAL);
			error = false;
		} else {
			AlarmsClear(SYSTEMALARMS_ALARM_SENSORS);
		}



		AccelsData accelsData;
		GyrosData gyrosData;

		static float accels[3];
		PIOS_LSM303_read_accel( (float *) accels);
		accels[0] = -accels[0];
		accels[1] =  accels[1];
		accels[2] = -accels[2];
						
		static float gyros[3];
		float temp;
		PIOS_LSM330_read_gyro( (float *) gyros);
		temp = gyros[0];
		gyros[0] = gyros[1];
		gyros[1] = temp;
		gyros[2] = -gyros[2];
		// Scale the accels
		float accels_out[3] = {accels[0] * accel_scaling * accel_scale[0] - accel_bias[0],
		                       accels[1] * accel_scaling * accel_scale[1] - accel_bias[1],
		                       accels[2] * accel_scaling * accel_scale[2] - accel_bias[2]};
		if (rotate) {
			rot_mult(R, accels_out, accels);
			accelsData.x = accels[0];
			accelsData.y = accels[1];
			accelsData.z = accels[2];
		} else {
			accelsData.x = accels_out[0];
			accelsData.y = accels_out[1];
			accelsData.z = accels_out[2];
		}
		AccelsSet(&accelsData);

		// Scale the gyros
		float gyros_out[3] = {gyros[0] * gyro_scaling,
		                      gyros[1] * gyro_scaling,
		                      gyros[2] * gyro_scaling};
		if (rotate) {
			rot_mult(R, gyros_out, gyros);
			gyrosData.x = gyros[0];
			gyrosData.y = gyros[1];
			gyrosData.z = gyros[2];
		} else {
			gyrosData.x = gyros_out[0];
			gyrosData.y = gyros_out[1];
			gyrosData.z = gyros_out[2];
		}
		
		if (bias_correct_gyro) {
			// Apply bias correction to the gyros from the state estimator
			GyrosBiasData gyrosBias;
			GyrosBiasGet(&gyrosBias);
			gyrosData.x -= gyrosBias.x;
			gyrosData.y -= gyrosBias.y;
			gyrosData.z -= gyrosBias.z;
		}

		GyrosSet(&gyrosData);
		

#if defined(PIOS_INCLUDE_HMC5883)
		MagnetometerData mag;
		if (PIOS_HMC5883_NewDataAvailable() || PIOS_DELAY_DiffuS(mag_update_time) > 150000) {
			int16_t mag_values[3];
			PIOS_HMC5883_ReadMag(mag_values);
			float mag_temp;
			mag_temp = mag_values[0];
			mag_values[0] =  mag_values[1];
			mag_values[1] =  mag_temp;
			mag_values[2] = -mag_values[2];
			float mags[3] = {(float) mag_values[0] * mag_scale[0] - mag_bias[0],
			                (float) mag_values[1] * mag_scale[1] - mag_bias[1],
			                (float) mag_values[2] * mag_scale[2] - mag_bias[2]};

			if (rotate) {
				float mag_out[3];
				rot_mult(R, mags, mag_out);
				mag.x = mag_out[0];
				mag.y = mag_out[1];
				mag.z = mag_out[2];
			} else {
				mag.x = mags[0];
				mag.y = mags[1];
				mag.z = mags[2];
			}
			
			// Correct for mag bias and update if the rate is non zero
			if(cal.MagBiasNullingRate > 0)
				magOffsetEstimation(&mag);

			MagnetometerSet(&mag);
			mag_update_time = PIOS_DELAY_GetRaw();
		}
#endif



		PIOS_WDG_UpdateFlag(PIOS_WDG_SENSORS);

		lastSysTime = xTaskGetTickCount();
	}
}

/**
 * Perform an update of the @ref MagBias based on
 * Magnetometer Offset Cancellation: Theory and Implementation, 
 * revisited William Premerlani, October 14, 2011
 */
static void magOffsetEstimation(MagnetometerData *mag)
{
#if 0
	// Constants, to possibly go into a UAVO
	static const float MIN_NORM_DIFFERENCE = 50;

	static float B2[3] = {0, 0, 0};

	MagBiasData magBias;
	MagBiasGet(&magBias);

	// Remove the current estimate of the bias
	mag->x -= magBias.x;
	mag->y -= magBias.y;
	mag->z -= magBias.z;

	// First call
	if (B2[0] == 0 && B2[1] == 0 && B2[2] == 0) {
		B2[0] = mag->x;
		B2[1] = mag->y;
		B2[2] = mag->z;
		return;
	}

	float B1[3] = {mag->x, mag->y, mag->z};
	float norm_diff = sqrtf(powf(B2[0] - B1[0],2) + powf(B2[1] - B1[1],2) + powf(B2[2] - B1[2],2));
	if (norm_diff > MIN_NORM_DIFFERENCE) {
		float norm_b1 = sqrtf(B1[0]*B1[0] + B1[1]*B1[1] + B1[2]*B1[2]);
		float norm_b2 = sqrtf(B2[0]*B2[0] + B2[1]*B2[1] + B2[2]*B2[2]);
		float scale = cal.MagBiasNullingRate * (norm_b2 - norm_b1) / norm_diff;
		float b_error[3] = {(B2[0] - B1[0]) * scale, (B2[1] - B1[1]) * scale, (B2[2] - B1[2]) * scale};

		magBias.x += b_error[0];
		magBias.y += b_error[1];
		magBias.z += b_error[2];

		MagBiasSet(&magBias);

		// Store this value to compare against next update
		B2[0] = B1[0]; B2[1] = B1[1]; B2[2] = B1[2];
	}
#else
	MagBiasData magBias;
	MagBiasGet(&magBias);
	
	// Remove the current estimate of the bias
	mag->x -= magBias.x;
	mag->y -= magBias.y;
	mag->z -= magBias.z;
	
	HomeLocationData homeLocation;
	HomeLocationGet(&homeLocation);
	
	AttitudeActualData attitude;
	AttitudeActualGet(&attitude);
	
	const float Rxy = sqrtf(homeLocation.Be[0]*homeLocation.Be[0] + homeLocation.Be[1]*homeLocation.Be[1]);
	const float Rz = homeLocation.Be[2];
	
	const float rate = cal.MagBiasNullingRate;
	float R[3][3];
	float B_e[3];
	float xy[2];
	float delta[3];
	
	// Get the rotation matrix
	Quaternion2R(&attitude.q1, R);
	
	// Rotate the mag into the NED frame
	B_e[0] = R[0][0] * mag->x + R[1][0] * mag->y + R[2][0] * mag->z;
	B_e[1] = R[0][1] * mag->x + R[1][1] * mag->y + R[2][1] * mag->z;
	B_e[2] = R[0][2] * mag->x + R[1][2] * mag->y + R[2][2] * mag->z;
	
	float cy = cosf(attitude.Yaw * M_PI / 180.0f);
	float sy = sinf(attitude.Yaw * M_PI / 180.0f);
	
	xy[0] =  cy * B_e[0] + sy * B_e[1];
	xy[1] = -sy * B_e[0] + cy * B_e[1];
	
	float xy_norm = sqrtf(xy[0]*xy[0] + xy[1]*xy[1]);
	
	delta[0] = -rate * (xy[0] / xy_norm * Rxy - xy[0]);
	delta[1] = -rate * (xy[1] / xy_norm * Rxy - xy[1]);
	delta[2] = -rate * (Rz - B_e[2]);
	
	if (delta[0] == delta[0] && delta[1] == delta[1] && delta[2] == delta[2]) {		
		magBias.x += delta[0];
		magBias.y += delta[1];
		magBias.z += delta[2];
		MagBiasSet(&magBias);
	}
#endif
}

/**
 * Locally cache some variables from the AtttitudeSettings object
 */
static void settingsUpdatedCb(UAVObjEvent * objEv) {
	RevoCalibrationGet(&cal);
	
	mag_bias[0] = cal.mag_bias[REVOCALIBRATION_MAG_BIAS_X];
	mag_bias[1] = cal.mag_bias[REVOCALIBRATION_MAG_BIAS_Y];
	mag_bias[2] = cal.mag_bias[REVOCALIBRATION_MAG_BIAS_Z];
	mag_scale[0] = cal.mag_scale[REVOCALIBRATION_MAG_SCALE_X];
	mag_scale[1] = cal.mag_scale[REVOCALIBRATION_MAG_SCALE_Y];
	mag_scale[2] = cal.mag_scale[REVOCALIBRATION_MAG_SCALE_Z];
	accel_bias[0] = cal.accel_bias[REVOCALIBRATION_ACCEL_BIAS_X];
	accel_bias[1] = cal.accel_bias[REVOCALIBRATION_ACCEL_BIAS_Y];
	accel_bias[2] = cal.accel_bias[REVOCALIBRATION_ACCEL_BIAS_Z];
	accel_scale[0] = cal.accel_scale[REVOCALIBRATION_ACCEL_SCALE_X];
	accel_scale[1] = cal.accel_scale[REVOCALIBRATION_ACCEL_SCALE_Y];
	accel_scale[2] = cal.accel_scale[REVOCALIBRATION_ACCEL_SCALE_Z];
	// Do not store gyros_bias here as that comes from the state estimator and should be
	// used from GyroBias directly
	
	// Zero out any adaptive tracking
	MagBiasData magBias;
	MagBiasGet(&magBias);
	magBias.x = 0;
	magBias.y = 0;
	magBias.z = 0;
	MagBiasSet(&magBias);
	

	AttitudeSettingsData attitudeSettings;
	AttitudeSettingsGet(&attitudeSettings);
	bias_correct_gyro = (cal.BiasCorrectedRaw == REVOCALIBRATION_BIASCORRECTEDRAW_TRUE);

	// Indicates not to expend cycles on rotation
	if(attitudeSettings.BoardRotation[0] == 0 && attitudeSettings.BoardRotation[1] == 0 &&
	   attitudeSettings.BoardRotation[2] == 0) {
		rotate = 0;
	} else {
		float rotationQuat[4];
		const float rpy[3] = {attitudeSettings.BoardRotation[ATTITUDESETTINGS_BOARDROTATION_ROLL],
			attitudeSettings.BoardRotation[ATTITUDESETTINGS_BOARDROTATION_PITCH],
			attitudeSettings.BoardRotation[ATTITUDESETTINGS_BOARDROTATION_YAW]};
		RPY2Quaternion(rpy, rotationQuat);
		Quaternion2R(rotationQuat, R);
		rotate = 1;
	}

}
/**
  * @}
  * @}
  */
