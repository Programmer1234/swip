/*
 * cc.h
 *
 *  Created on: Feb 18, 2017
 *      Author: shaha
 */

#ifndef LWIP_2_0_1_SRC_INCLUDE_ARCH_CC_H_
#define LWIP_2_0_1_SRC_INCLUDE_ARCH_CC_H_

//#define BYTE_ORDER  BIG_ENDIAN

#define LWIP_NO_STDINT_H 1

/* Define generic types used in lwIP */
typedef unsigned char  u8_t;
typedef char     	   s8_t;
typedef unsigned short u16_t;
typedef short    	   s16_t;
typedef unsigned int   u32_t;
typedef int            s32_t;
typedef unsigned int uintptr_t;
typedef uintptr_t mem_ptr_t;

/* Compiler hints for packing structures */
#define PACK_STRUCT_FIELD(x)    x
#define PACK_STRUCT_STRUCT  __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

/***
typedef uint8_t     u8_t;
typedef int8_t      s8_t;
typedef uint16_t    u16_t;
typedef int16_t     s16_t;
typedef uint32_t    u32_t;
typedef int32_t     s32_t;
typedef uintptr_t   mem_ptr_t;
*/
typedef int sys_prot_t;


#endif /* LWIP_2_0_1_SRC_INCLUDE_ARCH_CC_H_ */
