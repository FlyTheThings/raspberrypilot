/*
 * pios_lsm303.c
 *
 * Created: 6/2/2012 4:00:21 PM
 *  Author: zlewko
 */ 

#include <pios.h>
#include <pios_lsm303.h>

// static vars
static uint32_t accel_i2c_id;
static uint32_t mag_i2c_id;

// returns 1 if successful
bool PIOS_LSM303_init_accel(uint32_t pios_i2c_adapter_id) {

	accel_i2c_id = pios_i2c_adapter_id;
	const uint8_t init_data_A[] = {
		PIOS_LSM303_CTL_REG1_A | PIOS_LSM303_A_REPEATED,
		PIOS_LSM303_CTL_REG1_A_SETTING,
		PIOS_LSM303_CTL_REG2_A_SETTING,
		PIOS_LSM303_CTL_REG3_A_SETTING,
		PIOS_LSM303_CTL_REG4_A_SETTING,
		PIOS_LSM303_CTL_REG5_A_SETTING
	};
	
	const struct pios_i2c_txn PIOS_lsm303_init_i2c_txn_A[] = {
		{
				.info = "LSM303InitA",
				.addr = PIOS_LSM303_A_ADDR,
				.rw = PIOS_I2C_TXN_WRITE,
				.len = sizeof(init_data_A),
				.buf = (uint8_t *)&init_data_A
		}
	} ;

	if ( PIOS_I2C_Transfer(accel_i2c_id, PIOS_lsm303_init_i2c_txn_A, 1) ) {
		return 0;
	}
	
	return 1;
}


// returns 1 if successful
bool PIOS_LSM303_init_mag(uint32_t pios_i2c_adapter_id) {
	
	mag_i2c_id = pios_i2c_adapter_id;
	const uint8_t init_data_M[] = {
			PIOS_LSM303_CRA_REG_M ,
			PIOS_LSM303_CRA_REG_M_SETTING,
			PIOS_LSM303_CRB_REG_M_SETTING,
			PIOS_LSM303_MR_REG_M_SETTING
	};
	
	const struct pios_i2c_txn PIOS_lsm303_init_i2c_txn_M[] = {
		{
				.info = "LSM303InitM",
				.addr = PIOS_LSM303_M_ADDR,
				.rw = PIOS_I2C_TXN_WRITE,
				.len = sizeof(init_data_M),
				.buf = (uint8_t *) &init_data_M
		}
	} ;

	if (PIOS_I2C_Transfer(mag_i2c_id, PIOS_lsm303_init_i2c_txn_M, 1) ) {
		return 0;
	}
	
	return 1;
}

//returns 1 for success
bool PIOS_LSM303_read_accel(float accel_vector[]) {
	uint8_t res[6];
	
	uint8_t subaddr = PIOS_LSM303_A_READ_START | PIOS_LSM303_A_REPEATED;
	const struct pios_i2c_txn PIOS_lsm303_read_i2c_txn_A[] = {
		{
			.info = "LSM303readM",
			.addr = PIOS_LSM303_A_ADDR ,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(subaddr),
			.buf = (uint8_t *) &subaddr
		},
		{
			.info = "LSM303readM",
			.addr = PIOS_LSM303_A_ADDR,
			.rw = PIOS_I2C_TXN_READ,
			.len = sizeof(res),
			.buf =  (uint8_t *) &res
		}
	} ;
	
	if (PIOS_I2C_Transfer(accel_i2c_id, PIOS_lsm303_read_i2c_txn_A, 2) ) {
		return 0;
	}

	accel_vector[0] =   ( int16_t ) (res[1] << 8) | res[0];
	accel_vector[1] =  ( int16_t ) (res[3] << 8) | res[2];
	accel_vector[2] =   ( int16_t ) (res[5] << 8) | res[4];
	accel_vector[0] *= PIOS_LSM303_A_PER_LSB * 9.81;
	accel_vector[1] *= PIOS_LSM303_A_PER_LSB * 9.81;
	accel_vector[2] *= PIOS_LSM303_A_PER_LSB * 9.81;
	return 1;
}

//returns 1 for success
bool PIOS_LSM303_read_mag(float mag_vector[]) {
	static volatile char res[6];
	float tmp;
	
	uint8_t subaddr = PIOS_LSM303_M_READ_START;
	const struct pios_i2c_txn PIOS_lsm303_read_i2c_txn_M[] = {
		{
			.info = "LSM303readA",
			.addr = PIOS_LSM303_M_ADDR ,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(subaddr),
			.buf = (uint8_t *) &subaddr
		},
		{
			.info = "LSM303readA",
			.addr = PIOS_LSM303_M_ADDR,
			.rw = PIOS_I2C_TXN_READ,
			.len = sizeof(res),
			.buf =(uint8_t *)  &res
		}
	} ;

	if (PIOS_I2C_Transfer(mag_i2c_id, PIOS_lsm303_read_i2c_txn_M, 2) ) {
		return 0;
	}

	tmp =   (int16_t) (res[0] << 8 | res[1]) ;
	mag_vector[0] = tmp * PIOS_LSM303_M_X_PER_LSB;

	tmp =   (int16_t) (res[2] << 8 | res[3]) ;
	mag_vector[2] = tmp* PIOS_LSM303_M_Y_PER_LSB;

	tmp =    (int16_t) (res[4] << 8 | res[5]);
	mag_vector[1] = tmp * PIOS_LSM303_M_Z_PER_LSB;
	return 1;
}
