/**
  ******************************************************************************
  * @addtogroup PIOS PIOS Core hardware abstraction layer
  * @{
  * @addtogroup PIOS_BMP180 BMP180 Functions
  * @brief Hardware functions to deal with the altitude pressure sensor
  * @{
  *
  * @file       pios_bmp180.c  
  * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
  * @brief      BMP180 Pressure Sensor Routines
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_BMP180)

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
} BMP180CalDataTypeDef;

/* Glocal Variables */
ConversionTypeTypeDef ADC_Conversion_Type;

/* Local Functions */
static uint32_t	PIOS_BMP180_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static uint32_t	PIOS_BMP180_Write(uint8_t address, uint8_t buffer);
static int32_t	PIOS_BMP180_ReadADC(ConversionTypeTypeDef ADC_Conversion_Type_type);
static uint8_t	PIOS_BMP180_ReadID(void);

/* Local Variables */
static BMP180CalDataTypeDef		CalData;
static volatile uint32_t		ADC_Start_Time;
static volatile bool			ADC_is_Running = false; // Assumed not running at startup.

/* Straight from the datasheet */
static	int32_t		X1, X2, X3, B3, B5, B6, P = 0;
static	uint32_t	B4, B7;
static	uint16_t	RawTemperature;
static	uint32_t	RawPressure;
static	uint32_t	Pressure = 0;
static	uint16_t	Temperature = 0;

/**
* @brief Initialize the BMP180 sensor, get the calibration constants
* \return Should hang on I²C failure
*/
void PIOS_BMP180_Init(void)
{
	/* Read all 22 bytes of calibration data in one transfer */
	uint8_t Data[PIOS_BMP180_CALIB_LEN];
	while (PIOS_BMP180_Read(PIOS_BMP180_CALIBRATE, Data, PIOS_BMP180_CALIB_LEN))
		continue;

	/* Parameters AC1-AC6 */
	CalData.AC1 = (Data[0] << 8) | Data[1];
	CalData.AC2 = (Data[2] << 8) | Data[3];
	CalData.AC3 = (Data[4] << 8) | Data[5];
	CalData.AC4 = (Data[6] << 8) | Data[7];
	CalData.AC5 = (Data[8] << 8) | Data[9];
	CalData.AC6 = (Data[10] << 8) | Data[11];

	/* Parameters B1, B2 */
	CalData.B1 = (Data[12] << 8) | Data[13];
	CalData.B2 = (Data[14] << 8) | Data[15];

	/* Parameters MB, MC, MD */
	CalData.MB = (Data[16] << 8) | Data[17];
	CalData.MC = (Data[18] << 8) | Data[19];
	CalData.MD = (Data[20] << 8) | Data[21];
}

/**
* Start the ADC conversion
* \param[in] PIOS_BMP180_PRES_START_ADC or PIOS_BMP180_TEMP_START_ADC
* \return  0 for success
* \return -1 for I²C failure
*/
int32_t PIOS_BMP180_StartADC(ConversionTypeTypeDef ADC_Conversion_Type)
{
	/* Start the ADC conversion */
	if (ADC_Conversion_Type == TEMPERATURE) {
		if (PIOS_BMP180_Write(PIOS_BMP180_CTRL, PIOS_BMP180_TEMP_START_ADC) != 0 )
			return -1;
			
	} else if (ADC_Conversion_Type == PRESSURE) {
		if (PIOS_BMP180_Write(PIOS_BMP180_CTRL, PIOS_BMP180_PRES_START_ADC) != 0 )
			return -1;
	}
	
	ADC_Start_Time = PIOS_DELAY_GetuS();
	ADC_is_Running = true;
	return 0;
}

/**
* Read the ADC conversion value (once ADC conversion has completed)
* \param[in] PIOS_BMP180_PRES_START_ADC or PIOS_BMP180_TEMP_START_ADC
* \return Raw ADC value
*/
static int32_t PIOS_BMP180_ReadADC(ConversionTypeTypeDef ADC_Conversion_Type)
{
	uint8_t Data[3] = {0,0,0};
	
	if (ADC_Conversion_Type == TEMPERATURE) {
		while (PIOS_BMP180_Read(PIOS_BMP180_ADC_MSB, Data, 2))
			continue;
		
		RawTemperature = (Data[0] << 8) | Data[1];

		X1 = (RawTemperature - CalData.AC6) * CalData.AC5 >> 15;
		X2 = ((int32_t) CalData.MC << 11) / (X1 + CalData.MD);
		B5 = X1 + X2;
		
		Temperature = (B5 + 8) >> 4;
		return Temperature;
		
	} else { /* if (ADC_Conversion_Type == PRESSURE) */
		while (PIOS_BMP180_Read(PIOS_BMP180_ADC_MSB, Data, 3))
			continue;
		
		RawPressure = ((Data[0] << 16) | (Data[1] << 8) | Data[2]) >> (8 - PIOS_BMP180_OVERSAMPLING);

		B6 = B5 - 4000;
		X1 = (CalData.B2 * (B6 * B6 >> 12)) >> 11;
		X2 = CalData.AC2 * B6 >> 11;
		X3 = X1 + X2;
		B3 = ((((int32_t) CalData.AC1 * 4 + X3) << PIOS_BMP180_OVERSAMPLING) + 2) >> 2;
		X1 = CalData.AC3 * B6 >> 13;
		X2 = (CalData.B1 * (B6 * B6 >> 12)) >> 16;
		X3 = ((X1 + X2) + 2) >> 2;
		B4 = (CalData.AC4 * (uint32_t) (X3 + 32768)) >> 15;
		B7 = ((uint32_t) RawPressure - B3) * (50000 >> PIOS_BMP180_OVERSAMPLING);
		P = B7 < 0x80000000 ? (B7 * 2) / B4 : (B7 / B4) * 2;

		X1 = (P >> 8) * (P >> 8);
		X1 = (X1 * 3038) >> 16;
		X2 = (-7357 * P) >> 16;
		
		Pressure = P + ((X1 + X2 + 3791) >> 4);
		return Pressure;
	}
}

int16_t PIOS_BMP180_GetTemperature(void)
{
	return PIOS_BMP180_ReadADC(TEMPERATURE);
}

int32_t PIOS_BMP180_GetPressure(void)
{
	return PIOS_BMP180_ReadADC(PRESSURE);
}

/**
 * @brief Returns the time until a valid reading is expected to be available
 * \return 0 for data available now
 * \return time in us for a positive time assuming a result is expected
 * \return PIOS_Assert() if no ADC start was triggered and no result expected
*/
int32_t PIOS_BMP180_Data_Ready_Time_us(void)
{
	uint32_t waiting_time;			// In ms
	int32_t return_value;
	
	PIOS_Assert(ADC_is_Running);	// Because you shouldn't have asked.

	if (ADC_Conversion_Type == TEMPERATURE)
		waiting_time = PIOS_BMP180_TEMP_WAIT_us;
	
	else
		waiting_time = PIOS_BMP180_PRESSURE_WAITING_TIME;
	
	if (PIOS_DELAY_GetuSSince(ADC_Start_Time) < waiting_time) {
		return_value = (waiting_time - PIOS_DELAY_GetuSSince(ADC_Start_Time));
		return return_value;
	}
	else {
		ADC_is_Running = false;
		return 0;
	};
}

/**
 * @brief Read the identification byte from the BMP180 sensor
 * \param[out] int32_t to store BMP180 ID.
 * \return id if successful
 * \return -1 if not successful (Don't worry, the BMP180's id is not "-1" (0xFF))
 */
static uint8_t PIOS_BMP180_ReadID(void)
{
	uint8_t id = 0xFF;

	if (PIOS_BMP180_Read(PIOS_BMP180_WHO_AM_I, &id, 1) != 0);
		return -1;

	return id;
}

/**
* @brief Run self-test operation. See if the part is alive and the ID matches the datasheet.
* \return  0 if passed
* \return -1 if fail
*/
int32_t PIOS_BMP180_Test(void)
{
	if( PIOS_BMP180_ReadID() != PIOS_BMP180_ID )
		return -1;
	return 0;
}

/**
* Reads one or more bytes into a buffer
* \param[in] address BMP180 register address (depends on size)
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read
* \return  0 if operation was successful
* \return -1 if error during I²C transfer
* \return -2 if BMP180 blocked by another task (retry it!)
* \return -4 if invalid length
*/
static uint32_t PIOS_BMP180_Read(uint8_t address, uint8_t * buffer, uint8_t len)
{
	uint8_t addr_buffer[] = {
		address,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_BMP180_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(addr_buffer),
		 .buf = addr_buffer,
		 }
		,
		{
		 .info = __func__,
		 .addr = PIOS_BMP180_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = len,
		 .buf = buffer,
		 }
	};

	return PIOS_I2C_Transfer(PIOS_I2C_BMP180_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
* Writes one or more bytes to the BMP180
* \param[in] address Register address
* \param[in] buffer source buffer
* \return  0 if operation was successful
* \return -1 if error during I²C transfer
* \return -2 if BMP180 blocked by another task (retry it!)
*/
static uint32_t PIOS_BMP180_Write(uint8_t address, uint8_t buffer)
{
	uint8_t data[] = {
		address,
		buffer,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_BMP180_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(data),
		 .buf = data,
		 }
		,
	};

	return PIOS_I2C_Transfer(PIOS_I2C_BMP180_ADAPTER, txn_list, NELEMENTS(txn_list));
}

#endif /* PIOS_INCLUDE_BMP180 */
