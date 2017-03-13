/*
 * http_client.h
 *
 *  Created on: 12 במרץ 2017
 *      Author: shahar
 */

#ifndef LWIP_2_0_1_SRC_INCLUDE_LWIP_APPS_HTTP_CLIENT_H_
#define LWIP_2_0_1_SRC_INCLUDE_LWIP_APPS_HTTP_CLIENT_H_


void tcp_setup(void);

void http_send_request(const void* data, u16_t data_length);


#endif /* LWIP_2_0_1_SRC_INCLUDE_LWIP_APPS_HTTP_CLIENT_H_ */
