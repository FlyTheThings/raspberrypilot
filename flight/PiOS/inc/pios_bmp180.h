/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BMP180 BMP180 Functions
 * @brief Hardware functions to deal with the altitude pressure sensor
 * @{
 *
 * @file       pios_bmp180.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      BMP180 functions header.
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

#ifndef PIOS_BMP180_H
#define PIOS_BMP180_H

/* BMP180 Addresses */
#define PIOS_BMP180_I2C_DATASHEET_ADDR					0xEE	// Datasheet has 8 bit address
#define PIOS_BMP180_I2C_ADDR							PIOS_BMP180_I2C_DATASHEET_ADDR >> 1 // PIOS uses 7 Bit address
#define PIOS_I2C_BMP180_ADAPTER							PIOS_I2C_MAIN_ADAPTER
#define PIOS_BMP180_CALIBRATE							0xAA	// Calibration registers...
#define PIOS_BMP180_CALIB_LEN							22		// ...run from 0xAA to 0xBF
#define PIOS_BMP180_CTRL								0xF4	// Control Register
#define PIOS_BMP180_RESET								0xE0	// Reset Control Register - see PIOS_BMP180_RESTART
// Measurement			Control register value(0xF4)	Max. conversion time [ms]
// ------------------	----------------------------	-------------------------
// Temperature			0x2E							4.5
// Pressure (oss = 0)	0x34							4.5
// Pressure (oss = 1)	0x74							7.5
// Pressure (oss = 2)	0xB4							13.5
// Pressure (oss = 3)	0xF4							25.5
#define PIOS_BMP180_OVERSAMPLING						PIOS_BMP180_OSS // Over Sampling Selection, oss = [0..3]
#define PIOS_BMP180_ULTRA_LOW_POWER_MODE				0		// Over Sampling Selection, ( oss = 0 )
#define PIOS_BMP180_STANDARD_MODE						1		// Over Sampling Selection, ( oss = 1 )
#define PIOS_BMP180_HIGH_RESOLUTION_MODE				2		// Over Sampling Selection, ( oss = 2 )
#define PIOS_BMP180_ULTRA_HIGH_RESOLUTION_MODE			3		// Over Sampling Selection, ( oss = 3 )
#define PIOS_BMP180_RESTART								0xB6	// PIOS_BMP180_RESET password
#define PIOS_BMP180_PRES_START_ADC						(0x34 + (PIOS_BMP180_OVERSAMPLING << 6)) // ADC pressure start convert instruction
#define PIOS_BMP180_TEMP_START_ADC						0x2E	// ADC temperature start convert instruction
#define PIOS_BMP180_ADC_MSB								0xF6	// ADC register, LSB = 0xF7
#define PIOS_BMP180_P0									101325	// Sea Level Atmopheric Pressure (Pa)
#define PIOS_BMP180_TEMP_WAIT_us						4500	// Wait after start convert for temperature
#define PIOS_BMP180_ULTRA_LOW_POWER_PRES_WAIT_us		4500	// Wait after start convert for pressure ( oss = 0 )
#define PIOS_BMP180_STANDARD_PRES_WAIT_us				7500	// Wait after start convert for pressure ( oss = 1 )
#define PIOS_BMP180_HIGH_RESOLUTION_PRES_WAIT_us		13500	// Wait after start convert for pressure ( oss = 2 )
#define PIOS_BMP180_ULTRA_HIGH_RESOLUTION_PRES_WAIT_us	25500	// Wait after start convert for pressure ( oss = 3 )
#define PIOS_BMP180_WHO_AM_I							0xD0	// Identification register
#define PIOS_BMP180_ID									0x55	// Identification value

#if		PIOS_BMP180_OSS == PIOS_BMP180_ULTRA_LOW_POWER_MODE
		#define PIOS_BMP180_PRESSURE_WAITING_TIME		PIOS_BMP180_ULTRA_LOW_POWER_PRES_WAIT_us
	
#elif	PIOS_BMP180_OSS == PIOS_BMP180_STANDARD_MODE
		#define PIOS_BMP180_PRESSURE_WAITING_TIME		PIOS_BMP180_STANDARD_PRES_WAIT_us
	
#elif	PIOS_BMP180_OSS == PIOS_BMP180_HIGH_RESOLUTION_MODE
		#define PIOS_BMP180_PRESSURE_WAITING_TIME		PIOS_BMP180_HIGH_RESOLUTION_PRES_WAIT_us
	
#elif	PIOS_BMP180_OSS == PIOS_BMP180_ULTRA_HIGH_RESOLUTION_MODE
		#define PIOS_BMP180_PRESSURE_WAITING_TIME		PIOS_BMP180_ULTRA_HIGH_RESOLUTION_PRES_WAIT_us
#endif

/* Local Types */
typedef struct {
	int16_t		AC1;
	int16_t		AC2;
	int16_t		AC3;
	uint16_t	AC4;
	uint16_t	AC5;
	uint16_t	AC6;
	int16_t		B1;
	int16_t		B2;
	int16_t		MB;
	int16_t		MC;
	int16_t		MD;
} BMP180CalibDataTypeDef;

typedef enum {
	PRESSURE,
	TEMPERATURE
} ConversionTypeTypeDef;

/* Global Variables */

/* Public Functions */
extern void		PIOS_BMP180_Init(void);
extern int32_t	PIOS_BMP180_StartADC(ConversionTypeTypeDef Conversion_Type);
extern int16_t	PIOS_BMP180_GetTemperature(void);
extern int32_t	PIOS_BMP180_GetPressure(void);
extern int32_t	PIOS_BMP180_Test(void);
extern int32_t	PIOS_BMP180_Data_Ready_Time_us(void);

#endif /* PIOS_BMP180_H */

/** 
  * @}
  * @}
  */
