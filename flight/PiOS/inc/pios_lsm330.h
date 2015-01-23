/*
 * pios_lsm330.h
 *
 * Created: 6/2/2012 3:59:17 PM
 *  Author: zlewko
 */ 

#ifndef PIOS_LSM330_H_
#define PIOS_LSM330_H_

#include <pios.h>
#include <stdbool.h>

// LSM330 I²C Address
// -------------------------------------------------------------------------
// 7 Bit LSB follows the SDO_A pin. This address assumes grounded.
#define PIOS_LSM330_I2C_ADDR_DATASHEET_A	0x30 // Add 2 if SDO_A == 1
#define PIOS_LSM330_ADDR_A					(PIOS_LSM330_I2C_ADDR_DATASHEET_A >> 1)
// 7 Bit LSB follows the SDO_G pin. This address assumes grounded.
#define PIOS_LSM330_I2C_ADDR_DATASHEET_G	0xD4 // Add 2 if SDO_G == 1
#define PIOS_LSM330_ADDR_G					(PIOS_LSM330_I2C_ADDR_DATASHEET_G >> 1)

/*****************************************************************************/
/*                Accelerometer Register Map                                 */
/*****************************************************************************/
// In order to read multiple bytes, it is necessary to assert the most significant bit of the sub-
// address field. In other words, SUB(7) must be equal to 1 while SUB(6-0) represents the
// address of first register to be read.
#define PIOS_LSM330_REPEATED_A				(1 << 7)
#define PIOS_LSM330_CTRL_REG1_A				0x20 // Data rate and enables
#define PIOS_LSM330_CTRL_REG2_A				0x21 // Filters
#define PIOS_LSM330_CTRL_REG3_A				0x22 // Ints and clicks
#define PIOS_LSM330_CTRL_REG4_A				0x23 // Scale and high res bit
#define PIOS_LSM330_CTRL_REG5_A				0x24 // Boot and fifo enable
#define PIOS_LSM330_CTRL_REG6_A				0x25 // Int enables
#define PIOS_LSM330_STATUS_REG_A			0x27
#define PIOS_LSM330_OUT_X_L_A				0x28
#define PIOS_LSM330_OUT_X_H_A				0x29
#define PIOS_LSM330_OUT_Y_L_A				0x2A
#define PIOS_LSM330_OUT_Y_H_A				0x2B
#define PIOS_LSM330_OUT_Z_L_A				0x2C
#define PIOS_LSM330_OUT_Z_H_A				0x2D

// CTRL_REG1_A (20h)
// ODR3-0 Data rate selection. Default value: 0
// (0000: Power-down; Others: Refer to Table 21, “Data rate configuration”)
// ------------------------------------------------------------------------------------------
#define PIOS_LSM330_ODR_0_A					(0 << 4)	//	Power-down mode
#define PIOS_LSM330_ODR_1_A					(1 << 4)	//	(1 Hz)
#define PIOS_LSM330_ODR_2_A					(2 << 4)	//	(10 Hz)
#define PIOS_LSM330_ODR_3_A					(3 << 4)	//	(25 Hz)
#define PIOS_LSM330_ODR_4_A					(4 << 4)	//	(50 Hz)
#define PIOS_LSM330_ODR_5_A					(5 << 4)	//	(100 Hz)
#define PIOS_LSM330_ODR_6_A					(6 << 4)	//	(200 Hz)
#define PIOS_LSM330_ODR_7_A					(7 << 4)	//	(400 Hz)
#define PIOS_LSM330_ODR_8_A					(8 << 4)	//	(1620 Hz / Low Power Only)
#define PIOS_LSM330_ODR_9_A					(9 << 4)	//	(1344 Hz / 5376Hz)
// Low power mode enable. Default value: 0
// (0: Normal mode, 1: Low power mode)
#define PIOS_LSM330_LPENA					(1 << 3)
// Zen, Z axis enable. Default value: 1                
// (0: Z axis disabled; 1: Z axis enabled)        
#define PIOS_LSM330_ZEN_A					(1 << 2)
// Yen, Y axis enable. Default value: 1                
// (0: Y axis disabled; 1: Y axis enabled)        
#define PIOS_LSM330_YEN_A					(1 << 1)
// Xen, X axis enable. Default value: 1                
// (0: X axis disabled; 1: X axis enabled)        
#define PIOS_LSM330_XEN_A					(1 << 0)

// CTRL_REG4_A (23h)
// --------------------------------------------------------------------------------

// Full Scale selection. default value: 00
// (00: +/- 2G; 01: +/- 4G; 10: +/- 8G; 11: +/- 16G)
#define PIOS_LSM330_FS_2G_A					(0 << 4)
#define PIOS_LSM330_FS_4G_A					(1 << 4)
#define PIOS_LSM330_FS_8G_A					(2 << 4)
#define PIOS_LSM330_FS_16G_A				(3 << 4)

// HR High resolution output mode: Default value: 0
// (0: High resolution disable; 1: High resolution enable)
#define PIOS_LSM330_HIGH_RES_A				(1 << 3)

// From the Spec Table, not the same as FS/2^16
#define PIOS_LSM330_G_PER_LSB_2G_A			(float) 0.000061
#define PIOS_LSM330_G_PER_LSB_4G_A			(float) 0.000122
#define PIOS_LSM330_G_PER_LSB_6G_A			(float) 0.000183
#define PIOS_LSM330_G_PER_LSB_8G_A			(float) 0.000244
#define PIOS_LSM330_G_PER_LSB_16G_A			(float) 0.000732

/*****************************************************************************/
/*                Gyroscope Register Map                                     */
/*****************************************************************************/
/*  In order to read multiple bytes, it is necessary to assert the most significant
 bit of the subaddress field. In other words, SUB(7) must be equal to 1 while SUB(6-0)
 represents the address of first register to be read. The 7 LSb represent the actual
 register address while the MSb enables address auto increment. If the MSb of the SUB
 field is ‘1’, the SUB (register address) will be automatically increased to allow
 multiple data read/write. */
#define PIOS_LSM330_REPEATED_G				(1 << 7)
#define PIOS_LSM330_WHO_AM_I_G				0x0F // Return value should be 0xC4
#define PIOS_LSM330_CTRL_REG1_G				0x20
#define PIOS_LSM330_CTRL_REG2_G				0x21
#define PIOS_LSM330_CTRL_REG3_G				0x22
#define PIOS_LSM330_CTRL_REG4_G				0x23
#define PIOS_LSM330_CTRL_REG5_G				0x24
#define PIOS_LSM330_REFERENCE_G				0x25
#define PIOS_LSM330_OUT_TEMP_G				0x26
#define PIOS_LSM330_STATUS_REG_G			0x27
#define PIOS_LSM330_OUT_X_L_G				0x28
#define PIOS_LSM330_OUT_X_H_G				0x29
#define PIOS_LSM330_OUT_Y_L_G				0x2A
#define PIOS_LSM330_OUT_Y_H_G				0x2B
#define PIOS_LSM330_OUT_Z_L_G				0x2C
#define PIOS_LSM330_OUT_Z_H_G				0x2D
#define PIOS_LSM330_FIFO_CTRL_REG_G			0x2E
#define PIOS_LSM330_FIFO_SRC_REG_G			0x2F
#define PIOS_LSM330_INT1_CFG_G				0x30
#define PIOS_LSM330_INT1_SRC_G				0x31
#define PIOS_LSM330_INT1_TSH_XH_G			0x32
#define PIOS_LSM330_INT1_TSH_XL_G			0x33
#define PIOS_LSM330_INT1_TSH_YH_G			0x34
#define PIOS_LSM330_INT1_TSH_YL_G			0x35
#define PIOS_LSM330_INT1_TSH_ZH_G			0x36
#define PIOS_LSM330_INT1_TSH_ZL_G			0x37
#define PIOS_LSM330_INT1_DURATION_G			0x38

// CTRL_REG1_G (20h)
// [DR1 DR0 BW1 BW0 PD Zen Xen Yen]
// DR<1:0> is used to set ODR selection. BW <1:0> is used to set bandwidth selection.
// 											Value			ODR(Hz)		Cut-Off
// -----------------------------------------------------------------------------
#define PIOS_LSM330_ODR_0_G					(0x00 << 4)	//	95			12.5
#define PIOS_LSM330_ODR_1_G					(0x01 << 4)	//	95			25
#define PIOS_LSM330_ODR_2_G					(0x02 << 4)	//	95			25
#define PIOS_LSM330_ODR_3_G					(0x03 << 4)	//	95			25
#define PIOS_LSM330_ODR_4_G					(0x04 << 4)	//	190			12.5
#define PIOS_LSM330_ODR_5_G					(0x05 << 4)	//	190			25
#define PIOS_LSM330_ODR_6_G					(0x06 << 4)	//	190			50
#define PIOS_LSM330_ODR_7_G					(0x07 << 4)	//	190			70
#define PIOS_LSM330_ODR_8_G					(0x08 << 4)	//	380			20
#define PIOS_LSM330_ODR_9_G					(0x09 << 4)	//	380			25
#define PIOS_LSM330_ODR_A_G					(0x0A << 4)	//	380			50
#define PIOS_LSM330_ODR_B_G					(0x0B << 4)	//	380			100
#define PIOS_LSM330_ODR_C_G					(0x0C << 4)	//	760			30
#define PIOS_LSM330_ODR_D_G					(0x0D << 4)	//	760			35
#define PIOS_LSM330_ODR_E_G					(0x0E << 4)	//	760			50
#define PIOS_LSM330_ODR_F_G					(0x0F << 4)	//	760			100

// PD Power-down mode enable. Default value: 0
// (0: Power-down mode, 1: Normal mode or Sleep mode)
#define PIOS_LSM330_PD_G					(1 << 3)
// Zen Z axis enable. Default value: 1
// (0: Z axis disabled; 1: Z axis enabled)
#define PIOS_LSM330_ZEN_G					(1 << 2)
// Yen Y axis enable. Default value: 1
// (0: Y axis disabled; 1: Y axis enabled)
#define PIOS_LSM330_YEN_G					(1 << 1)
// Xen X axis enable. Default value: 1
// (0: X axis disabled; 1: X axis enabled)
#define PIOS_LSM330_XEN_G					(1 << 0)

// Angular rate sensor control register 2 (r/w).
// Table 104. CTRL_REG2_G register
//  -------------------------------------------------------------
// |EXTRen | LVLen | HPM1 | HPM0 | HPCF3 | HPCF2 | HPCF1 | HPCF0 |
//  -------------------------------------------------------------
// Table 105. CTRL_REG2_G description

// EXTRen Edge-sensitive trigger enable: Default value: 0
// (0: external trigger disabled; 1: External trigger enabled)
#define PIOS_LSM330_EXTRen_G				(1 << 7)

// LVLen Level-sensitive trigger enable: Default value: 0
// (0: level-sensitive trigger disabled; 1: level-sensitive trigger enabled)
#define PIOS_LSM330_LVLen_G					(1 << 6)

// HPM[1:0] High-pass filter mode selection. Default value: 00
// High-pass filter mode
// Refer to Table 106
// 0 0 Normal mode (reset by reading REFERENCE_G (25h) register)
// 0 1 Reference signal for filtering
// 1 0 Normal mode
// 1 1 Autoreset on interrupt event
#define PIOS_LSM330_HPM_NORMAL_RESET_G				(0 << 4)
#define PIOS_LSM330_HPM_REFERENCE_G					(1 << 4)
#define PIOS_LSM330_HPM_NORMAL_MODE_G				(2 << 4)
#define PIOS_LSM330_HPM_NORMAL_AUTORESET_G			(3 << 4)

// HPCF [3:0] High-pass filter cutoff frequency selection. Default value: 0000
// Table 107. High-pass filter cutoff frequency configuration [Hz]

                                         // HPCF[3:0] ODR=95Hz  ODR=190Hz  ODR=380Hz  ODR=760Hz
#define PIOS_LSM330_HPM_HPCF0_G	(0 << 0) // 0000      7.2       13.5       27         51.4
#define PIOS_LSM330_HPM_HPCF1_G (1 << 0) // 0001      3.5       7.2        13.5       27
#define PIOS_LSM330_HPM_HPCF2_G (2 << 0) // 0010      1.8       3.5        7.2        13.5
#define PIOS_LSM330_HPM_HPCF3_G (3 << 0) // 0011      0.9       1.8        3.5        7.2
#define PIOS_LSM330_HPM_HPCF4_G (4 << 0) // 0100      0.45		0.9        1.8        3.5
#define PIOS_LSM330_HPM_HPCF5_G (5 << 0) // 0101      0.18		0.45       0.9        1.8
#define PIOS_LSM330_HPM_HPCF6_G (6 << 0) // 0110      0.09		0.18       0.45       0.9
#define PIOS_LSM330_HPM_HPCF7_G (7 << 0) // 0111      0.045		0.09       0.18       0.45
#define PIOS_LSM330_HPM_HPCF8_G (8 << 0) // 1000      0.018		0.045      0.09       0.18
#define PIOS_LSM330_HPM_HPCF9_G (9 << 0) // 1001      0.009		0.018      0.045      0.09

// Angular rate sensor control register 5 (r/w).
// Table 112. CTRL_REG5_G register
//  --------------------------------------------------------------------------
// |BOOT  | FIFO_EN | -- | HPen | INT1_Sel1 | INT1_Sel0 | Out_Sel1 | Out_Sel0 |
//  --------------------------------------------------------------------------

// BOOT Reboot memory content. Default value: 0
// (0: Normal mode; 1: reboot memory content)
#define PIOS_LSM330_BOOT_G			(1 << 7)

// FIFO_EN FIFO enable. Default value: 0
// (0: FIFO disabled; 1: FIFO enabled)
#define PIOS_LSM330_FIFO_EN_G		(1 << 6)
   
// HPen High-pass filter enable. Default value: 0
// (0: HPF disabled; 1: HPF enabled, see Figure 21)
#define PIOS_LSM330_HPen_G			(1 << 4)
	
// INT1_Sel[1:0] INT1 selection configuration. Default value: 0
// (see Figure 21)
#define PIOS_LSM330_INT1_Sel_G		(0 << 4)

// Out_Sel[1:0] Out selection configuration. Default value: 0
// (see Figure 21)
#define PIOS_LSM330_Out_Sel_G		(0 << 4)

// CTRL_REG4_G (23h)
// Table 78.
// BDU BLE FS1 FS0 - 0 0 SIM
// Table 79.
// CTRL_REG4_G description
// BDU Block data update. Default value: 0
// (0: continuous update; 1: output registers not updated until MSb and LSb
// reading)
// BLE Big/little endian data selection. Default value 0.
// (0: Data LSb @ lower address; 1: Data MSb @ lower address)
// FS1-FS0 Full scale selection. Default value: 00
// (00: 250 dps; 01: 500 dps; 10: 2000 dps; 11: 2000 dps)
// SIM 3-wire SPI Serial interface read mode enable. Default value: 0
// (0: 3-wire Read mode disabled; 1: 3-wire read enabled).
#define PIOS_LSM330_BDU_G					(1 << 7)
#define PIOS_LSM330_BLE_G					(1 << 6)
#define PIOS_LSM330_FS_250_DPS_G			(0 << 4)
#define PIOS_LSM330_FS_500_DPS_G			(1 << 4)
#define PIOS_LSM330_FS_2000_DPS_G			(2 << 4)
// #define PIOS_LSM330_FS_2000_DPS_G		(3 << 4) Either should work

// From the Spec Table, not the same as FS/2^16
#define PIOS_LSM330_DPS_PER_LSB_250_G		(float) 0.00875
#define PIOS_LSM330_DPS_PER_LSB_500_G		(float) 0.01750
#define PIOS_LSM330_DPS_PER_LSB_2000_G		(float) 0.07000

/*****************************************************************************/
/*            Raspberry Pilot LSM330 Gyro Register Settings                  */
/*****************************************************************************/
// 380 samples, 50hz bw, all axis enabled - may want to speed this up later.
#define PIOS_LSM330_CTRL_REG1_G_SETTING		(PIOS_LSM330_ODR_A_G	| \
											 PIOS_LSM330_PD_G		| \
											 PIOS_LSM330_ZEN_G		| \
											 PIOS_LSM330_YEN_G		| \
											 PIOS_LSM330_XEN_G)
											 
#define PIOS_LSM330_CTRL_REG2_G_SETTING	0x00 // default
// When High Pass Filter Cutoff enabled, 0.18Hz High Pass Frequency.
// #define PIOS_LSM330_CTRL_REG2_G_SETTING		(PIOS_LSM330_HPM_NORMAL_MODE_G |
											 // PIOS_LSM330_HPM_HPCF7_G)

#define PIOS_LSM330_CTRL_REG3_G_SETTING		0x00 // default

// block data update between msb and lsb, 2000 deg/s fs
#define PIOS_LSM330_CTRL_REG4_G_SETTING		(PIOS_LSM330_BDU_G		|\
											 PIOS_LSM330_FS_2000_DPS_G)

// Enable the high pass filter to eliminate offsets.
// #define PIOS_LSM330_CTRL_REG5_G_SETTING		PIOS_LSM330_HPen_G
#define PIOS_LSM330_CTRL_REG5_G_SETTING		0x00 // default value

#define PIOS_LSM330_READ_START_G			PIOS_LSM330_OUT_X_L_G
#define PIOS_LSM330_DPS_PER_LSB				PIOS_LSM330_DPS_PER_LSB_2000_G

/*****************************************************************************/
/*            Raspberry Pilot LSM330 Accel Register Settings                 */
/*****************************************************************************/
// 400Hz, enable all outputs
#define PIOS_LSM330_CTRL_REG1_A_SETTING		(PIOS_LSM330_ODR_7_A	| \
											 PIOS_LSM330_ZEN_A		| \
											 PIOS_LSM330_YEN_A		| \
											 PIOS_LSM330_XEN_A)
											 
#define PIOS_LSM330_CTRL_REG2_A_SETTING		0x00 // default

#define PIOS_LSM330_CTRL_REG3_A_SETTING		0x00 // default

#define PIOS_LSM330_CTRL_REG4_A_SETTING		(PIOS_LSM330_FS_8G_A	| \
											 PIOS_LSM330_HIGH_RES_A)
											
#define PIOS_LSM330_CTRL_REG5_A_SETTING		0x00 // default

#define PIOS_LSM330_CTRL_REG6_A_SETTING		0x00 // default

#define PIOS_LSM330_READ_START_A			PIOS_LSM330_OUT_X_L_A
#define PIOS_LSM330_G_PER_LSB				PIOS_LSM330_G_PER_LSB_8G_A

/*****************************************************************************/
/*            Public API                                                     */
/*****************************************************************************/
bool PIOS_LSM330_init_gyro(uint32_t pios_i2c_adapter_id);
bool PIOS_LSM330_read_gyro(float gyro_vector[3]);

bool PIOS_LSM330_init_accel(uint32_t pios_i2c_adapter_id);
bool PIOS_LSM330_read_accel(float accel_vector[3]);

#endif /* PIOS_LSM330_I2C_H_ */





