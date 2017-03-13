/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== maic.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/UART.h>
#include <ti/sysbios/knl/Clock.h>

/* Board Header files */
#include "Board.h"

/* lwIP core includes */
#include "lwip/opt.h"

#define TASKSTACKSIZE   1024
#include "lwip_main.h"

Task_Struct task1Struct;
Char task1Stack[TASKSTACKSIZE];

#if LWIP_HTTPD_CLIENT && LWIP_TCP
// 30 sec
#define HTTP_REQ_INTERVAL   1000*30
Task_Struct task2Struct;
Char task2Stack[TASKSTACKSIZE];

void reporter_task(UArg arg0, UArg arg1)
{
	while(1){
		Task_sleep(HTTP_REQ_INTERVAL); // 20 sec
		System_printf("\nreporter_task woke_up\n"); System_flush();
		http_send_request("keep-alive1", 11);

		Task_sleep(HTTP_REQ_INTERVAL);
		System_printf("\nreporter_task woke_up\n"); System_flush();
		http_send_request("keep-alive2", 11);
	}
}
#endif


void initLwip(UArg arg0, UArg arg1)
{
	main_lwip();
}

/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams1;
    Task_Params taskParams2;

    /* Call board init functions */
    Board_initGeneral();
    Board_initUART();
    /* Construct lwip_init Task Thread */
    Task_Params_init(&taskParams1);
    taskParams1.stackSize = TASKSTACKSIZE;
    taskParams1.stack = &task1Stack;
    taskParams1.instance->name = "lwipInit";
    taskParams1.priority = 1;
    Task_construct(&task1Struct, (Task_FuncPtr)initLwip, &taskParams1, NULL);

	#if LWIP_HTTPD_CLIENT && LWIP_TCP
    /* Construct HTTP request reporter Task Thread */
    Task_Params_init(&taskParams2);
    taskParams2.stackSize = TASKSTACKSIZE;
    taskParams2.stack = &task2Stack;
    taskParams2.instance->name = "http_reporter";
    taskParams2.priority = 2; //higer priority
    Task_construct(&task2Struct, (Task_FuncPtr)reporter_task, &taskParams2, NULL);
	#endif

    System_printf("Starting the lwip porting...\n");

    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
