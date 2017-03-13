/*
 * sio.c
 *
 *  Created on: Feb 25, 2017
 *      Author: shaha
 */


#include <lwip/sio.h>
#include <ti/drivers/UART.h>
#include "Board.h"

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <xdc/runtime/Error.h>

#ifndef SIO_COMPORT_SPEED
#define SIO_COMPORT_SPEED 115200
#endif

UART_Handle g_uart_handle;
Mailbox_Handle mbox;


#define MAX_NUM_RX_BYTES    32   // Maximum RX bytes to receive in one go
u8_t rxBuf[MAX_NUM_RX_BYTES];   // Receive buffer
uint32_t wantedRxBytes;            // Number of bytes received so far
// Callback function
static void readCallback(UART_Handle handle, void *rxBuf, size_t size)
{
	size_t i;
    // Copy bytes from RX buffer to mailbox
    for(i = 0; i < size; i++)
    	Mailbox_post(mbox, &rxBuf[i], BIOS_NO_WAIT);
    // Start another read, with size the same as it was during first call to
    // UART_read()
    UART_read(handle, rxBuf, wantedRxBytes);
}


/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
	UART_Params uartParams;

	//init MAILBOX

    Mailbox_Params mboxParams;
    Error_Block eb;

    Mailbox_Params_init(&mboxParams);
    Error_init(&eb);
    mbox = Mailbox_create(1, 50, &mboxParams, &eb);
    if (mbox == NULL) {
	    System_printf("taskFxn(): %s\n", Error_getMsg(&eb) );
		System_abort("Mailbox create failed");
		return NULL;
	}

    //init UART

	UART_Params_init(&uartParams);

	uartParams.readMode = UART_MODE_CALLBACK;
    uartParams.readCallback  = readCallback;
	uartParams.readDataMode = UART_DATA_BINARY;
	uartParams.readReturnMode = UART_RETURN_FULL;

	uartParams.writeMode = UART_MODE_BLOCKING;
	uartParams.writeDataMode = UART_DATA_BINARY;

	uartParams.baudRate = SIO_COMPORT_SPEED;

	g_uart_handle = UART_open(Board_UART, &uartParams);

	if(!g_uart_handle) {
		System_abort("UART_open failed!");
		return NULL;
	}

    wantedRxBytes = sizeof(u8_t);
    int rxBytes = UART_read(g_uart_handle, rxBuf, wantedRxBytes);

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
		//System_abort("UART_write failed!");
		System_printf("UART_write failed!"); System_flush();
		return 0;
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
	int i;

	for (i=0; i<len;i++)
	{
		if ( !Mailbox_pend(mbox, &data[i], BIOS_NO_WAIT) ){
			break;
		}
	}

	return i;
}
