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

/* C runtime includes */
#include <stdarg.h>
#include <time.h>
#include <string.h>
//#include <conio.h>

/* lwIP core includes */
#include "lwip/opt.h"

#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/api.h"

#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"

/* lwIP netif includes */
#include "lwip/etharp.h"
#include "netif/ethernet.h"

/* applications includes */
/**
#include "lwip/apps/lwiperf.h"
#include "lwip/apps/netbiosns.h"
#include "lwip/apps/sntp.h"
#include "lwip/apps/httpd.h"
#include "apps/httpserver/httpserver-netconn.h"
#include "apps/netio/netio.h"
#include "apps/ping/ping.h"
#include "apps/rtp/rtp.h"
#include "apps/chargen/chargen.h"
#include "apps/shell/shell.h"
#include "apps/tcpecho/tcpecho.h"
#include "apps/udpecho/udpecho.h"
#include "apps/tcpecho_raw/tcpecho_raw.h"
#include "apps/socket_examples/socket_examples.h"
*/
#include "lwip/apps/mdns.h"

#if NO_SYS
/* ... then we need information about the timer intervals: */
#include "lwip/ip4_frag.h"
#include "lwip/igmp.h"
#endif /* NO_SYS */

#include "netif/ppp/ppp_opts.h"

/* include the port-dependent configuration */
#include "lwipcfg.h"

/** Define this to 1 to enable a PCAP interface as default interface. */
#ifndef USE_PCAPIF
#define USE_PCAPIF 0
#endif

#ifndef USE_ETHERNET
#define USE_ETHERNET  0
#endif

/** Define this to 1 or 2 to support 1 or 2 SLIP interfaces. */
#ifndef USE_SLIPIF
#define USE_SLIPIF 1
#endif

#if USE_SLIPIF
#include <netif/slipif.h>
#endif /* USE_SLIPIF */

#ifndef USE_DHCP
#define USE_DHCP    LWIP_DHCP
#endif
#ifndef USE_AUTOIP
#define USE_AUTOIP  LWIP_AUTOIP
#endif

#if USE_SLIPIF
struct netif slipif1;
#if USE_SLIPIF > 1
struct netif slipif2;
#endif /* USE_SLIPIF > 1 */
#endif /* USE_SLIPIF */

#if LWIP_NETIF_STATUS_CALLBACK
static void
status_callback(struct netif *state_netif)
{
  if (netif_is_up(state_netif)) {
	  System_printf("status_callback==UP, local interface IP is %s\n", ip4addr_ntoa(netif_ip4_addr(state_netif)));
	  System_flush();
#if LWIP_MDNS_RESPONDER
    mdns_resp_netif_settings_changed(state_netif);
#endif
  } else {
	  System_printf("status_callback==DOWN\n"); System_flush();
  }
}
#endif /* LWIP_NETIF_STATUS_CALLBACK */

#if LWIP_NETIF_LINK_CALLBACK
static void
link_callback(struct netif *state_netif)
{
  if (netif_is_link_up(state_netif)) {
	  System_printf("link_callback==UP\n"); System_flush();
  } else {
	  System_printf("link_callback==DOWN\n"); System_flush();
  }
}
#endif /* LWIP_NETIF_LINK_CALLBACK */

/* This function initializes all network interfaces */
static void
msvc_netif_init(void)
{
#if USE_SLIPIF
  u8_t num_slip1 = 0;
#if LWIP_IPV4
  ip4_addr_t ipaddr_slip1, netmask_slip1, gw_slip1;
#endif
#if USE_SLIPIF > 1
  u8_t num_slip2 = 1;
#if LWIP_IPV4
  ip4_addr_t ipaddr_slip2, netmask_slip2, gw_slip2;
#endif
#endif /* USE_SLIPIF > 1 */
#endif /* USE_SLIPIF */
#if USE_DHCP || USE_AUTOIP
  err_t err;
#endif

#if USE_SLIPIF
#if LWIP_IPV4
#define SLIP1_ADDRS &ipaddr_slip1, &netmask_slip1, &gw_slip1,
  LWIP_PORT_INIT_SLIP1_IPADDR(&ipaddr_slip1);
  LWIP_PORT_INIT_SLIP1_GW(&gw_slip1);
  LWIP_PORT_INIT_SLIP1_NETMASK(&netmask_slip1);
  System_printf("Starting lwIP slipif, local interface IP is %s\n", ip4addr_ntoa(&ipaddr_slip1)); System_flush();
#else
#define SLIP1_ADDRS
  System_printf("Starting lwIP slipif\n"); System_flush();
#endif
#if defined(SIO_USE_COMPORT) && SIO_USE_COMPORT
  num_slip1++; /* COM ports cannot be 0-based */
#endif
  netif_add(&slipif1, SLIP1_ADDRS &num_slip1, slipif_init, ip_input);
#if !USE_ETHERNET
  netif_set_default(&slipif1);
#endif /* !USE_ETHERNET */
#if LWIP_NETIF_STATUS_CALLBACK
  netif_set_status_callback(&slipif1, status_callback);
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
  netif_set_link_callback(&slipif1, link_callback);
#endif /* LWIP_NETIF_LINK_CALLBACK */
  netif_set_up(&slipif1);

#if USE_SLIPIF > 1
#if LWIP_IPV4
#define SLIP2_ADDRS &ipaddr_slip2, &netmask_slip2, &gw_slip2,
  LWIP_PORT_INIT_SLIP2_IPADDR(&ipaddr_slip2);
  LWIP_PORT_INIT_SLIP2_GW(&gw_slip2);
  LWIP_PORT_INIT_SLIP2_NETMASK(&netmask_slip2);
  System_printf("Starting lwIP SLIP if #2, local interface IP is %s\n", ip4addr_ntoa(&ipaddr_slip2)); System_flush();
#else
#define SLIP2_ADDRS
  System_printf ("Starting lwIP SLIP if #2\n"); System_flush();
#endif
#if defined(SIO_USE_COMPORT) && SIO_USE_COMPORT
  num_slip2++; /* COM ports cannot be 0-based */
#endif
  netif_add(&slipif2, SLIP2_ADDRS &num_slip2, slipif_init, ip_input);
#if LWIP_NETIF_STATUS_CALLBACK
  netif_set_status_callback(&slipif2, status_callback);
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
  netif_set_link_callback(&slipif2, link_callback);
#endif /* LWIP_NETIF_LINK_CALLBACK */
  netif_set_up(&slipif2);
#endif /* USE_SLIPIF > 1*/
#endif /* USE_SLIPIF */
}

#if LWIP_DNS_APP && LWIP_DNS
static void
dns_found(const char *name, const ip_addr_t *addr, void *arg)
{
  LWIP_UNUSED_ARG(arg);
  System_printf("%s: %s\n", name, addr ? ipaddr_ntoa(addr) : "<not found>"); System_flush();
}

static void
dns_dorequest(void *arg)
{
  const char* dnsname = "3com.com";
  ip_addr_t dnsresp;
  LWIP_UNUSED_ARG(arg);
 
  if (dns_gethostbyname(dnsname, &dnsresp, dns_found, 0) == ERR_OK) {
    dns_found(dnsname, &dnsresp, 0);
  }
}
#endif /* LWIP_DNS_APP && LWIP_DNS */

#if LWIP_LWIPERF_APP
static void
lwiperf_report(void *arg, enum lwiperf_report_type report_type,
  const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
  u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(local_addr);
  LWIP_UNUSED_ARG(local_port);

  System_printf("IPERF report: type=%d, remote: %s:%d, total bytes: %lu, duration in ms: %lu, kbits/s: %lu\n",
    (int)report_type, ipaddr_ntoa(remote_addr), (int)remote_port, bytes_transferred, ms_duration, bandwidth_kbitpsec);
  System_flush();
}
#endif /* LWIP_LWIPERF_APP */

#if LWIP_MDNS_RESPONDER
static void srv_txt(struct mdns_service *service, void *txt_userdata)
{
   err_t res = mdns_resp_add_service_txtitem(service, "path=/", 6);
   LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
   LWIP_UNUSED_ARG(txt_userdata);
}
#endif

/* This function initializes applications */
static void
apps_init(void)
{
#if LWIP_DNS_APP && LWIP_DNS
  /* wait until the netif is up (for dhcp, autoip or ppp) */
  sys_timeout(5000, dns_dorequest, NULL);
#endif /* LWIP_DNS_APP && LWIP_DNS */

#if LWIP_CHARGEN_APP && LWIP_SOCKET
  chargen_init();
#endif /* LWIP_CHARGEN_APP && LWIP_SOCKET */

#if LWIP_PING_APP && LWIP_RAW && LWIP_ICMP
  ping_init();
#endif /* LWIP_PING_APP && LWIP_RAW && LWIP_ICMP */

#if LWIP_NETBIOS_APP && LWIP_UDP
  netbiosns_init();
#ifndef NETBIOS_LWIP_NAME
#if LWIP_NETIF_HOSTNAME
  netbiosns_set_name(netif_default->hostname);
#else
  netbiosns_set_name("NETBIOSLWIPDEV");
#endif
#endif
#endif /* LWIP_NETBIOS_APP && LWIP_UDP */

#if LWIP_HTTPD_APP && LWIP_TCP
#ifdef LWIP_HTTPD_APP_NETCONN
  http_server_netconn_init();
#else /* LWIP_HTTPD_APP_NETCONN */
  httpd_init();
#endif /* LWIP_HTTPD_APP_NETCONN */
#endif /* LWIP_HTTPD_APP && LWIP_TCP */

#if LWIP_MDNS_RESPONDER
  mdns_resp_init();
#if LWIP_NETIF_HOSTNAME
  mdns_resp_add_netif(netif_default, netif_default->hostname, 3600);
#else
  mdns_resp_add_netif(netif_default, "lwip", 3600);
#endif
  //mdns_resp_add_service(netif_default, "lwipweb", "_http", DNSSD_PROTO_TCP, HTTPD_SERVER_PORT, 3600, srv_txt, NULL);
#endif

#if LWIP_NETIO_APP && LWIP_TCP
  netio_init();
#endif /* LWIP_NETIO_APP && LWIP_TCP */

#if LWIP_RTP_APP && LWIP_SOCKET && LWIP_IGMP
  rtp_init();
#endif /* LWIP_RTP_APP && LWIP_SOCKET && LWIP_IGMP */

#if LWIP_SNTP_APP && LWIP_SOCKET
  sntp_init();
#endif /* LWIP_SNTP_APP && LWIP_SOCKET */

#if LWIP_SHELL_APP && LWIP_NETCONN
  shell_init();
#endif /* LWIP_SHELL_APP && LWIP_NETCONN */
#if LWIP_TCPECHO_APP
#if LWIP_NETCONN && defined(LWIP_TCPECHO_APP_NETCONN)
  tcpecho_init();
#else /* LWIP_NETCONN && defined(LWIP_TCPECHO_APP_NETCONN) */
  tcpecho_raw_init();
#endif
#endif /* LWIP_TCPECHO_APP && LWIP_NETCONN */
#if LWIP_UDPECHO_APP && LWIP_NETCONN
  udpecho_init();
#endif /* LWIP_UDPECHO_APP && LWIP_NETCONN */
#if LWIP_LWIPERF_APP
  lwiperf_start_tcp_server_default(lwiperf_report, NULL);
#endif
#if LWIP_SOCKET_EXAMPLES_APP && LWIP_SOCKET
  socket_examples_init();
#endif /* LWIP_SOCKET_EXAMPLES_APP && LWIP_SOCKET */
#ifdef LWIP_APP_INIT
  LWIP_APP_INIT();
#endif
}

/* This function initializes this lwIP test. When NO_SYS=1, this is done in
 * the main_loop context (there is no other one), when NO_SYS=0, this is done
 * in the tcpip_thread context */
static void
test_init(void * arg)
{ /* remove compiler warning */
#if NO_SYS
  LWIP_UNUSED_ARG(arg);
#else /* NO_SYS */
  sys_sem_t *init_sem;
  LWIP_ASSERT("arg != NULL", arg != NULL);
  init_sem = (sys_sem_t*)arg;
#endif /* NO_SYS */

  /* init network interfaces */
  msvc_netif_init();

  /* init apps */
  apps_init();

#if !NO_SYS
  sys_sem_signal(init_sem);
#endif /* !NO_SYS */
}

/* This is somewhat different to other ports: we have a main loop here:
 * a dedicated task that waits for packets to arrive. This would normally be
 * done from interrupt context with embedded hardware, but we don't get an
 * interrupt in windows for that :-) */
static void
main_loop_lwip(void)
{
#if !NO_SYS
  err_t err;
  sys_sem_t init_sem;
#endif /* NO_SYS */
#if USE_PPP
#if !USE_ETHERNET
  int count;
  u8_t rxbuf[1024];
#endif
  volatile int callClosePpp = 0;
#endif /* USE_PPP */

  /* initialize lwIP stack, network interfaces and applications */
#if NO_SYS
  lwip_init();
  test_init(NULL);
#else /* NO_SYS */
  err = sys_sem_new(&init_sem, 0);
  LWIP_ASSERT("failed to create init_sem", err == ERR_OK);
  LWIP_UNUSED_ARG(err);
  tcpip_init(test_init, &init_sem);
  /* we have to wait for initialization to finish before
   * calling update_adapter()! */
  sys_sem_wait(&init_sem);
  sys_sem_free(&init_sem);
#endif /* NO_SYS */

#if (LWIP_SOCKET || LWIP_NETCONN) && LWIP_NETCONN_SEM_PER_THREAD
  netconn_thread_init();
#endif

  /* MAIN LOOP for driver update (and timers if NO_SYS) */
  //while (!_kbhit()) {
  for (;;) {
#if NO_SYS
    /* handle timers (already done in tcpip.c when NO_SYS=0) */
    sys_check_timeouts();
#endif /* NO_SYS */

#if USE_SLIPIF
    slipif_poll(&slipif1);
#if USE_SLIPIF > 1
    slipif_poll(&slipif2);
#endif /* USE_SLIPIF > 1 */
#endif /* USE_SLIPIF */
#if ENABLE_LOOPBACK && !LWIP_NETIF_LOOPBACK_MULTITHREADING
    /* check for loopback packets on all netifs */
    netif_poll_all();
#endif /* ENABLE_LOOPBACK && !LWIP_NETIF_LOOPBACK_MULTITHREADING */
  }

#if (LWIP_SOCKET || LWIP_NETCONN) && LWIP_NETCONN_SEM_PER_THREAD
  netconn_thread_cleanup();
#endif
}

int main_lwip(void)
{
  /* no stdio-buffering, please! */
  //setvbuf(stdout, NULL,_IONBF, 0);

  main_loop_lwip();

  return 0;
}
