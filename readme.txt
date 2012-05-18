Contents of this directory
--------------------------

This directory contains the following items:

- pressanykey: The pressanykey runtime for the LPC1343 / press any key board, linker script and makefile
- checksum: A tool to calculate and adjust the checksum of a firmware file
- several example projects (see bottom for descriptions)

In contrast to other runtimes, pressanykey does not link against precompiled libraries. Instead, the runtime is included into projects as source code. The projects compile in a few seconds and linking is static anyway, so there's no need for precompiled libraries. You always have the full source at your fingertips - less magic, more control.

The makefile is designed to compile and link all .c files in your project directory and subirectories, so there's no need to modify it for new source files. Just make sure you don't have unused .c files in your project directory.

We're currently building up the firmware utilities, so not all peripherals are working yet.

Setup build environment
-----------------------

- Install gcc arm-none-eabi toolchain 
- Go to checksum directory, build checksum tool (make)
- Move checksum tool or adjust $PATH so that it can be found

Alternatively, the examples can be built with clang/llvm (experimental - more info coming).

Setup a project
---------------

Either copy an existing example directory (using cp -R) or do the following steps to create a project directory from scratch:

- create a project directory
- create a main.c file (the name actually doesn't matter)
- to use pressanykey firmware library, #include "pressanykey/pressanykey.h"
- link, symlink or copy pressanykey directory to project directory
- link, symlink or copy makefile (inside pressanykey dir) to project directory
- link, symlink or copy lpc1343.ld (inside pressanykey dir) to project directory
- make (will compile and link all c files in project dir and subdirs)
- Connect target board in USB programming mode
- make install (optionally copy the resulting .bin file manually)
- Disconnect and re-plug target board

In most cases, symlinking (or hard-linking, if you prefer) the pressanykey directory, makefile and linker script from your project directory to the pressanykey directory is recommended. This way, the project refers to the original, unaltered runtime library. Making copies them to the project allows you to modify them without altering the default runtime.

The build system should work in OSX without modifications. For other *NIX platforms, please adjust the target board path and unmount command for the makefile ("install" target). 

Examples
--------

This kit contains some examples. Here's a short overview, from basic to advanced:

simple:
The embedded "Hello World": A blinking LED. Timing is done using busy wait. One of the simplest possible applications. Shows writing digital output pins.

lightbutton:
The LED lights up when the button is pressed. Shows reading digital input from pins and configuring pullup/pulldown resistors for inputs.

smooth:
A smoothly pulsing LED. The LED is dimmed and animated using PWM (pulse width modulation) in the systick timer. Shows how to use the systick timer.

colorbars:
Generates a VGA signal with eight vertical color bars. To use this example, a VGA connector and three resistors are required (schematics can be found in the main.c file). The example shows inline assembly.

vgaterm:
Generates a VGA signal with text (800 x 600 @ 56Hz). To use this example, a VGA connector and three resistors are required (schematics identical to colorbars example). The example shows some advanced inline assembly strategies.



