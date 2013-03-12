/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_com_loopback com_loopback Functions
 * @brief PIOS interface for com_loopback port
 * @{
 *
 * @file       pios_com_loopback.c
 * @author
 * @brief      com_loopback commands. Inits com_loopbacks, controls com_loopbacks & Interupt handlers. (STM32 dependent)
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

/*
 * @todo This is virtually identical to the F1xx driver and should be merged.
 */

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_COM_LOOPBACK)

#include <pios_com_loopback_priv.h>

/* Provide a COM driver */
static void PIOS_com_loopback_ChangeBaud(uint32_t com_loopback_id, uint32_t baud);
static void PIOS_com_loopback_RegisterRxCallback_a(uint32_t com_loopback_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_com_loopback_RegisterTxCallback_a(uint32_t com_loopback_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_com_loopback_TxStart_a(uint32_t com_loopback_id, uint16_t tx_bytes_avail);
static void PIOS_com_loopback_RxStart_a(uint32_t com_loopback_id, uint16_t rx_bytes_avail);

static void PIOS_com_loopback_RegisterRxCallback_b(uint32_t com_loopback_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_com_loopback_RegisterTxCallback_b(uint32_t com_loopback_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_com_loopback_TxStart_b(uint32_t com_loopback_id, uint16_t tx_bytes_avail);
static void PIOS_com_loopback_RxStart_b(uint32_t com_loopback_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_com_loopback_com_a_driver = {
	.set_baud   = PIOS_com_loopback_ChangeBaud,
	.tx_start   = PIOS_com_loopback_TxStart_a,
	.rx_start   = PIOS_com_loopback_RxStart_a,
	.bind_tx_cb = PIOS_com_loopback_RegisterRxCallback_a,
	.bind_rx_cb = PIOS_com_loopback_RegisterTxCallback_a,
};

const struct pios_com_driver pios_com_loopback_com_b_driver = {
	.set_baud   = PIOS_com_loopback_ChangeBaud,
	.tx_start   = PIOS_com_loopback_TxStart_b,
	.rx_start   = PIOS_com_loopback_RxStart_b,
	.bind_tx_cb = PIOS_com_loopback_RegisterRxCallback_b,
	.bind_rx_cb = PIOS_com_loopback_RegisterTxCallback_b,
};

enum pios_com_loopback_dev_magic {
	PIOS_com_loopback_DEV_MAGIC = 0x2852234A,
};

struct pios_com_loopback_dev {
	enum pios_com_loopback_dev_magic     magic;

	bool rx_a_started;
	bool rx_b_started;

	uint8_t *com_loopback_a_buffer;
	uint16_t com_loopback_a_buffer_len;
	uint8_t *com_loopback_b_buffer;
	uint16_t com_loopback_b_buffer_len;

	pios_com_callback rx_in_cb_a;
	uint32_t rx_in_context_a;
	pios_com_callback rx_in_cb_b;
	uint32_t rx_in_context_b;

	pios_com_callback tx_out_cb_a;
	uint32_t tx_out_context_a;
	pios_com_callback tx_out_cb_b;
	uint32_t tx_out_context_b;
};

static bool PIOS_com_loopback_validate(struct pios_com_loopback_dev * com_loopback_dev)
{
	return (com_loopback_dev->magic == PIOS_com_loopback_DEV_MAGIC);
}


static struct pios_com_loopback_dev * PIOS_com_loopback_alloc(void)
{
	struct pios_com_loopback_dev * com_loopback_dev;

	com_loopback_dev = (struct pios_com_loopback_dev *)pvPortMalloc(sizeof(*com_loopback_dev));
	if (!com_loopback_dev) return(NULL);

	com_loopback_dev->rx_started_a = 0;
	com_loopback_dev->rx_started_b = 0;
	com_loopback_dev->a_buffer = 0;
	com_loopback_dev->a_buffer_len = 0;
	com_loopback_dev->b_buffer = 0;
	com_loopback_dev->b_buffer_len = 0;

	com_loopback_dev->rx_in_cb_a = 0;
	com_loopback_dev->rx_in_context_b = 0;
	com_loopback_dev->rx_in_cb_b = 0;
	com_loopback_dev->rx_in_context_b = 0;


	com_loopback_dev->tx_out_cb_a = 0;
	com_loopback_dev->tx_out_context_a = 0;
	com_loopback_dev->tx_out_cb_b = 0;
	com_loopback_dev->tx_out_context_b = 0;

	com_loopback_dev->magic = PIOS_com_loopback_DEV_MAGIC;
	return(com_loopback_dev);
}


/**
* Initialise a single com_loopback device
*/
int32_t PIOS_com_loopback_Init(uint32_t * com_loopback_id, uint8_t * a_buffer, uint16_t a_buffer_len, uint8_t * b_buffer, uint16_t b_buffer_len)
{
	PIOS_DEBUG_Assert(com_loopback_id);
	PIOS_DEBUG_Assert(a_buffer);
	PIOS_DEBUG_Assert(a_buffer_len);
	PIOS_DEBUG_Assert(b_buffer);
	PIOS_DEBUG_Assert(b_buffer_len);

	//allocate memory for the structure
	struct pios_com_loopback_dev * com_loopback_dev;
	com_loopback_dev = (struct pios_com_loopback_dev *) PIOS_com_loopback_alloc();
	if (!com_loopback_dev) goto out_fail;

	// copy in the buffer data
	com_loopback_dev->buffer_a = a_buffer;
	com_loopback_dev->buffer_len_a = a_buffer_len;
	com_loopback_dev->buffer_b = b_buffer;
	com_loopback_dev->buffer_len_b = b_buffer_len;

	// set the id handle and return
	*com_loopback_id = (uint32_t)com_loopback_dev;
	return(0);

out_fail:
	return(-1);
}

static void PIOS_com_loopback_RxStart_a(uint32_t com_loopback_id, uint16_t rx_bytes_avail)
{
	struct pios_com_loopback_dev * com_loopback_dev = (struct pios_com_loopback_dev *)com_loopback_id;

	bool valid = PIOS_com_loopback_validate(com_loopback_dev);
	PIOS_Assert(valid);

	pios_com_loopback_dev->rx_started_a = true;
}
static void PIOS_com_loopback_RxStart_b(uint32_t com_loopback_id, uint16_t rx_bytes_avail)
{
	struct pios_com_loopback_dev * com_loopback_dev = (struct pios_com_loopback_dev *)com_loopback_id;
	
	bool valid = PIOS_com_loopback_validate(com_loopback_dev);
	PIOS_Assert(valid);
	
	pios_com_loopback_dev->rx_started_b = true;
}


static void PIOS_com_loopback_TxStart_a(uint32_t com_loopback_id, uint16_t tx_bytes_avail)
{
	struct pios_com_loopback_dev * com_loopback_dev = (struct pios_com_loopback_dev *)com_loopback_id;
	
	bool valid = PIOS_com_loopback_validate(com_loopback_dev);
	PIOS_Assert(valid);
	
	PIOS_com_loopback_tx (tx_bytes_avail, tx_out_cb_a, tx_out_context_a, com_loopback_dev->buffer_b, com_loopback_dev->buffer_len_b, com_loopback_dev->rx_started_b, com_loopback_dev->rx_in_cb_b, com_loopback_dev->rx_in_context_b)
}


static void PIOS_com_loopback_tx(uint16_t tx_bytes_avail,pios_com_callback tx_out_cb,uint32_t tx_out_context,uint8_t *buffer,uint16_t buffer_len,bool rx_started,pios_com_callback rx_in_cb,uint32_t rx_in_context)
{
	bool rx_need_yield = false;
	bool tx_need_yield = false;

	uint16_t bytes_to_send;
	uint16_t tx_headroom;

	 do {
		bytes_to_send = (com_loopback_dev->tx_out_cb_a)(tx_out_context, buffer, buffer_len, &tx_headroom, &tx_need_yield);
		if (com_loopback_dev->rx_in_cb_b & rx_started) {
			(void) (com_loopback_dev->rx_in_?_cb)(com_loopback_dev->rx_in_b_context, buffer, bytes_to_send, NULL, &rx_need_yield);
#if defined(PIOS_INCLUDE_FREERTOS)
			if (rx_need_yield) {
				vPortYield();
				}
#endif	/* PIOS_INCLUDE_FREERTOS */
		}

	} while (bytes_to_send + headroom)

	// this yields only after transfer is done since its purpose is to allow the writer to refill the buffer
#if defined(PIOS_INCLUDE_FREERTOS)
	if (tx_need_yield) {
		vPortYield();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */


}















/**
* Changes the baud rate of the com_loopback peripheral without re-initialising.
* \param[in] com_loopback_id com_loopback name (GPS, TELEM, AUX)
* \param[in] baud Requested baud rate
*/
static void PIOS_com_loopback_ChangeBaud(uint32_t com_loopback_id, uint32_t baud)
{
	struct pios_com_loopback_dev * com_loopback_dev = (struct pios_com_loopback_dev *)com_loopback_id;

	bool valid = PIOS_com_loopback_validate(com_loopback_dev);
	PIOS_Assert(valid);

	// nothing to do, loopback doesn't have a baudrate
}

static void PIOS_com_loopback_RegisterRxCallback(uint32_t com_loopback_id, pios_com_callback rx_in_cb, uint32_t context)
{
	struct pios_com_loopback_dev * com_loopback_dev = (struct pios_com_loopback_dev *)com_loopback_id;

	bool valid = PIOS_com_loopback_validate(com_loopback_dev);
	PIOS_Assert(valid);
	
	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	com_loopback_dev->rx_in_context = context;
	com_loopback_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_com_loopback_RegisterTxCallback(uint32_t com_loopback_id, pios_com_callback tx_out_cb, uint32_t context)
{
	struct pios_com_loopback_dev * com_loopback_dev = (struct pios_com_loopback_dev *)com_loopback_id;

	bool valid = PIOS_com_loopback_validate(com_loopback_dev);
	PIOS_Assert(valid);
	
	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	com_loopback_dev->tx_out_context = context;
	com_loopback_dev->tx_out_cb = tx_out_cb;
}

#endif

/**
  * @}
  * @}
  */
