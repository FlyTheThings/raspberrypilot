/*
 * pios_lsm303.h
 *
 * Created: 6/2/2012 3:59:17 PM
 *  Author: zlewko
 */ 


#ifndef PIOS_LSM303_H_
#define PIOS_LSM303_H_

#include <pios.h>
#include <stdbool.h>


//LSM303 Accelerometer Register Information
#define PIOS_LSM303_A_ADDR					0x19	// 7 Bit address SAO_A pin high on RP
#define PIOS_LSM303_A_REPEATED				0x80
#define PIOS_LSM303_CTL_REG1_A				0x20
#define PIOS_LSM303_CTL_LEN_A				5
#define PIOS_LSM303_CTL_REG1_A_SETTING		0x2F	// 100hz rate, all axis enabled
#define PIOS_LSM303_CTL_REG2_A_SETTING		0x00	// default
#define PIOS_LSM303_CTL_REG3_A_SETTING		0x00	// default
#define PIOS_LSM303_CTL_REG4_A_SETTING		0xB0	// block data update between msb and lsb, 8 g fs
#define PIOS_LSM303_CTL_REG5_A_SETTING		0x00	// default value
#define PIOS_LSM303_A_READ_START			0x28
#define PIOS_LSM303_A_READ_LEN				6
#define PIOS_LSM303_A_PER_LSB				(float) 16/65535

//LSM303 Compass Register Information
#define PIOS_LSM303_M_ADDR					0x1E	// 7 Bit address
#define PIOS_LSM303_CTL_LEN_M				3
#define PIOS_LSM303_CRA_REG_M				0x00
#define PIOS_LSM303_CRA_REG_M_SETTING		0x18
#define PIOS_LSM303_CRB_REG_M_SETTING		0x20
#define PIOS_LSM303_MR_REG_M_SETTING		0x00
#define PIOS_LSM303_M_READ_START			0x03
#define PIOS_LSM303_M_READ_LEN				6
#define PIOS_LSM303_M_X_PER_LSB				(float)1/1100
#define PIOS_LSM303_M_Y_PER_LSB				(float)1/1100
#define PIOS_LSM303_M_Z_PER_LSB				(float)1/980


struct vector_3d {
	float x ;
	float y ;
	float z ;
};


// public api
bool PIOS_LSM303_init_accel(uint32_t pios_i2c_adapter_id) ;
bool PIOS_LSM303_read_accel(float accel_vector[]);

bool PIOS_LSM303_init_mag(uint32_t pios_i2c_adapter_id) ;
bool PIOS_LSM303_read_mag(float mag_vector[]);


#endif /* PIOS_LSM303_H_ */
