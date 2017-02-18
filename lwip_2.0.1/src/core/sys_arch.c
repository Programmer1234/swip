/*
 * sys_arch.c
 *
 *  Created on: Feb 18, 2017
 *      Author: shaha
 */
#include <lwip/opt.h>
#include <arch/sys_arch.h>
#include <ti/sysbios/knl/Clock.h>

sys_prot_t sys_arch_protect(void)
{
  return 0;
}

void sys_arch_unprotect(sys_prot_t pval)
{
  return 0;
}

u32_t sys_now(void)
{
  return Clock_getTicks();
}
