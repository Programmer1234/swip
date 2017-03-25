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

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Swi.h>
#include <xdc/runtime/Error.h>


/* Board Header files */
#include "Board.h"

/* lwIP core includes */
#include "lwip/opt.h"

#define TASKSTACKSIZE   1024
#include "lwip_main.h"

Task_Struct task1Struct;
Char task1Stack[TASKSTACKSIZE];



#if LWIP_HTTPD_CLIENT && LWIP_TCP
Task_Struct task2Struct;
Char task2Stack[TASKSTACKSIZE];

PIN_Config buttonPinTable[] = {
    Board_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
    Board_BUTTON1  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
    PIN_TERMINATE
};

static Swi_Handle swi0;

static Semaphore_Handle sem;

static PIN_State buttonPinState;

static PIN_Handle buttonPinHandle;

void buttonCallbackFxn(PIN_Handle handle, PIN_Id pinId) {
	CPUdelay(8000*50);
	switch (pinId) {
		case Board_BUTTON0:
			Swi_post(swi0);
			break;

		case Board_BUTTON1:
			break;

		default:
			/* Do nothing */
			break;
	}
}


void swiFunc0(UArg param0, UArg param1)
{
	System_printf("Posting semaphore\n"); System_flush();
	Semaphore_post(sem);
}


void reporter_task(UArg arg0, UArg arg1)
{
	System_printf("http reporter task started\n"); System_flush();
	while (1)
	{
		Semaphore_pend(sem, BIOS_WAIT_FOREVER);
		System_printf("\nreporter_task woke_up\n"); System_flush();
		http_send_request("button1-pressed", 15);
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
    Error_Block eb;
    Swi_Params swiParams0;

    buttonPinHandle = PIN_open(&buttonPinState, buttonPinTable);
    if(!buttonPinHandle) {
        System_abort("Error initializing button pins\n");
    }

    /* init semaphore */
    sem = Semaphore_create(0, NULL, &eb);
    if (sem == NULL) {
    	System_abort("Semaphore create failed");
    }

    /* init SWI handling led */
    Error_init(&eb);
    Swi_Params_init(&swiParams0);
    swiParams0.priority = 5;
    swi0 = Swi_create(swiFunc0, &swiParams0, &eb);
    if (swi0 == NULL) {Swi_create
     System_abort("Swi0 create failed");
    }

    /* Construct HTTP request reporter Task Thread */
    Task_Params_init(&taskParams2);
    taskParams2.stackSize = TASKSTACKSIZE;
    taskParams2.stack = &task2Stack;
    taskParams2.instance->name = "http_reporter";
    taskParams2.priority = 2; //higer priority
    Task_construct(&task2Struct, (Task_FuncPtr)reporter_task, &taskParams2, NULL);

    /* Setup callback for button pins */
    if (PIN_registerIntCb(buttonPinHandle, &buttonCallbackFxn) != 0) {
        System_abort("Error registering button callback function");
    }
	#endif

    System_printf("Starting the lwip porting...\n");

    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
