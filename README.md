Contents of this directory
==========================

This directory contains the following items:

- pressanykey: The pressanykey runtime for the LPC1343 / Anykey0x.de
  board, linker script and makefile
- checksum: A tool to calculate and adjust the checksum of a firmware
  file
- several example projects (see below for descriptions)

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

- Install gcc arm-none-eabi toolchain (see [below](#installing-the-compiler)) 
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
- run `make` (will compile and link all c files in project dir and subdirs)
- connect target board in USB programming mode
- run `make mac-install` (optionally copy the resulting .bin file manually)
- disconnect and re-plug target board

In most cases, symlinking (or hard-linking, if you prefer) the
pressanykey directory, makefile and linker script from your project
directory to the pressanykey directory is recommended. This way, the
project refers to the original, unaltered runtime library. 

The build system should work in OSX without modifications. For other
*NIX platforms, please adjust the target board path and unmount command
for the makefile ("mac-install" target).

Linux' FAT drivers apparently interfere with copying, you need to transfer
the files directly onto the block device (after unmounting the USB mass
storage device in case it got mounted automatically by udev):

    dd if=firmware.bin of=/dev/<wherever the ANYKEY appears> seek=4
    
In case your Linux distribution automatically mount the `/dev/sdb<whatever>`
device (typically to `/media/No\ CRP`) you'll need to umount the drive.

Alternatively, you can use the `simpleflash` python script from the
r0ket project:

https://raw.github.com/r0ket/r0ket/master/tools/bootloader/simpleflash

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

usbraw
------

This example uses the software USB stack to implement a raw USB device. The device does not have any functionality other than the required standard USB functionality for USB enumeration.

usbkeyboard
-----------

Implements a simple USB HID keyboard. The example includes a two-step abstraction: hid.c implements a HID device based on the USB software stack. keyboard.c builds a keyboard on top of the HID device functionality. The keyboard can write text and the LED will act as a caps lock key (note that not all OSes synchronize caps lock among multiple keyboards). This example may be useful as a base for implementing your own HID devices.

nerdpursuit
-----------
A HID-keyboard implementation of Nerd Pursuit - see https://github.com/Nerds/NerdPursuit. Play it on a standard *NIX shell.



Installing the compiler
=======================

Please see [Setting up the Toolchain](https://github.com/anykey0xde/anykey-sdk/wiki/Setting-up-the-Toolchain) on our Wiki.








