/*
 * Copyright (c) 2001,2002 Florian Schulze.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * test.c - This file is part of lwIP test
 *
 */

#include "lwip_main.h"

/* C runtime includes */
#include <stdarg.h>
#include <time.h>
#include <string.h>

/* lwIP core includes */
#include "lwip/opt.h"

#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#include "lwip/netif.h"

#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/udp.h"

/* lwIP netif includes */
#include "lwip/etharp.h"

#include "lwip/apps/mdns.h"
#include "lwip/ip4_frag.h"
#include "lwip/igmp.h"

/* include the port-dependent configuration */
#include "lwipcfg.h"

/* Include slip related header*/
#include <netif/slipif.h>

/* Include apps headers*/
#include "udp_echo_raw.h"

struct netif slipif1;

static void status_callback(struct netif *state_netif)
{
  if (netif_is_up(state_netif)) {
	  System_printf("status_callback==UP, local interface IP is %s\n", ip4addr_ntoa(netif_ip4_addr(state_netif)));
	  System_flush();
  } else {
	  System_printf("status_callback==DOWN\n"); System_flush();
  }
}

static void link_callback(struct netif *state_netif)
{
  if (netif_is_link_up(state_netif)) {
	  System_printf("link_callback==UP\n"); System_flush();
  } else {
	  System_printf("link_callback==DOWN\n"); System_flush();
  }
}

/* This function initializes all network interfaces */
static void netif_init(void)
{
  u8_t num_slip1 = 0;
  ip4_addr_t ipaddr_slip1, netmask_slip1, gw_slip1;

#define SLIP1_ADDRS &ipaddr_slip1, &netmask_slip1, &gw_slip1,
  LWIP_PORT_INIT_SLIP1_IPADDR(&ipaddr_slip1);
  LWIP_PORT_INIT_SLIP1_GW(&gw_slip1);
  LWIP_PORT_INIT_SLIP1_NETMASK(&netmask_slip1);
  System_printf("Starting lwIP slipif, local interface IP is %s\n", ip4addr_ntoa(&ipaddr_slip1)); System_flush();

  netif_add(&slipif1, SLIP1_ADDRS &num_slip1, slipif_init, ip_input);
  netif_set_default(&slipif1);

  netif_set_status_callback(&slipif1, status_callback);
  netif_set_link_callback(&slipif1, link_callback);

  /* Set netif up, and link layer up*/
  netif_set_up(&slipif1);
  netif_set_link_up(&slipif1);
}

/* This function initializes applications */
static void apps_init(void)
{
	/* Initialize UDP echo server on port 7 */
	udpecho_raw_init();
}

/* This function initializes this lwIP test. When NO_SYS=1, this is done in
 * the main_loop context (there is no other one), when NO_SYS=0, this is done
 * in the tcpip_thread context */
static void lwip_apps_init()
{
  /* Initialize network interfaces */
  netif_init();

  /* Initialize applications */
  apps_init();
}

/* This is somewhat different to other ports: we have a main loop here:
 * a dedicated task that waits for packets to arrive. This would normally be
 * done from interrupt context with embedded hardware, but we don't get an
 * interrupt in windows for that :-) */
static void main_loop_lwip(void)
{
  /* initialize lwIP stack, network interfaces and applications */
  lwip_init();
  lwip_apps_init();

  /* MAIN LOOP for driver update and timers */
  for (;;) {
    /* Handle timers */
    sys_check_timeouts();

    /* Driver update */
    slipif_poll(&slipif1);
  }
}

int main_lwip(void)
{
  main_loop_lwip();

  return 0;
}
