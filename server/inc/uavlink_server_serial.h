/*
 * serial.h
 *
 *  Created on: Mar 31, 2013
 *      Author: zlewko
 */

#ifndef SERIAL_H_
#define SERIAL_H_
#include <stdlib.h>
#include <uavlink.h>

int32_t serial_open(void);
int32_t serial_write(uint8_t *buf, uint32_t len) ;
void handle_serial_rx(int fd_uavlink_serial, UAVLinkConnection uav_link_conn) ;
bool wait_uavlink_response(UAVLinkConnection uav_link_conn, uint8_t *buf, uint16_t *len);

#endif /* SERIAL_H_ */
