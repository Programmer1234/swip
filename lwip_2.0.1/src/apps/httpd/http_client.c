#include "lwip/init.h"
#include "lwip/apps/httpd.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/apps/fs.h"
#include "httpd_structs.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"

#include <string.h> /* memset */
#include <stdlib.h> /* atoi */
#include <stdio.h>

#if LWIP_TCP
#define MAX_HTTP_REQ_SIZE    			200
#define DEST_PORT    					80
#define DEST_WEB_SERVER_IP(addr)   		IP4_ADDR((addr), 45, 33, 49, 119)

struct http_client_state {
  u8_t buffer[MAX_HTTP_REQ_SIZE];
  u8_t *ptr;
  u16_t size_left;
  struct tcp_pcb *pcb;
};


char *string_p1 = "HEAD /report.html?data=";
char *string_p2 = " HTTP/1.0\r\nHost: swip.com\r\n\r\n";

/* global static allocation - support only 1 simultanious http request*/
struct tcp_pcb *testpcb;
struct http_client_state ghs;

err_t
my_http_write(struct tcp_pcb *pcb, const void* ptr, u16_t *length, u8_t apiflags)
{
  u16_t len, max_len;
  err_t err;
  len = *length;
  if (len == 0) {
    return ERR_OK;
  }
  /* We cannot send more data than space available in the send buffer. */
  max_len = tcp_sndbuf(pcb);
  if (max_len < len) {
    len = max_len;
  }
  do {
    err = tcp_write(pcb, ptr, len, apiflags);
    if (err == ERR_MEM) {
      if (tcp_sndbuf(pcb) == 0) {
          /* no need to try smaller sizes */
          len = 1;
      } else {
        len /= 2;
      }
    }
  } while ((err == ERR_MEM) && (len > 1));

  if (err == ERR_OK) {
    *length = len;
  } else {
    *length = 0;
  }
  return err;
}


u8_t
my_http_send_data(struct tcp_pcb *pcb, struct http_client_state *hs)
{
  err_t err;
  u16_t len;
  u8_t return_value = 0;

  /* If we were passed a NULL state structure pointer, ignore the call. */
  if (hs == NULL) {
    return 0;
  }

  len = hs->size_left;
  if (len > 0)
  {
	  // if we still have data to send
	  err = my_http_write(pcb, hs->ptr, &len, TCP_WRITE_FLAG_COPY);
	  if (err == ERR_OK) {
		return_value = 1;
	    hs->ptr += len;
	    hs->size_left -= len;
	  }
  }

  return return_value;
}


static err_t
my_http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
  struct http_client_state *hs = (struct http_client_state *)arg;
  if (hs == NULL) {
    return ERR_OK;
  }

  my_http_send_data(pcb, hs);

  return ERR_OK;
}


/* connection established callback, err is unused and only return 0 */
err_t connectCallback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	System_printf("Connection Established.\n"); System_flush();

	struct http_client_state *hs = (struct http_client_state *)arg;
	if (hs == NULL) {
		return ERR_OK;
	}

	/* start sending the request */
	my_http_send_data(tpcb, hs);
    return 0;
}


err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	System_printf("Data recieved.\n");System_flush();
    if (p == NULL) {
    	System_printf("The remote host closed the connection.\nNow I'm closing the connection.\n"); System_flush();
    	tcp_abort(tpcb);
        return ERR_ABRT;
    } else {
    	System_printf("Number of pbufs %d\n", pbuf_clen(p));  System_flush();
    }

    return 0;
}


void
tcpErrorHandler(void *arg, err_t err)
{
	System_printf("http_err: %d", err); System_flush();
}



void http_send_request(const void* data, u16_t data_length)
{
	struct http_client_state *hs;
	u16_t tmp_len;
	u16_t port = DEST_PORT;
    /* create an ip */
    ip4_addr_t ip;
    DEST_WEB_SERVER_IP(&ip);    //IP of my server

    /* create the control block */
    testpcb = tcp_new();    //testpcb is a global struct tcp_pcb

	/* initialize state object for http request */
    hs = &ghs;
    if (hs == NULL) {
    	return 1;
    }
	hs->pcb = testpcb;
	hs->ptr = hs->buffer;

	/* generate start of http req */
	hs->size_left = 0;
	tmp_len = strlen(string_p1);
	SMEMCPY(hs->buffer + hs->size_left, string_p1, tmp_len);
	hs->size_left += tmp_len;

	/* append data (in url parameters) */
	SMEMCPY(hs->buffer + hs->size_left, data, data_length);
	hs->size_left += data_length;

	/* generate end of http req */
	tmp_len = strlen(string_p2);
	SMEMCPY(hs->buffer + hs->size_left, string_p2, tmp_len);
	hs->size_left += tmp_len;

    /* state object to pass to callbacks*/
    tcp_arg(testpcb, hs);

    /* register callbacks with the pcb */
    tcp_err(testpcb, tcpErrorHandler);
    tcp_recv(testpcb, tcpRecvCallback);
    tcp_sent(testpcb, my_http_sent);

    /* now connect */
    tcp_connect(testpcb, &ip, port, connectCallback);
}


void tcp_setup(void)
{
	http_send_request("initialized", 9);
}

#endif




