/**
 * Additional settings for the cc2650 port.
 */


/* remember to change this MAC address to suit your needs!
   the last octet will be increased by netif->num for each netif */
#define LWIP_MAC_ADDR_BASE            {0x00,0x01,0x02,0x03,0x04,0x05}

/** Define this to 1 or 2 to support 1 or 2 SLIP interfaces. */
#ifndef USE_SLIPIF
#define USE_SLIPIF 1
#endif

/* #define USE_SLIPIF 0 */
/* #define SIO_USE_COMPORT 0 */
#ifdef USE_SLIPIF
#if USE_SLIPIF
/*
#define LWIP_PORT_INIT_SLIP1_IPADDR(addr)   IP4_ADDR((addr), 10, 0,   0, 111)
#define LWIP_PORT_INIT_SLIP1_GW(addr)       IP4_ADDR((addr), 10, 0,   0, 138)
#define LWIP_PORT_INIT_SLIP1_NETMASK(addr)  IP4_ADDR((addr), 255, 255, 255, 0)
*/
#define LWIP_PORT_INIT_SLIP1_IPADDR(addr)   IP4_ADDR((addr), 192, 168,   43, 111)
#define LWIP_PORT_INIT_SLIP1_GW(addr)       IP4_ADDR((addr),  192, 168,   43, 138)
#define LWIP_PORT_INIT_SLIP1_NETMASK(addr)  IP4_ADDR((addr), 255, 255, 255, 0)
#if USE_SLIPIF > 1
#define LWIP_PORT_INIT_SLIP2_IPADDR(addr)   IP4_ADDR((addr), 192, 168,   2, 1)
#define LWIP_PORT_INIT_SLIP2_GW(addr)       IP4_ADDR((addr), 0,     0,   0, 0)
#define LWIP_PORT_INIT_SLIP2_NETMASK(addr)  IP4_ADDR((addr), 255, 255, 255, 0)*/
#endif /* USE_SLIPIF > 1 */
#endif /* USE_SLIPIF */
#endif /* USE_SLIPIF */

/* configuration for applications */

#define LWIP_HTTPD_APP                1
#define LWIP_HTTPD_CLIENT			  0
#define LWIP_TCPECHO_APP              1
#define LWIP_UDPECHO_APP              1

