/*
 * pios_LSM330.c
 *
 * Created: 6/2/2012 4:00:21 PM
 *  Author: zlewko
 */ 

#include "pios.h"

#if defined(PIOS_INCLUDE_LSM330)
#include "pios_LSM330.h"

// static vars

static int32_t PIOS_LSM330_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static int32_t PIOS_LSM330_Write(uint8_t address, uint8_t * buffer);

/**
 * @brief initializes the LSM330 Gyroscope
 * \return 0 for success or -1 for failure
 */
bool PIOS_LSM330_init_gyro(void) {
	
	uint8_t subaddr = LSM330_CTL_REG1_G | LSM330_G_REPEATED;
	
		uint8_t init_data_G[] = {
		LSM330_CTL_REG1_G_SETTING,
		LSM330_CTL_REG2_G_SETTING,
		LSM330_CTL_REG3_G_SETTING,
		LSM330_CTL_REG4_G_SETTING,
		LSM330_CTL_REG5_G_SETTING
	};

	if ( !PIOS_LSM330_Write(subaddr, init_data_G) )
		return 0;
	return -1;
}
/**
 * @brief Read current X, Y, Z values
 * \return 0 for success or -1 for failure
 */
bool PIOS_LSM330_read_gyro(float gyro_vector[]) {
	static /*volatile*/ uint8_t result[6];
	
	uint8_t subaddr = LSM330_G_READ_START | LSM330_G_REPEATED;

	if (!PIOS_LSM330_Read(subaddr, result, sizeof(result)))
		return -1;
		
	gyro_vector[0] = ( int16_t ) (result[1] << 8) | result[0];
	gyro_vector[1] = ( int16_t ) (result[3] << 8) | result[2];
	gyro_vector[2] = ( int16_t ) (result[5] << 8) | result[4];
	gyro_vector[0] *= LSM330_G_PER_LSB ;
	gyro_vector[1] *= LSM330_G_PER_LSB ;
	gyro_vector[2] *= LSM330_G_PER_LSB ;
	return 0;
}

/**
 * @brief Reads one or more bytes into a buffer
 * \param[in] address LSM330 register address
 * \param[out] buffer destination buffer
 * \param[in] len number of bytes which should be read
 * \return 0 if operation was successful
 * \return -1 if error during I2C transfer
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_LSM330_Read(uint8_t address, uint8_t *buffer, uint8_t len)
{
	uint8_t addr_buffer[] = {
		address,
	};
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = PIOS_LSM330_G_I2C_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(addr_buffer),
			.buf = addr_buffer,
		}
		,
		{
			.info = __func__,
			.addr = PIOS_LSM330_G_I2C_ADDR,
			.rw = PIOS_I2C_TXN_READ,
			.len = len,
			.buf = buffer,
		}
	};
	
	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
 * @brief Writes one or more bytes to the LSM330
 * \param[in] address Register address
 * \param[in] buffer source buffer
 * \return 0 if operation was successful
 * \return -1 if error during I2C transfer
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_LSM330_Write(uint8_t address, uint8_t *buffer)
{
	uint8_t data[] = {
		address,
		*buffer,
	};
	
	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = PIOS_LSM330_G_I2C_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(data),
			.buf = data,
		}
		,
	};

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}
#endif /* PIOS_INCLUDE_LSM330 */

/**
 * @}
 * @}
 */

