/*
 * pios_lsm330.c
 *
 * Created: 6/2/2012 4:00:21 PM
 *  Author: zlewko
 */ 

#include "pios.h"
#include "pios_lsm330.h"

// static vars
static uint32_t gyro_i2c_id;
static uint32_t accel_i2c_id;
/******************************************************************************/
/*                                                                            */
/*    LSM330 Gyros                                                            */
/*                                                                            */
/******************************************************************************/
// returns 1 if successful
bool PIOS_LSM330_init_gyro(uint32_t pios_i2c_adapter_id) {
	
	gyro_i2c_id = pios_i2c_adapter_id;
	const uint8_t init_data_G[] = {
		PIOS_LSM330_CTRL_REG1_G | PIOS_LSM330_REPEATED_G,
		PIOS_LSM330_CTRL_REG1_G_SETTING,
		PIOS_LSM330_CTRL_REG2_G_SETTING,
		PIOS_LSM330_CTRL_REG3_G_SETTING,
		PIOS_LSM330_CTRL_REG4_G_SETTING,
		PIOS_LSM330_CTRL_REG5_G_SETTING
	};
	
	const struct pios_i2c_txn PIOS_lsm330_init_i2c_txn_G[] = {
		{
				.info = "LSM330InitG",
				.addr = PIOS_LSM330_ADDR_G,
				.rw = PIOS_I2C_TXN_WRITE,
				.len = sizeof(init_data_G),
				.buf = (uint8_t *)  &init_data_G
		}
	};

	//FIXME add error checking
	if ( PIOS_I2C_Transfer(gyro_i2c_id, PIOS_lsm330_init_i2c_txn_G, 1) ) {
		return 0;
	}

	return 1;
}

//returns 1 for success
bool PIOS_LSM330_read_gyro(float gyro_vector[]) {
	static volatile char gyro_res[6];
	
	uint8_t subaddr = PIOS_LSM330_READ_START_G | PIOS_LSM330_REPEATED_G;
	const struct pios_i2c_txn PIOS_lsm330_read_i2c_txn_G[] = {
		{
			.info = "LSM330readG",
			.addr = PIOS_LSM330_ADDR_G,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(subaddr),
			.buf = &subaddr
		},
		{
			.info = "LSM330readG",
			.addr = PIOS_LSM330_ADDR_G,
			.rw = PIOS_I2C_TXN_READ,
			.len = sizeof(gyro_res),
			.buf = (uint8_t *) &gyro_res
		}
	};
	
	if ( PIOS_I2C_Transfer(gyro_i2c_id, PIOS_lsm330_read_i2c_txn_G, 2) ) {
		return 0;
	}

	gyro_vector[0] = ( int16_t ) (gyro_res[1] << 8) | gyro_res[0];
	gyro_vector[1] = ( int16_t ) (gyro_res[3] << 8) | gyro_res[2];
	gyro_vector[2] = ( int16_t ) (gyro_res[5] << 8) | gyro_res[4];
	gyro_vector[0] *= PIOS_LSM330_DPS_PER_LSB ;
	gyro_vector[1] *= PIOS_LSM330_DPS_PER_LSB ;
	gyro_vector[2] *= PIOS_LSM330_DPS_PER_LSB ;
	return 1;
}
/******************************************************************************/
/*                                                                            */
/*    LSM330 Accels                                                           */
/*                                                                            */
/******************************************************************************/
// returns 1 if successful
bool PIOS_LSM330_init_accel(uint32_t pios_i2c_adapter_id) {
	
	accel_i2c_id = pios_i2c_adapter_id;
	const uint8_t init_data_A[] = {
		PIOS_LSM330_CTRL_REG1_A | PIOS_LSM330_REPEATED_A,
		PIOS_LSM330_CTRL_REG1_A_SETTING,
		PIOS_LSM330_CTRL_REG2_A_SETTING,
		PIOS_LSM330_CTRL_REG3_A_SETTING,
		PIOS_LSM330_CTRL_REG4_A_SETTING,
		PIOS_LSM330_CTRL_REG5_A_SETTING,
		PIOS_LSM330_CTRL_REG6_A_SETTING
	};
	
	const struct pios_i2c_txn PIOS_lsm330_init_i2c_txn_A[] = {
		{
				.info = "LSM330InitA",
				.addr = PIOS_LSM330_ADDR_A,
				.rw = PIOS_I2C_TXN_WRITE,
				.len = sizeof(init_data_A),
				.buf = (uint8_t *)  &init_data_A
		}
	};

	//FIXME add error checking
	if ( PIOS_I2C_Transfer(accel_i2c_id, PIOS_lsm330_init_i2c_txn_A, 1) ) {
		return 0;
	}

	return 1;
}

//returns 1 for success
bool PIOS_LSM330_read_accel(float accel_vector[]) {
	static volatile char accel_res[6];
	
	uint8_t subaddr = PIOS_LSM330_READ_START_A | PIOS_LSM330_REPEATED_A;
	const struct pios_i2c_txn PIOS_lsm330_read_i2c_txn_A[] = {
		{
			.info = "LSM330readA",
			.addr = PIOS_LSM330_ADDR_A,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(subaddr),
			.buf = &subaddr
		},
		{
			.info = "LSM330readA",
			.addr = PIOS_LSM330_ADDR_A,
			.rw = PIOS_I2C_TXN_READ,
			.len = sizeof(accel_res),
			.buf = (uint8_t *) &accel_res
		}
	};
	
	if ( PIOS_I2C_Transfer(accel_i2c_id, PIOS_lsm330_read_i2c_txn_A, 2) ) {
		return 0;
	}

	accel_vector[0] = ( int16_t ) (accel_res[1] << 8) | accel_res[0];
	accel_vector[1] = ( int16_t ) (accel_res[3] << 8) | accel_res[2];
	accel_vector[2] = ( int16_t ) (accel_res[5] << 8) | accel_res[4];
	accel_vector[0] *= PIOS_LSM330_G_PER_LSB * 9.81;
	accel_vector[1] *= PIOS_LSM330_G_PER_LSB * 9.81; // To get from G's to m/s^2
	accel_vector[2] *= PIOS_LSM330_G_PER_LSB * 9.81;
	return 1;
}
