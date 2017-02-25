/*
 * sys_arch.c
 *
 *  Created on: Feb 25, 2017
 *      Author: shaha
 */
#include <lwip/opt.h>
#include <lwip/arch.h>
#include <lwip/stats.h>
#include <lwip/debug.h>
#include <lwip/sys.h>

#include <ti/sysbios/knl/Clock.h>


u32_t sys_now(void)
{
	u32_t time1 = Clock_getTicks();
	return time1;
}
