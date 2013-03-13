#ifndef PIOS_COM_LOOPBACK_PRIV_H
#define PIOS_COM_LOOPBACK_PRIV_H

#include <pios.h>
#include <pios_stm32.h>
#include "pios_usart.h"

extern const struct pios_com_driver pios_com_loopback_com_driver_a;
extern const struct pios_com_driver pios_com_loopback_com_driver_b;

extern int32_t PIOS_com_loopback_Init(uint32_t * com_loopback_id, uint8_t * a_buffer, uint16_t a_buffer_len, uint8_t * b_buffer, uint16_t b_buffer_len);


#endif /* PIOS_COM_LOOPBACK_PRIV_H */

/**
  * @}
  * @}
  */
