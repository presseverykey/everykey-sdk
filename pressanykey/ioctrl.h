#include "memorymap.h"

typedef struct {
  unsigned FUNC:3;
  unsigned MODE:2;
  unsigned HYS:1;
  unsigned RESERVED0:4;
  unsigned OD:1;
  unsigned RESERVED1:21;
} IOCON__

typedef union {
  IOCON__ ctrl;
  HW_RW   raw;
} IOCON_
