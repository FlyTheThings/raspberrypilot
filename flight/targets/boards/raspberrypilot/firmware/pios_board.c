/**
 ******************************************************************************
 * @addtogroup Revolution Revolution configuration files
 * @{
 * @brief Configures the revolution board
 * @{
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Defines board specific static initializers for hardware for the Revolution board.
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

#include <pios.h>

#include <openpilot.h>
#include <uavobjectsinit.h>
#include "hwsettings.h"
#include "manualcontrolsettings.h"
#include "objectpersistence.h"

#include "board_hw_defs.c"

/**
 * Sensor configurations 
 */
 
#if defined(PIOS_INCLUDE_MAG3110)
#include "pios_mag3110.h"
 static const struct pios_MAG3110_cfg pios_mag3110_cfg = {
	.data_rate =	PIOS_MAG3110_ODR_80_16,			// see header file for others
	.fast_read =	0,								// alternately PIOS_MAG3110_FAST_READ
	.trig_mode =	PIOS_MAG3110_CONTINUOUS,		// see header file for others
	.auto_degauss = 0,								// alternately PIOS_MAG3110_AUTO_MRST_EN
	.raw_read =		PIOS_MAG3110_RAW,				// alternately 0
	.degauss_now =	0,								// alternately PIOS_MAG3110_MAG_RST
};
#endif /* PIOS_INCLUDE_MAG3110 */

#if defined(PIOS_INCLUDE_HMC5883)
#include "pios_hmc5883.h"

static const struct pios_hmc5883_cfg pios_hmc5883_cfg = {
	.M_ODR = PIOS_HMC5883_ODR_75,
	.Meas_Conf = PIOS_HMC5883_MEASCONF_NORMAL,
	.Gain = PIOS_HMC5883_GAIN_1_9,
	.Mode = PIOS_HMC5883_MODE_CONTINUOUS,

};
#endif /* PIOS_INCLUDE_HMC5883 */

/**
 * Configuration for the MS5611 chip
 */
#if defined(PIOS_INCLUDE_MS5611)
#include "pios_ms5611.h"
static const struct pios_ms5611_cfg pios_ms5611_cfg = {
	.oversampling = 1,
};
#endif /* PIOS_INCLUDE_MS5611 */

/**
 * Configuration for the BMA180 chip
 */
#if defined(PIOS_INCLUDE_BMA180)
#include "pios_bma180.h"
static const struct pios_exti_cfg pios_exti_bma180_cfg __exti_config = {
	.vector = PIOS_BMA180_IRQHandler,
	.line = EXTI_Line4,
	.pin = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_4,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd = GPIO_PuPd_NOPULL,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI4_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line4, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};
static const struct pios_bma180_cfg pios_bma180_cfg = {
	.exti_cfg = &pios_exti_bma180_cfg,
	.bandwidth = BMA_BW_600HZ,
	.range = BMA_RANGE_8G,
};
#endif /* PIOS_INCLUDE_BMA180 */

/**
 * Configuration for the MPU6000 chip
 */
#if defined(PIOS_INCLUDE_MPU6000)
#include "pios_mpu6000.h"
static const struct pios_exti_cfg pios_exti_mpu6000_cfg __exti_config = {
	.vector = PIOS_MPU6000_IRQHandler,
	.line = EXTI_Line8,
	.pin = {
		.gpio = GPIOD,
		.init = {
			.GPIO_Pin = GPIO_Pin_8,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd = GPIO_PuPd_NOPULL,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI9_5_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line8, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

static const struct pios_mpu6000_cfg pios_mpu6000_cfg = {
	.exti_cfg = &pios_exti_mpu6000_cfg,
	.Fifo_store = PIOS_MPU6000_FIFO_TEMP_OUT | PIOS_MPU6000_FIFO_GYRO_X_OUT | PIOS_MPU6000_FIFO_GYRO_Y_OUT | PIOS_MPU6000_FIFO_GYRO_Z_OUT,
	// Clock at 8 khz, downsampled by 8 for 1khz
	.Smpl_rate_div = 7,
	.interrupt_cfg = PIOS_MPU6000_INT_CLR_ANYRD,
	.interrupt_en = PIOS_MPU6000_INTEN_DATA_RDY,
	.User_ctl = PIOS_MPU6000_USERCTL_FIFO_EN,
	.Pwr_mgmt_clk = PIOS_MPU6000_PWRMGMT_PLL_X_CLK,
	.accel_range = PIOS_MPU6000_ACCEL_8G,
	.gyro_range = PIOS_MPU6000_SCALE_500_DEG,
	.filter = PIOS_MPU6000_LOWPASS_256_HZ
};
#endif /* PIOS_INCLUDE_MPU6000 */

/**
 * Configuration for L3GD20 chip
 */
#if defined(PIOS_INCLUDE_L3GD20)
#include "pios_l3gd20.h"
static const struct pios_exti_cfg pios_exti_l3gd20_cfg __exti_config = {
	.vector = PIOS_L3GD20_IRQHandler,
	.line = EXTI_Line8,
	.pin = {
		.gpio = GPIOD,
		.init = {
			.GPIO_Pin = GPIO_Pin_8,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd = GPIO_PuPd_NOPULL,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI9_5_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line8, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

static const struct pios_l3gd20_cfg pios_l3gd20_cfg = {
	.exti_cfg = &pios_exti_l3gd20_cfg,
	.range = PIOS_L3GD20_SCALE_500_DEG,
};
#endif /* PIOS_INCLUDE_L3GD20 */


/* One slot per selectable receiver group.
 *  eg. PWM, PPM, GCS, SPEKTRUM1, SPEKTRUM2, SBUS
 * NOTE: No slot in this map for NONE.
 */
uint32_t pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE];

#define PIOS_COM_TELEM_RF_RX_BUF_LEN 512
#define PIOS_COM_TELEM_RF_TX_BUF_LEN 512

#define PIOS_COM_GPS_RX_BUF_LEN 32

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 65
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 65

#define PIOS_COM_BRIDGE_RX_BUF_LEN 65
#define PIOS_COM_BRIDGE_TX_BUF_LEN 12

#define PIOS_COM_AUX_RX_BUF_LEN 512
#define PIOS_COM_AUX_TX_BUF_LEN 512

uint32_t pios_com_aux_id = 0;
uint32_t pios_com_gps_id = 0;
uint32_t pios_com_telem_usb_id = 0;
uint32_t pios_com_telem_rf_id = 0;
uint32_t pios_com_bridge_id = 0;
uint32_t pios_com_telem_loop_id = 0;
uint32_t pios_com_loop_a_id = 0;
uint32_t pios_com_loop_b_id = 0;
uint32_t pios_com_uavlink_id = 0;

/* 
 * Setup a com port based on the passed cfg, driver and buffer sizes. tx size of -1 make the port rx only
 */
static void PIOS_Board_configure_com(const struct pios_usart_cfg *usart_port_cfg, size_t rx_buf_len, size_t tx_buf_len,
		const struct pios_com_driver *com_driver, uint32_t *pios_com_id) 
{
	uint32_t pios_usart_id;
	if (PIOS_USART_Init(&pios_usart_id, usart_port_cfg)) {
		PIOS_Assert(0);
	}
	
	uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(rx_buf_len);
	PIOS_Assert(rx_buffer);
	if(tx_buf_len!= -1){ // this is the case for rx/tx ports
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(tx_buf_len);
		PIOS_Assert(tx_buffer);
		
		if (PIOS_COM_Init(pios_com_id, com_driver, pios_usart_id,
				rx_buffer, rx_buf_len,
				tx_buffer, tx_buf_len)) {
			PIOS_Assert(0);
		}
	}
	else{ //rx only port
		if (PIOS_COM_Init(pios_com_id, com_driver, pios_usart_id,
				rx_buffer, rx_buf_len,
				NULL, 0)) {
			PIOS_Assert(0);
		}
	}
}

static void PIOS_Board_configure_dsm(const struct pios_usart_cfg *pios_usart_dsm_cfg, const struct pios_dsm_cfg *pios_dsm_cfg, 
		const struct pios_com_driver *pios_usart_com_driver,enum pios_dsm_proto *proto, 
		ManualControlSettingsChannelGroupsOptions channelgroup,uint8_t *bind)
{
	uint32_t pios_usart_dsm_id;
	if (PIOS_USART_Init(&pios_usart_dsm_id, pios_usart_dsm_cfg)) {
		PIOS_Assert(0);
	}
	
	uint32_t pios_dsm_id;
	if (PIOS_DSM_Init(&pios_dsm_id, pios_dsm_cfg, pios_usart_com_driver,
			pios_usart_dsm_id, *proto, *bind)) {
		PIOS_Assert(0);
	}
	
	uint32_t pios_dsm_rcvr_id;
	if (PIOS_RCVR_Init(&pios_dsm_rcvr_id, &pios_dsm_rcvr_driver, pios_dsm_id)) {
		PIOS_Assert(0);
	}
	pios_rcvr_group_map[channelgroup] = pios_dsm_rcvr_id;
}



/*
 * This function setups up the uavlink hardware port and the loopboack devices for telemetry
 */
static void PIOS_Board_configure_uavlink() {
	//configure the loop back com device
	uint8_t * a_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
	PIOS_Assert(a_buffer);
	uint8_t * b_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
	PIOS_Assert(b_buffer);
	uint32_t pios_com_loopback_id;
	PIOS_com_loopback_Init(&pios_com_loopback_id, a_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN, b_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN);

	// make two com ports using the loopback device,
	uint8_t * rx_buffer_loopback_a = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
	PIOS_Assert(rx_buffer_loopback_a);
	uint8_t * tx_buffer_loopback_a = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
	PIOS_Assert(tx_buffer_loopback_a);
	if (PIOS_COM_Init(&pios_com_telem_rf_id, &pios_com_loopback_com_driver_a, pios_com_loopback_id,
			rx_buffer_loopback_a, PIOS_COM_TELEM_RF_RX_BUF_LEN,
			tx_buffer_loopback_a, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
		PIOS_Assert(0);
	}
	uint8_t * rx_buffer_loopback_b = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
	PIOS_Assert(rx_buffer_loopback_b);
	uint8_t * tx_buffer_loopback_b = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
	PIOS_Assert(tx_buffer_loopback_b);
	if (PIOS_COM_Init(&pios_com_telem_loop_id, &pios_com_loopback_com_driver_b, pios_com_loopback_id,
			rx_buffer_loopback_b, PIOS_COM_TELEM_RF_RX_BUF_LEN,
			tx_buffer_loopback_b, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
		PIOS_Assert(0);
	}


	// configure the uavlink serial port
	uint32_t pios_uavlink_usart_id;
	if (PIOS_USART_Init(&pios_uavlink_usart_id, &pios_usart_telem_cfg)) {
		PIOS_Assert(0);
	}

	uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
	PIOS_Assert(rx_buffer);
	uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
	PIOS_Assert(tx_buffer);

	if (PIOS_COM_Init(&pios_com_uavlink_id, &pios_usart_com_driver, pios_uavlink_usart_id,
			rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
			tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
		PIOS_Assert(0);
	}
}


/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */

#include <pios_board_info.h>
#include <UAVLinkBridge.h>

void PIOS_Board_Init(void) {

//	const struct pios_board_info * bdinfo = &pios_board_info_blob;
	
	/* Delay system */
	PIOS_DELAY_Init();

	PIOS_LED_Init(&pios_led_cfg);
	
	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();
	
	
#if defined(PIOS_INCLUDE_RTC)
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif

	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();


	/* setup the minimum hardware needed for uavlink bridge */
	PIOS_Board_configure_uavlink();
	PIOS_DELAY_WaitmS(100);


	/* Boot up uav linkbridge */
	UavlinkbridgeInitialize();

	HwSettingsInitialize();
	ObjectPersistenceInitialize();

	ObjectPersistenceData objper;
	objper.Operation = OBJECTPERSISTENCE_OPERATION_LOAD;
	objper.ObjectID = HWSETTINGS_OBJID;
	objper.Selection = OBJECTPERSISTENCE_SELECTION_SINGLEOBJECT;
	ObjectPersistenceSet(&objper);
	while(true) {
		vTaskDelay(100);
		ObjectPersistenceGet(&objper);
		if (objper.Operation == OBJECTPERSISTENCE_OPERATION_COMPLETED) break;
		if (objper.Operation == OBJECTPERSISTENCE_OPERATION_ERROR) break;  // in case there was no saved hwsettings
	}



	/* Set up pulse timers */
	PIOS_TIM_InitClock(&tim_1_cfg);
	PIOS_TIM_InitClock(&tim_3_cfg);
	PIOS_TIM_InitClock(&tim_4_cfg);
	PIOS_TIM_InitClock(&tim_5_cfg);
	PIOS_TIM_InitClock(&tim_9_cfg);
	PIOS_TIM_InitClock(&tim_10_cfg);
	PIOS_TIM_InitClock(&tim_11_cfg);

#ifdef PIOS_INCLUDE_SERVO
	PIOS_Servo_Init(&pios_servo_cfg);
#endif
	/* IAP System Setup */
	PIOS_IAP_Init();
	// check for safe mode commands from gcs
	if(PIOS_IAP_ReadBootCmd(0) == PIOS_IAP_CLEAR_FLASH_CMD_0 &&
	   PIOS_IAP_ReadBootCmd(1) == PIOS_IAP_CLEAR_FLASH_CMD_1 &&
	   PIOS_IAP_ReadBootCmd(2) == PIOS_IAP_CLEAR_FLASH_CMD_2)
	{
		 //PIOS_FLASHFS_Format(fs_id);
		 PIOS_IAP_WriteBootCmd(0,0);
		 PIOS_IAP_WriteBootCmd(1,0);
		 PIOS_IAP_WriteBootCmd(2,0);
	}

	// RaspberryPilot removed large section of USB code that would not compile, don't need anyway for raspberrypilot
	
	
	/* Configure IO ports */
	uint8_t hwsettings_DSMxBind;
	HwSettingsDSMxBindGet(&hwsettings_DSMxBind);
	
	PIOS_Board_configure_com(&pios_usart_gps_cfg, PIOS_COM_AUX_RX_BUF_LEN, PIOS_COM_AUX_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_gps_id);
	// This might not want to be here

	const static char msg_set_update_rate[] = "$PMTK220,100*2F\r\n";
	const static char msg_set_sentences[]  = "$PMTK314,1,1,1,1,1,5,0,0,0,0,0,0,0,0,0,0,0,0,0*2C\r\n";
	const static char msg_change_baud[] = "$PMTK251,57600*2C\r\n";
	//const static char msg_hot_start[] = "$PMTK101*32\r\n";
	PIOS_DELAY_WaitmS(1000);  //wait a second the gps can be slow starting
	PIOS_COM_SendBuffer(pios_com_gps_id,(uint8_t *)  &msg_change_baud, sizeof(msg_change_baud));
	PIOS_DELAY_WaitmS(200);
	PIOS_COM_ChangeBaud(pios_com_gps_id,57600);
	PIOS_DELAY_WaitmS(200);
	//PIOS_COM_SendBuffer(pios_com_gps_id, (uint8_t *) &msg_hot_start, sizeof(msg_hot_start));
	//PIOS_DELAY_WaitmS(800);
	PIOS_COM_SendBuffer(pios_com_gps_id, (uint8_t *) &msg_set_sentences, sizeof(msg_set_sentences));
	PIOS_DELAY_WaitmS(200);

	PIOS_COM_SendBuffer(pios_com_gps_id, (uint8_t *) &msg_set_update_rate, sizeof(msg_set_update_rate));
	PIOS_DELAY_WaitmS(200);




	PIOS_DELAY_WaitmS(50);
	
	enum pios_dsm_proto proto = PIOS_DSM_PROTO_DSM2;
	uint8_t DSMxBind = 0;
	PIOS_Board_configure_dsm(&pios_usart_dsm1_aux_cfg, &pios_dsm1_aux_cfg,
			&pios_usart_com_driver,&proto,
			MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMMAINPORT, &DSMxBind);



#if defined(PIOS_INCLUDE_I2C)
{
	if (PIOS_I2C_Init(&pios_i2c_mag_adapter_id, &pios_i2c_mag_adapter_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
//	if (PIOS_I2C_Init(&pios_i2c_flexiport_adapter_id, &pios_i2c_flexiport_adapter_cfg)) {
//		PIOS_Assert(0);
//	}
}
#endif
	
#if defined(PIOS_INCLUDE_ADC)
	PIOS_ADC_Init(&pios_adc_cfg);
#endif

#if defined(PIOS_INCLUDE_HMC5883)
	PIOS_HMC5883_Init(&pios_hmc5883_cfg);
#endif

#if defined(PIOS_INCLUDE_MAG3110)
	PIOS_MAG3110_Init(&pios_mag3110_cfg);
#endif

#if defined(PIOS_INCLUDE_BMP180)
	PIOS_BMP180_Init();
#endif

#if defined(PIOS_INCLUDE_BMP085)
	PIOS_BMP085_Init();
#endif

#if defined(PIOS_INCLUDE_LSM330)
	//#include "pios_lsm330.h"
	PIOS_LSM330_init_gyro(pios_i2c_mag_adapter_id);
#endif

#if defined(PIOS_INCLUDE_LSM303)
	//#include "pios_lsm303.h"
	PIOS_LSM303_init_accel(pios_i2c_mag_adapter_id);
	PIOS_LSM303_init_mag(pios_i2c_mag_adapter_id);
#endif


}

/**
 * @}
 * @}
 */

