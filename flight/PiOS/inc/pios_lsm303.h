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
#define LSM303_A_ADDR 0x19				// 7 Bit address
#define LSM303_A_REPEATED 0x80
#define LSM303_CTL_REG1_A 0x20
#define LSM303_CTL_LEN_A 5
#define LSM303_CTL_REG1_A_SETTING 0x2F	// 100hz rate, all axis enabled
#define LSM303_CTL_REG2_A_SETTING 0x00	// default
#define LSM303_CTL_REG3_A_SETTING 0x00	// default
#define LSM303_CTL_REG4_A_SETTING 0xB0	// block data update between msb and lsb, 8 g fs
#define LSM303_CTL_REG5_A_SETTING 0x00	// default value
#define LSM303_A_READ_START 0x28
#define LSM303_A_READ_LEN 6
#define LSM303_A_PER_LSB (float) 16/65535

//LSM303 Compass Register Information
#define LSM303_M_ADDR 0x1E				// 7 Bit address
#define LSM303_CTL_LEN_M 3
#define LSM303_CRA_REG_M 0x00
#define LSM303_CRA_REG_M_SETTING 0x18
#define LSM303_CRB_REG_M_SETTING 0x20
#define LSM303_MR_REG_M_SETTING 0x00
#define LSM303_M_READ_START 0x03
#define LSM303_M_READ_LEN 6
#define LSM303_M_X_PER_LSB (float)1/1100
#define LSM303_M_Y_PER_LSB (float)1/1100
#define LSM303_M_Z_PER_LSB (float)1/980


struct vector_3d {
	float x ;
	float y ;
	float z ;
};


// public api
bool PIOS_lsm303_init_accel(void) ;
bool PIOS_lsm303_read_accel(float accel_vector[]);

bool PIOS_lsm303_init_mag(void) ;
bool PIOS_lsm303_read_mag(float mag_vector[]);


#endif /* PIOS_LSM303_H_ */
