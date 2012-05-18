#ifndef _TYPES_
#define _TYPES_

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long size_t;

typedef char int8_t;
typedef short int16_t;
typedef long int32_t;

typedef _Bool bool;
#define true 1
#define false 0

/* Some definitions for registers memory mapping. All just resolve to volatile. This way, allowed usage can be seen in the typedefs. HW_WO: Write only, HW_RO: Read only, HW_RW: Read/Write, HW_RS: Reserved (do not access), HW_UU: Unused (padding etc.) */

typedef volatile uint32_t HW_WO;
typedef volatile uint32_t HW_RO;
typedef volatile uint32_t HW_RW;
typedef volatile uint32_t HW_RS;
typedef volatile uint32_t HW_UU;

#endif