/* Very simple linker script for LPC1343 */

MEMORY { 
  rom (rx)  : ORIGIN = 0x0, LENGTH = 0x8000
  ram (rwx) : ORIGIN = 0x10000000, LENGTH = 0x2000
}

SECTIONS {
  . = 0x0;
  .text : {
    *(vectors)
    *(.text)
    *(.rodata)
  }
  . = 0x10000000;
  .data : {
    *(.data)
  }
  .bss : {
    *(.bss)
  }
}