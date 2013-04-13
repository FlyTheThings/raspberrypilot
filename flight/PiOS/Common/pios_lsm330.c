/*
 * pios_lsm303.c
 *
 * Created: 6/2/2012 4:00:21 PM
 *  Author: zlewko
 */ 

#include "pios.h"
#include "pios_lsm330.h"

// static vars
static uint32_t i2c_id;

// returns 1 if successful
bool PIOS_LSM330_init_gyro(uint32_t pios_i2c_adapter_id) {
	
	i2c_id = pios_i2c_adapter_id;
	const uint8_t init_data_G[] = {
		PIOS_LSM330_CTRL_REG1_G | PIOS_LSM330_G_REPEATED,
		PIOS_LSM330_CTRL_REG1_G_SETTING,
		PIOS_LSM330_CTRL_REG2_G_SETTING,
		PIOS_LSM330_CTRL_REG3_G_SETTING,
		PIOS_LSM330_CTRL_REG4_G_SETTING,
		PIOS_LSM330_CTRL_REG5_G_SETTING
	};
	
	const struct pios_i2c_txn PIOS_lsm303_init_i2c_txn_A[] = {
		{
				.info = "LSM330InitG",
				.addr = PIOS_LSM330_G_ADDR,
				.rw = PIOS_I2C_TXN_WRITE,
				.len = sizeof(init_data_G),
				.buf = (uint8_t *)  &init_data_G
		}
	} ;

	//FIXME add error checking
	if ( PIOS_I2C_Transfer(i2c_id, PIOS_lsm303_init_i2c_txn_A, 1) ) {
		return 0;
	}

	return 1;
}




//returns 1 for success
bool PIOS_LSM330_read_gyro(float gyro_vector[]) {
	static volatile char res[6];
	
	uint8_t subaddr = PIOS_LSM330_G_READ_START | PIOS_LSM330_G_REPEATED;
	const struct pios_i2c_txn PIOS_lsm330_read_i2c_txn_A[] = {
		{
			.info = "LSM330readG",
			.addr = PIOS_LSM330_G_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(subaddr),
			.buf = &subaddr
		},
		{
			.info = "LSM330readG",
			.addr = PIOS_LSM330_G_ADDR,
			.rw = PIOS_I2C_TXN_READ,
			.len = sizeof(res),
			.buf = (uint8_t *) &res
		}
	} ;
	
	if ( PIOS_I2C_Transfer(i2c_id, PIOS_lsm330_read_i2c_txn_A, 2) ) {
		return 0;
	}

	gyro_vector[0] =   ( int16_t ) (res[1] << 8) | res[0];
	gyro_vector[1] =  ( int16_t ) (res[3] << 8) | res[2];
	gyro_vector[2] =   ( int16_t ) (res[5] << 8) | res[4];
	gyro_vector[0] *= PIOS_LSM330_G_PER_LSB ;
	gyro_vector[1] *= PIOS_LSM330_G_PER_LSB ;
	gyro_vector[2] *= PIOS_LSM330_G_PER_LSB ;
	return 1;
}


