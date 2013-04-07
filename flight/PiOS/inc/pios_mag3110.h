/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MAG3110 MAG3110 Functions
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 *
 * @file       pios_MAG3110.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      MAG3110 functions header.
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

#ifndef PIOS_MAG3110_H
#define PIOS_MAG3110_H

#include <pios.h>

//*******************************************************************************************
//*							MAG3110 Address													*
//*******************************************************************************************
#define PIOS_MAG3110_I2C_ADDR			0x0E                                // 7 Bit Address TODO - not 100% sure they meant 7 bit address in the datasheet...
#define PIOS_MAG3110_I2C_WRITE_ADDR		PIOS_MAG3110_I2C_ADDR << 1          // 8 Bit Write Address
#define PIOS_MAG3110_I2C_READ_ADDR		PIOS_MAG3110_I2C_WRITE_ADDR | 0x01  // 8 Bit Read Address
#define PIOS_MAG3110_ID					0xC4								// Self Assigned Internal ID Number
#define PIOS_I2C_MAG3110_ADAPTER		PIOS_I2C_MAIN_ADAPTER
/**
The MAG3110 automatically increments its subaddress for repeated reads or writes.
It has a "fast" read mode which skips the least significant data bytes if CTRL_REG1 bit2 is set.
*/
//*******************************************************************************************
//*							MAG3110 Sub-Addresses											*
//*******************************************************************************************
// Register							Subaddress					Next	 Next	Mode	Description
//																Addr.	(Fast)
//--------------------------------------------------------------------------------------------------------------------------
#define PIOS_MAG3110_DR_STATUS		0x00					//	0x01			R		Data ready status per axis
#define PIOS_MAG3110_OUT_X_MSB		0x01					//	0x02	(0x03)	R		data Bits [15:8] of X measurement
#define PIOS_MAG3110_OUT_X_LSB		0x02					//	0x03			R		data Bits [7:0]	 of	X measurement
#define PIOS_MAG3110_OUT_Y_MSB		0x03					//	0x04	(0x05)	R		data Bits [15:8] of Y measurement
#define PIOS_MAG3110_OUT_Y_LSB		0x04					//	0x05			R		data Bits [7:0]  of Y measurement
#define PIOS_MAG3110_OUT_Z_MSB		0x05					//	0x06	(0x07)	R		data Bits [15:8] of Z measurement
#define PIOS_MAG3110_OUT_Z_LSB		0x06					//	0x07			R		data Bits [7:0]  of Z measurement
#define PIOS_MAG3110_WHO_AM_I		0x07					//	0x08			R		Device ID Number - should return 0xC4
#define PIOS_MAG3110_SYSMOD			0x08					//	0x09			R		data Current System Mode
#define PIOS_MAG3110_OFF_X_MSB		0x09					//	0x0A			R/W		Bits [14:7] of user X offset
#define PIOS_MAG3110_OFF_X_LSB		0x0A					//	0x0B			R/W		Bits [6:0] of user X offset
#define PIOS_MAG3110_OFF_Y_MSB		0x0B					//	0x0C			R/W		Bits [14:7] of user Y offset
#define PIOS_MAG3110_OFF_Y_LSB		0x0C					//	0x0D			R/W		Bits [6:0] of user Y offset
#define PIOS_MAG3110_OFF_Z_MSB		0x0D					//	0x0E			R/W		Bits [14:7] of user Z offset
#define PIOS_MAG3110_OFF_Z_LSB		0x0E					//	0x0F			R/W		Bits [6:0] of user Z offset
#define PIOS_MAG3110_DIE_TEMP		0x0F					//	0x10			R		data Temperature, signed 8 bits in °C
#define PIOS_MAG3110_CTRL_REG1		0x10					//	0x11			R/W		Operation modes
#define PIOS_MAG3110_CTRL_REG2		0x11					//	0x12			R/W		Operation modes
//*******************************************************************************************
//*							PIOS_MAG3110_CTRL_REG1											*
//*******************************************************************************************
//	Data Rate Nomenclature is:
//	"PIOS_MAG3110_ODR_OutputRateInHz_OverSampleRatio"
//									Setting						Output		Over	ADC		Current	Noise
//																Rate(Hz)	Sample	Rate	(µA)	(µT)
//	DR2 DR1 DR0 OS1 OS0														Ratio	(Hz)			rms
//-------------------------------------------------------------------------------------------------------
#define PIOS_MAG3110_ODR_80_16  	(0 	<< 3)				//	80.00		16		1280	900.0	0.4
#define PIOS_MAG3110_ODR_40_32  	(1 	<< 3)				//	40.00		32		1280	900.0	0.35
#define PIOS_MAG3110_ODR_20_64  	(2 	<< 3)				//	20.00		64		1280	900.0	0.3
#define PIOS_MAG3110_ODR_10_128		(3 	<< 3)				//	10.00		128		1280	900.0	0.25
#define PIOS_MAG3110_ODR_40_16  	(4 	<< 3)				//	40.00		16		640		550.0	0.4
#define PIOS_MAG3110_ODR_20_32  	(5 	<< 3)				//	20.00		32		640		550.0	0.35
#define PIOS_MAG3110_ODR_10_64  	(6 	<< 3)				//	10.00		64		640		550.0	0.3
#define PIOS_MAG3110_ODR_5_128  	(7 	<< 3)				//	5.00		128		640		550.0	0.25
#define PIOS_MAG3110_ODR_20_16  	(8 	<< 3)				//	20.00		16		320		275.0	0.4
#define PIOS_MAG3110_ODR_10_32  	(9 	<< 3)				//	10.00		32		320		275.0	0.35
#define PIOS_MAG3110_ODR_5_64   	(10 << 3)				//	5.00		64		320		275.0	0.3
#define PIOS_MAG3110_ODR_2p50_128	(11 << 3)				//	2.50		128		320		275.0	0.25
#define PIOS_MAG3110_ODR_10_16  	(12 << 3)				//	10.00		16		160		137.5	0.4
#define PIOS_MAG3110_ODR_5_32   	(13 << 3)				//	5.00		32		160		137.5	0.35
#define PIOS_MAG3110_ODR_2p50_64	(14 << 3)				//	2.50		64		160		137.5	0.3
#define PIOS_MAG3110_ODR_1p25_128	(15 << 3)				//	1.25		128		160		137.5	0.25
#define PIOS_MAG3110_ODR_5_16   	(16 << 3)				//	5.00		16		80		68.8	0.4
#define PIOS_MAG3110_ODR_2p50_32	(17 << 3)				//	2.50		32		80		68.8	0.35
#define PIOS_MAG3110_ODR_1p25_64	(18 << 3)				//	1.25		64		80		68.8	0.3
#define PIOS_MAG3110_ODR_0p63_128	(19 << 3)				//	0.63		128		80		68.8	0.25
#define PIOS_MAG3110_ODR_2p50_16	(20 << 3)				//	2.50		16		80		34.4	0.4
#define PIOS_MAG3110_ODR_1p25_32	(21 << 3)				//	1.25		32		80		34.4	0.35
#define PIOS_MAG3110_ODR_0p63_64	(22 << 3)				//	0.63		64		80		34.4	0.3
#define PIOS_MAG3110_ODR_0p31_128	(23 << 3)				//	0.31		128		80		34.4	0.25
#define PIOS_MAG3110_ODR_1p25_16	(24 << 3)				//	1.25		16		80		17.2	0.4
#define PIOS_MAG3110_ODR_0p63_32	(25 << 3)				//	0.63		32		80		17.2	0.35
#define PIOS_MAG3110_ODR_0p31_64	(26 << 3)				//	0.31		64		80		17.2	0.3
#define PIOS_MAG3110_ODR_0p16_128	(27 << 3)				//	0.16		128		80		17.2	0.25
#define PIOS_MAG3110_ODR_0p63_16	(28 << 3)				//	0.63		16		80		8.6		0.4
#define PIOS_MAG3110_ODR_0p31_32	(29 << 3)				//	0.31		32		80		8.6		0.35
#define PIOS_MAG3110_ODR_0p16_64	(30 << 3)				//	0.16		64		80		8.6		0.3
#define PIOS_MAG3110_ODR_0p08_128	(31 << 3)				//	0.08		128		80		8.6		0.25

#define PIOS_MAG3110_FAST_READ		(1 << 2)				//			Skips the low byte if auto-increment reading
															//	TM/AC
#define PIOS_MAG3110_STANDBY		((0 << 1) | (0 << 0))	//	0	0	ASIC is in low power standby mode.
#define PIOS_MAG3110_ONESHOT		((1 << 1) | (0 << 0))	//	1	0	The ASIC will exit standby mode, perform one measurement cycle based on the
															//			programmed ODR and OSR setting, update the I²C data registers and re-enter
															//			standby mode.
#define PIOS_MAG3110_CONTINUOUS		((0 << 1) | (1 << 0))	//	0	1	The ASIC will perform continuous measurements based on the current OSR and
															//			ODR settings.
#define PIOS_MAG3110_TRIGGERED		((1 << 1) | (1 << 0))	//	1	1	The ASIC will continue the current measurement at the fastest applicable ODR
															//			for the user programmed OSR. The ASIC will return back to the programmed
															//			ODR after completing the triggered measurement.
//*******************************************************************************************
//*							PIOS_MAG3110_CTRL_REG2											*
//*******************************************************************************************
#define PIOS_MAG3110_AUTO_MRST_EN	(1 << 7)
/*	Automatic Magnetic Sensor Reset. Default value: 0.
	0: Automatic magnetic sensor resets disabled.
	1: Automatic magnetic sensor resets enabled.
	Similar to Mag_RST, however, the resets occur automatically before each data acquisition.
	This bit is recommended to be always explicitly enabled by the host application.
	This a WRITE ONLY bit and always reads back as 0.
*/
#define PIOS_MAG3110_RAW			(1 << 5)
/*	Data output correction. Default value: 0.
	0: Normal mode: data values are corrected by the user offset register values.
	1: Raw mode: data values are not corrected by the user offset register values.
	Note: The factory calibration is always applied to the measured data stored in registers 0x01 to 0x06 irrespective of the
	setting of the RAW bit.
*/
#define PIOS_MAG3110_MAG_RST		(1 << 4)
/*	Magnetic Sensor Reset (One-Shot). Default value: 0.
	0: Reset cycle not active.
	1: Reset cycle initiate or Reset cycle busy/active.
	When asserted, initiates a magnetic sensor reset cycle that will restore correct operation after exposure to an excessive
	magnetic field which exceeds the Full Scale Range (see Table 2) but is less than the Maximum Applied Magnetic Field
	(see Table 3). When the cycle is finished, value returns to 0.
*/
//*********************************************************************************************
//*							PIOS_MAG3110_DR_STATUS											  *
//*																							  *
//* This read-only status register provides the acquisition status information on a per-sample*
//* basis and reflects real-time updates to the OUT_X, OUT_Y, and OUT_Z registers.			  *
//*********************************************************************************************
#define PIOS_MAG3110_ZYXOW			(1 << 7)
/*	X, Y, Z-axis Data Overwrite. Default value: 0.
	0: No data overwrite has occurred.
	1: Previous X or Y or Z data was overwritten by new X or Y or Z data before it was completely read.
*/
#define PIOS_MAG3110_ZOW			(1 << 6)
/*	Z-axis Data Overwrite. Default value: 0.
	0: No data overwrite has occurred.
	1: Previous Z-axis data was overwritten by new Z-axis data before it was read.
*/
#define PIOS_MAG3110_YOW			(1 << 5)
/*	Y-axis Data Overwrite. Default value: 0.
	0: No data overwrite has occurred.
	1: Previous Y-axis data was overwritten by new Y-axis data before it was read.
*/
#define PIOS_MAG3110_XOW			(1 << 4)
/*	X-axis Data Overwrite. Default value: 0
	0: No data overwrite has occurred.
	1: Previous X-axis data was overwritten by new X-axis data before it was read.
*/
#define PIOS_MAG3110_ZYXDR			(1 << 3)
/*	X or Y or Z-axis new Data Ready. Default value: 0.
	0: No new set of data ready.
	1: New set of data is ready.
*/
#define PIOS_MAG3110_ZDR			(1 << 2)
/*	Z-axis new Data Available. Default value: 0.
	0: No new Z-axis data is ready.
	1: New Z-axis data is ready.
*/
#define PIOS_MAG3110_YDR			(1 << 1)
/*	Z-axis new Data Available. Default value: 0.
	0: No new Y-axis data is ready.
	1: New Y-axis data is ready.
*/
#define PIOS_MAG3110_XDR			(1 << 0)
/*	Z-axis new Data Available. Default value: 0.
	0: No new X-axis data is ready.
	1: New X-axis data is ready.
*/
//*******************************************************************************************
//*							PIOS_MAG3110_SYSMOD												*
//*******************************************************************************************
																//	System Mode. Default value: 00.
#define PIOS_MAG3110_SYSMOD_STANDBY		((0 << 1) | (0 << 0))	//	00: STANDBY mode.
#define PIOS_MAG3110_SYSMOD_ACTIVE_RAW	((0 << 1) | (1 << 0))	//	01: ACTIVE mode, RAW data.
#define PIOS_MAG3110_SYSMOD_ACTIVE_CORR	((1 << 1) | (0 << 0))	//	10: ACTIVE mode, non-RAW user-corrected data.
#define PIOS_MAG3110_SYSMOD_ERROR		((1 << 1) | (1 << 0))	//	Not defined in Datasheet. If present something wrong?!


struct pios_MAG3110_cfg{
	uint8_t data_rate;			// ADC Conversion rate and noise tradeoff.
	uint8_t fast_read;			// Whether or not to use fast mode to retrieve the data (8 bits vs 16 on continuous I²C read).
	uint8_t trig_mode;			// One of four trigger modes, see above.
	uint8_t auto_degauss;		// Perform automatic degauss of the sensor before each reading.
	uint8_t raw_read;			// Use the factory calibration only, ignore the user calibration registers.
	uint8_t degauss_now;		// Oneshot degauss of the sensor. This bit will clear itself in the sensor.
};

/* Public Functions */
extern int32_t PIOS_MAG3110_Init(const struct pios_MAG3110_cfg * cfg);
extern int32_t PIOS_MAG3110_NewDataAvailable(void);
extern int32_t PIOS_MAG3110_ReadMag(int16_t out[3]);
extern int32_t PIOS_MAG3110_ReadID(void);
extern int32_t PIOS_MAG3110_Test(void);
extern int32_t PIOS_MAG3110_DeGaussNow(void);

#endif /* PIOS_MAG3110_H */

/** 
  * @}
  * @}
  */
