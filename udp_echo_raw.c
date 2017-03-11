/*
 * udp_echo_raw.c
 *
 *  Created on: Mar 11, 2017
 *      Author: shaha
 */

#include "lwip/opt.h"
#include "lwip/udp.h"
#include "lwip/debug.h"

#include "udp_echo_raw.h"

static struct udp_pcb *udpecho_raw_pcb;

static void udpecho_raw_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                 const ip_addr_t *addr, u16_t port)
{
  LWIP_UNUSED_ARG(arg);
  if (p != NULL) {
    /* send received packet back to sender */
    udp_sendto(upcb, p, addr, port);
    /* free the pbuf */
    pbuf_free(p);
  }
}

void udpecho_raw_init(void)
{
  udpecho_raw_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
  if (udpecho_raw_pcb != NULL) {
    err_t err;

    err = udp_bind(udpecho_raw_pcb, IP_ANY_TYPE, 7);
    if (err == ERR_OK) {
      udp_recv(udpecho_raw_pcb, udpecho_raw_recv, NULL);
    } else {
      /* abort? output diagnostic? */
    }
  } else {
    /* abort? output diagnostic? */
  }
}



