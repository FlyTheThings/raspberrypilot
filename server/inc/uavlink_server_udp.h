#include <sys/socket.h>
#include <netinet/in.h>
#include <uavlink.h>

void handle_udp_stream_rx(int udp_stream_fd, UAVLinkConnection uav_link_conn);
int open_socket_uavlink(int port);
void udp_stream_tx(uint8_t *buf, uint16_t len);
void handle_udp_uavlink_rx(int udp_stream_fd, UAVLinkConnection uav_link_conn);
