Contents of this directory
==========================

This directory contains the following items:

- pressanykey: The pressanykey runtime for the LPC1343 / press any key
  board, linker script and makefile
- checksum: A tool to calculate and adjust the checksum of a firmware
  file
- several example projects (see bottom for descriptions)

In contrast to other runtimes, pressanykey does not link against
precompiled libraries. Instead, the runtime is included into projects as
source code. The projects compile in a few seconds and linking is static
anyway, so there's no need for precompiled libraries. You always have
the full source at your fingertips - less magic, more control.

The makefile is designed to compile and link all .c files in your
project directory and subirectories, so there's no need to modify it for
new source files. Just make sure you don't have unused .c files in your
project directory.

We're currently building up the firmware utilities, so not all
peripherals are working yet.

Setup build environment
=======================

- Install gcc arm-none-eabi toolchain (see below) 
- Go to checksum directory, build checksum tool (`make`)
- Move checksum tool or adjust $PATH so that it can be found

Alternatively, the examples can be built with clang/llvm (experimental -
more info coming).

Setting up a project
====================

Either copy an existing example directory (using cp -R) or do the
following steps to create a project directory from scratch:

- create a project directory
- create a main.c file (the name doesn't actually matter)
- to use pressanykey firmware library, #include
  "pressanykey/pressanykey.h"
- link, symlink or copy pressanykey directory to project directory
- link, symlink or copy makefile (inside pressanykey dir) to project
  directory
- link, symlink or copy lpc1343.ld (inside pressanykey dir) to project
  directory
- make (will compile and link all c files in project dir and subdirs)
- connect target board in USB programming mode
- make install (optionally copy the resulting .bin file manually)
- disconnect and re-plug target board

In most cases, symlinking (or hard-linking, if you prefer) the
pressanykey directory, makefile and linker script from your project
directory to the pressanykey directory is recommended. This way, the
project refers to the original, unaltered runtime library. 

The build system should work in OSX without modifications. For other
*NIX platforms, please adjust the target board path and unmount command
for the makefile ("install" target). 

Examples
========

This kit contains some examples. Here's a short overview, from basic to
advanced:

simple 
------

The embedded "Hello World": A blinking LED. Timing is done using busy
wait. One of the simplest possible applications. Shows writing digital
output pins.

lightbutton
-----------

The LED lights up when the button is pressed. Shows reading digital
input from pins and configuring pullup/pulldown resistors for inputs.

smooth
------

A smoothly pulsing LED. The LED is dimmed and animated using PWM (pulse
width modulation) in the systick timer. Shows how to use the systick
timer.

colorbars
---------

Generates a VGA signal with eight vertical color bars. To use this
example, a VGA connector and three resistors are required (schematics
can be found in the main.c file). The example shows inline assembly.

vgaterm
-------

Generates a VGA signal with text (800 x 600 @ 56Hz). To use this
example, a VGA connector and three resistors are required (schematics
identical to colorbars example). The example shows some advanced inline
assembly strategies.

Installing the compiler
=======================

The most common open source compiler toolchain in use for arm is the gnu
compiler toolchain. [Mentor
Graphics](http://www.mentor.com/embedded-software/codesourcery),
formerly CodeSourcery provide maintain the ARM version of GCC.

Linux
-----

Check your distributions package manager, it will probably contain a
package called something like `arm-none-eabi`.

Alternatively, you can download am installer directly from CodeSourcery.
[Here](http://www.codesourcery.com/sgpp/lite/arm/portal/package8736/public/arm-none-eabi/arm-2011.03-42-arm-none-eabi.bin)
[mirror](ftp://pr0n.club.muc.ccc.de/incoming/arm-2011.03-42-arm-none-eabi.bin).
You'll need to make the installer executable, using `chmod +x`, and then
just run it.


OSX
---

Both `MacPorts` and `HomeBrew` have the ARM gcc compiler as packages:

    $ sudo port install arm-none-eabi-gcc

or

    $ brew install arm-none-eabi-gcc


Windows
-------

We're currently looking into this.

Others
------

You can find some helpful hints on how to install the toolchain at the
[r0cket](http://r0ket.badge.events.ccc.de/build) website. We're
currently putting together a VM that contains all necessary tools
preinstalled.






