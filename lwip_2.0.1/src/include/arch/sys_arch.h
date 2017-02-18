/*
 * sys_arch.h
 *
 *  Created on: Feb 18, 2017
 *      Author: shaha
 */

#ifndef LWIP_2_0_1_SRC_INCLUDE_ARCH_SYS_ARCH_H_
#define LWIP_2_0_1_SRC_INCLUDE_ARCH_SYS_ARCH_H_

#include <lwip/opt.h>


struct _sys_mut {
  void *mut;
};
typedef struct _sys_mut sys_mutex_t;


#ifndef MAX_QUEUE_ENTRIES
#define MAX_QUEUE_ENTRIES 100
#endif
struct lwip_mbox {
  void* sem;
  void* q_mem[MAX_QUEUE_ENTRIES];
  u32_t head, tail;
};
typedef struct lwip_mbox sys_mbox_t;

#define SYS_MBOX_NULL   NULL
#define SYS_SEM_NULL    NULL

//typedef void * sys_prot_t;
typedef void * sys_sem_t;
//typedef void * sys_mbox_t;
typedef void * sys_thread_t;

#endif /* LWIP_2_0_1_SRC_INCLUDE_ARCH_SYS_ARCH_H_ */
