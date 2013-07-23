Short and Sweat
===============

The Anykey SDK is an SDK for the NXP LPC1343 processor, featuring access
to all "basic" features (linking and startup, interrupt & timer
handling, GPIO, i2c, SPI und UART) as well as implementations for a
number of USB Classes (HID, CDC, Audio).

Have a look at the
[Examples](https://github.com/anykey0xde/anykey-sdk/tree/master/examples)
to get a feel for the libary.

Have a look at the [Tutorial](https://github.com/anykey0xde/tutorial) if
you are new to embedded programming.

Read the rest of this file and when you're ready to get started, [install
the development tools](https://github.com/anykey0xde/tutorial/tree/master/lesson1) ...

Contents of this directory
==========================

This directory contains the following items:

- `anykey`: The Anykey runtime for the LPC1343 / Anykey0x.de
  board, linker script and makefile
- `anykey_usb` : USB firmware, this is quite large so we keep it seperated.
- checksum: A tool to calculate and adjust the checksum of a firmware
  file
- several example projects (see below for descriptions)

In contrast to other runtimes, the Anykey SDK does not link against
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

Build environment
=================

For developing and building Anykey software, you will need:

- a computer and a text editor of your choice
- gcc arm-none-eabi toolchain  
- gnu make
- checksum tool (go to checksum directory, build checksum tool (`make`))

For compiling the checksum tool, you might also need a gcc for your development computer.

Make sure that the toolchain's tools, make and checksum are found by your terminal (adjust $PATH so that they can be found).

There's an in-depth guide for setting up the initial development environment [here](https://github.com/anykey0xde/tutorial/tree/master/lesson1).

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
- connect target board in USB programming mode (Anykey V1: Short-circuit the programming pins while connecting the USB cable, Anykey V2: Hold the application button while pressing the reset button)
- run `make mac-install` (optionally copy the resulting .bin file manually)
- Reset the board (Anykey V1: disconnect and re-plug, Anykey V2: press the reset button)

In most cases, symlinking (or hard-linking, if you prefer) the
`anykey` directory, makefile and linker script from your project
directory to the pressanykey directory is recommended. This way, the
project refers to the original, unaltered runtime library. 

The build system should work in OSX without modifications. For other
*NIX platforms, please adjust the target board path and unmount command
for the makefile ("mac-install" target).

Linux' FAT drivers apparently interfere with copying, you need to transfer
the files directly onto the block device (after unmounting the USB mass
storage device in case it got mounted automatically by udev, this will 
typically be mounted under `/media/NO\ CRP`):

    dd if=firmware.bin of=/dev/<wherever the ANYKEY appears> seek=4
    
Alternatively, you can use the `simpleflash` python script from the
r0ket project:

https://raw.github.com/r0ket/r0ket/master/tools/bootloader/simpleflash

Examples
========

If you are new to the Anykey-SDK or to (embedded) programming, have a
look at the
[README](https://github.com/anykey0xde/anykey-sdk/tree/master/examples)
in the `examples` directory. it describes all the examples included with
the SDK, which in turn provide examples for all the functionality.

Also have a look at the
[Tutorial](https://github.com/anykey0xde/tutorial), which provides a
more in depth introduction.







