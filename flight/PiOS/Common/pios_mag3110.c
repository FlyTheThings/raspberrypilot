/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MAG3110 MAG3110 Functions
 * @brief Deals with the hardware interface to the MAGnetometers
 * @{
 * @file       pios_MAG3110.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      MAG3110 MAGnetic Sensor Functions from AHRS
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

#if defined(PIOS_INCLUDE_MAG3110)

/* Global Variables */

/* Local Types */

/* Local Variables */
volatile bool fast_read_mode = 0;
// MAG3110 Datasheet says the value defaults to 0 so this is safe to initialize to 0.

/* Local Functions */
static int32_t PIOS_MAG3110_Config(const struct pios_MAG3110_cfg * cfg);
static int32_t PIOS_MAG3110_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static int32_t PIOS_MAG3110_Write(uint8_t address, uint8_t buffer);

/**
 * @brief Initialize the MAG3110 Magnetometer sensor.
 * @return none
 */
int32_t PIOS_MAG3110_Init(const struct pios_MAG3110_cfg * cfg)
{	
	if (PIOS_MAG3110_Config(cfg) != 0)
		return -1;
	return 0
}

/**
 * @brief Configure Magnetometer
 * \return 0 for success
 * \return 1 for failure
 */
static int32_t PIOS_MAG3110_Config(const struct pios_MAG3110_cfg * cfg)
{
	uint8_t ctrlreg1;
	uint8_t ctrlreg2;
	
	// DATASHEET:
	// "Note: Except for STANDBY mode selection (Bit 0, AC),
	// the device must be in STANDBY mode to change any of
	// the fields within CTRL_REG1 (0x10)."
	// According to the datasheet the default mode is:
	// ------------------------------------------------
	// "AC | TM Description"
	// "0  |  0 ASIC is in low power standby mode."
	// ------------------------------------------------
	// This is a public function, however, so I don't think
	// we can assume it's currently in STANDBY.
	
	fast_read_mode = (cfg->fast_read == PIOS_MAG3110_FAST_READ);
	
	ctrlreg1 = (uint8_t) (PIOS_MAG3110_STANDBY);
	if (PIOS_MAG3110_Write(PIOS_MAG3110_CTRL_REG1, ctrlreg1) != 0)
		return -1;

	ctrlreg1 = (uint8_t) (cfg->data_rate | cfg->fast_read | cfg->trig_mode);
	ctrlreg2 = (uint8_t) (cfg->auto_degauss | cfg->raw_read | cfg->degauss_now);
	
	if (PIOS_MAG3110_Write(PIOS_MAG3110_CTRL_REG1, ctrlreg1) != 0)
		return -1;
	
	if (PIOS_MAG3110_Write(PIOS_MAG3110_CTRL_REG2, ctrlreg2) != 0)
		return -1;
	
	return 0;
}

/**
 * @brief DeGauss Magnetometer if it has experienced a high field. The bit is self clearing.
 * \return 0 for success
 * \return -1 for failure
 */
int32_t PIOS_MAG3110_DeGaussNow(void)
{
	uint8_t ctrlreg2;
	
	if (PIOS_MAG3110_Read(PIOS_MAG3110_CTRL_REG2, &ctrlreg2, 1) != 0)
		return -1;

	ctrlreg2 |= PIOS_MAG3110_MAG_RST;

	if (PIOS_MAG3110_Write(PIOS_MAG3110_CTRL_REG2, ctrlreg2) != 0)
		return -1;

	return 0;
}

/**
 * @brief Read X, Y, Z Magnetometer values
 * \param[out] int16_t array of size 3 to store X, Y, and Z Magnetometer readings in mGauss
 * \return 1 for new data available
 * \return 0 for no new data
 * \return -1 for failure
 */
int32_t PIOS_MAG3110_ReadMAG(int16_t out[3])
{
	uint8_t answer;
	uint8_t buffer[6];
	
	// Read Data Ready Status Register
	if (PIOS_MAG3110_Read(PIOS_MAG3110_DR_STATUS, &answer, 1) != 0)
		return -1;
	
	// Check if any of the X, Y or Z data have been updated since last read
	// If not, bail.
	if (!((answer & PIOS_MAG3110_ZYXDR) == PIOS_MAG3110_ZYXDR))
		return 0;

	if (fast_read_mode)
	{
		if (PIOS_MAG3110_Read(PIOS_MAG3110_DATAOUT_XMSB_REG, buffer, 3) != 0)
			return -1;

		out[1] = (uint16_t) buffer[0] << 8;
		out[2] = (uint16_t) buffer[1] << 8;
		out[3] = (uint16_t) buffer[2] << 8;
	}
	else
	{
		if (PIOS_MAG3110_Read(PIOS_MAG3110_DATAOUT_XMSB_REG, buffer, 6) != 0)
			return -1;

		out[1] = (uint16_t) buffer[0] << 8 + buffer[1];
		out[2] = (uint16_t) buffer[2] << 8 + buffer[3];
		out[3] = (uint16_t) buffer[3] << 8 + buffer[4];
	}

	return 1;
//	The MAG3110 datasheet gives the sensitivity as 0.10uT/LSB.
//	1Gauss = 100uT so 0.1uT = 1mGauss.
//	"magnetometer.xml" specifies the desired units as mGa so
//	no scaling at all is necessary.
}

/**
 * @brief Read the identification byte from the MAG3110 sensor
 * \param[out] int32_t to store MAG3110 ID.
 * \return id if successful
 * \return -1 if not successful (Don't worry, the MAG3110's id is not "-1" (0xFF))
 */
int32_t PIOS_MAG3110_ReadID(void)
{
	uint8_t id = 0xFF;
	
	if (PIOS_MAG3110_Read(PIOS_MAG3110_WHO_AM_I, &id, 1) != 0);
		return -1;

	return (int32_t)id;
}

/**
 * @brief Reads one or more bytes into a buffer
 * \param[in] address MAG3110 register address (depends on size)
 * \param[out] buffer destination buffer
 * \param[in] len number of bytes which should be read
 * \return 0 if operation was successful
 * \return -1 if error during I²C transfer
 * \return -2 if unable to claim I²C device
 */
static int32_t PIOS_MAG3110_Read(uint8_t address, uint8_t * buffer, uint8_t len)
{
	uint8_t addr_buffer[] = {
		address,
	};
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = PIOS_MAG3110_I2C_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(addr_buffer),
			.buf = addr_buffer,
		}
		,
		{
			.info = __func__,
			.addr = PIOS_MAG3110_I2C_ADDR,
			.rw = PIOS_I2C_TXN_READ,
			.len = len,
			.buf = buffer,
		}
	};
	
	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
 * @brief Writes one or more bytes to the MAG3110
 * \param[in] address Register address
 * \param[in] buffer source buffer
 * \return 0 if operation was successful
 * \return -1 if error during I²C transfer
 * \return -2 if unable to claim I²C device
 */
static int32_t PIOS_MAG3110_Write(uint8_t address, uint8_t buffer)
{
	uint8_t data[] = {
		address,
		buffer,
	};
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = PIOS_MAG3110_I2C_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(data),
			.buf = data,
		}
		,
	};
	;
	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
 * @brief Run self-test operation. See if the part is alive and the ID matches the datasheet.
 * \return 0 if pass
 * \return -1 if fail
 */
int32_t PIOS_MAG3110_Test(void)
{
	if( PIOS_MAG3110_ReadID() != PIOS_MAG3110_ID )
		return -1;
	return 0;
}

#endif /* PIOS_INCLUDE_MAG3110 */

/**
 * @}
 * @}
 */
