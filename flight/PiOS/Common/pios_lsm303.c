/*
 * pios_lsm303.c
 *
 * Created: 6/2/2012 4:00:21 PM
 *  Author: zlewko
 */ 

#include "pios.h"

#if defined(PIOS_INCLUDE_LSM303)
#include "pios_lsm303.h"

// static vars

static int32_t PIOS_LSM303_Read(uint8_t address, uint8_t subaddress, uint8_t * buffer, uint8_t len);
static int32_t PIOS_LSM303_Write(uint8_t address, uint8_t subaddress, uint8_t * buffer);

/**
 * @brief initializes the LSM303 Accelerometer
 * \return 0 for success or -1 for failure
 */
bool PIOS_LSM303_init_accel(void) {

	uint8_t subaddr = LSM303_CTL_REG1_A | LSM303_A_REPEATED;

		uint8_t init_data_A[] = {
		LSM303_CTL_REG1_A_SETTING,
		LSM303_CTL_REG2_A_SETTING,
		LSM303_CTL_REG3_A_SETTING,
		LSM303_CTL_REG4_A_SETTING,
		LSM303_CTL_REG5_A_SETTING
	};
	
	if ( !PIOS_LSM303_Write(LSM303_A_ADDR, subaddr, init_data_A) )
		return 0;
	return -1;
}

/**
 * @brief initializes the LSM303 Accelerometer
 * \return 0 for success or -1 for failure
 */
bool PIOS_LSM303_init_mag(void) {

	uint8_t subaddr = LSM303_CRA_REG_M;

		uint8_t init_data_M[] = {
		LSM303_CRA_REG_M_SETTING,
		LSM303_CRB_REG_M_SETTING,
		LSM303_MR_REG_M_SETTING
	};
	
	if ( !PIOS_LSM303_Write(LSM303_M_ADDR, subaddr, init_data_M) )
		return 0;
	return -1;
}

/**
 * @brief Read current X, Y, Z Accelerometer values
 * \return 0 for success or -1 for failure
 */
bool PIOS_LSM303_read_accel(float accel_vector[]) {
	static /* volatile */ uint8_t result[6];
	
	uint8_t subaddr = LSM303_A_READ_START | LSM303_A_REPEATED;

	if (!PIOS_LSM330_Read(LSM303_A_ADDR, subaddr, result, sizeof(result)))
		return -1;

	accel_vector[0] = ( int16_t ) (result[1] << 8) | result[0];
	accel_vector[1] = ( int16_t ) (result[3] << 8) | result[2];
	accel_vector[2] = ( int16_t ) (result[5] << 8) | result[4];
	accel_vector[0] *= LSM303_A_PER_LSB * 9.81;
	accel_vector[1] *= LSM303_A_PER_LSB * 9.81;
	accel_vector[2] *= LSM303_A_PER_LSB * 9.81;
	return 0;
}

/**
 * @brief Read current X, Y, Z Magnetomter values
 * \return 0 for success or -1 for failure
 */
bool PIOS_LSM303_read_mag(float mag_vector[]) {
	static /* volatile */ uint8_t result[6];
	float tmp;
	
	uint8_t subaddr = LSM303_M_READ_START;
	
	if (!PIOS_LSM330_Read(LSM303_M_ADDR, subaddr, result, sizeof(result)))
		return -1;
		
	tmp =	(int16_t) (result[0] << 8 | result[1]) ;
	mag_vector[0] = tmp * LSM303_M_X_PER_LSB; 
	
	tmp =	(int16_t) (result[2] << 8 | result[3]) ;  
	mag_vector[2] = tmp * LSM303_M_Y_PER_LSB;
	
	tmp =	(int16_t) (result[4] << 8 | result[5]);
	mag_vector[1] = tmp * LSM303_M_Z_PER_LSB;
	return 0;
}

/**
 * @brief Reads one or more bytes into a buffer
 * \param[in] address LSM303 register address
 * \param[out] buffer destination buffer
 * \param[in] len number of bytes which should be read
 * \return 0 if operation was successful
 * \return -1 if error during I2C transfer
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_LSM303_Read(uint8_t address, uint8_t subaddress, uint8_t *buffer, uint8_t len)
{
	uint8_t addr_buffer[] = {
		subaddress,
	};
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = address,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(addr_buffer),
			.buf = addr_buffer,
		}
		,
		{
			.info = __func__,
			.addr = address,
			.rw = PIOS_I2C_TXN_READ,
			.len = len,
			.buf = buffer,
		}
	};
	
	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
 * @brief Writes one or more bytes to the LSM303
 * \param[in] address Register address
 * \param[in] buffer source buffer
 * \return 0 if operation was successful
 * \return -1 if error during I2C transfer
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_LSM303_Write(uint8_t address, uint8_t subaddress, uint8_t *buffer)
{
	uint8_t data[] = {
		subaddress,
		*buffer,
	};
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = address,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(data),
			.buf = data,
		}
		,
	};

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

#endif /* PIOS_INCLUDE_LSM303 */

/**
 * @}
 * @}
 */
