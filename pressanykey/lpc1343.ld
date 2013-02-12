/* Simple linker script for LPC1343 */

/* MEMORY describes the device's memory blocks and their addresses */

MEMORY { 
  flash (rx)  : ORIGIN = 0x00000000, LENGTH = 32K
  ram   (rwx) : ORIGIN = 0x10000000, LENGTH =  8K
}

_BOOTSTRAP_END_OF_TEXT = 0x1001ff0; /* Stack should start here */

/* SECTIONS lists where various parts data and code should go.
   Since they all just relate to the addressed defined above,
   porting code to other MCUs usually does not requre changes. */

SECTIONS {

    /* vector table must be located at beginning of flash,
     * so we assign it to a separate linker section.
     * It contains stack top address, start code and
     * interrupt handler pointers.  */
    vectors : {
        *(vectors)
    } >flash
 
    /* .text section contains code and readonly (const) globals */
    text : {   
        *(.text)
        *(.rodata)
	_BOOTSTRAP_END_OF_TEXT = .;     /* Remember position (=start of RAM mirror in FLASH) */
    } >flash
  
    /* data section contains read-write (non-const) globals and static variables.
     * The addresses are mapped to RAM (the code needs to write to it), but initial
     * values put to a mirror region in FLASH (because we need to have the values
     * at startup). Bootstrap code will copy the values from FLASH to RAM. 
     * Note that bootstrap code wants this section to follow the text section directly. */
    data : {
        _BOOTSTRAP_START_OF_DATA = .;   /* Remember position (=start of initialized RAM) */
        *(.data);
        _BOOTSTRAP_END_OF_DATA = .;     /* Remember position (=end of initialized RAM) */
    } >ram AT >flash

    /* bss section contains uninitialized data (globals without assignment).
     * We'll clear it in the bootstrap code. Note that bootstrap code wants this
     * section to follow the data section directly. */
    bss : {
        _BOOTSTRAP_START_OF_BSS = .;   /* Remember position (=start of uninitialized RAM) */
        *(.bss)
        _BOOTSTRAP_END_OF_BSS = .;     /* Remember position (=end of uninitialized RAM) */
    } > ram
}