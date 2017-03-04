/*
 * sio.c
 *
 *  Created on: Feb 25, 2017
 *      Author: shaha
 */


#include <lwip/sio.h>
#include <ti/drivers/UART.h>
#include "Board.h"

#ifndef SIO_COMPORT_SPEED
#define SIO_COMPORT_SPEED 115200
#endif

UART_Handle g_uart_handle;

/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
	UART_Params uartParams;
	UART_Params_init(&uartParams);

	uartParams.readMode = UART_MODE_BLOCKING;
	uartParams.readDataMode = UART_DATA_BINARY;
	uartParams.readReturnMode = UART_RETURN_FULL;
	uartParams.readTimeout = 10;

	uartParams.writeMode = UART_MODE_BLOCKING;
	uartParams.writeDataMode = UART_DATA_BINARY;
	//uartParams.writeTimeout=1000;

	uartParams.baudRate = SIO_COMPORT_SPEED;

	g_uart_handle = UART_open(Board_UART, &uartParams);

	if(!g_uart_handle) {
		System_abort("UART_open failed!");
		return NULL;
	}

	return g_uart_handle;
}


/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
	int ret = 0;

	// This is blocking, until the character is written
	ret = UART_write(fd, &c, sizeof(c));

	if (UART_ERROR == ret) {
		System_abort("UART_write failed!");
	}

}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t* data, u32_t len)
{
	int ret = 0;

	ret = UART_read(fd, data, len);

	if (UART_ERROR == ret) {
		System_abort("UART_read failed!");
	}

	return ret;
}
