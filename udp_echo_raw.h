/*
 * udp_echo_raw.h
 *
 *  Created on: Mar 11, 2017
 *      Author: shaha
 */

#ifndef UDP_ECHO_RAW_H_
#define UDP_ECHO_RAW_H_

static void udpecho_raw_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

void udpecho_raw_init(void);

#endif /* UDP_ECHO_RAW_H_ */
