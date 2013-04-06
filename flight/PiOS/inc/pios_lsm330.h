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

//LSM330 Accelerometer Register Information
#define PIOS_LSM330_G_I2C_ADDR 0x6A
#define LSM330_G_REPEATED 0x80
#define LSM330_CTL_REG1_G 0x20
#define LSM330_CTL_LEN_G 5
#define LSM330_CTL_REG1_G_SETTING 0xAF // 190 samples, 50hz bw, all axis enabled - may want to speed this up later
#define LSM330_CTL_REG2_G_SETTING 0x00 // default
#define LSM330_CTL_REG3_G_SETTING 0x00 // default
#define LSM330_CTL_REG4_G_SETTING 0xB0 // block data update between msb and lsb, 2000 deg/s fs
#define LSM330_CTL_REG5_G_SETTING 0x00 // default value
#define LSM330_G_READ_START 0x28
#define LSM330_G_READ_LEN 6
#define LSM330_G_PER_LSB (float) 4000.0/65535

// public api
bool PIOS_LSM330_init_gyro(void) ;
bool PIOS_LSM330_read_gyro(float gyro_vector[3]);

#endif /* PIOS_LSM330_I2C_H_ */
