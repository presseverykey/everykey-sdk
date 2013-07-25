# Examples ...

This directory contains a number of samples to help you get started
programming the Anykey (and with embedded programming in general).

Each of the example directories contain a file called `main.c`, and a
bunch of other stuff, ignore the other stuff and concentrate on the
`main.c` file.

The order below is a suggestion of how you should get started...

## `blink`

Start here! This is the Hello World of embedded programming, a routine
to make the Anykey's LED blink. This example is heavily commented, but
consists of hardly any code :) Be sure to work through the comments if
you're not particularly experienced in this sort of thing.

## `blink_pro`

This is a more "real life" example of blinking an LED. It also
introduces the concept of "timers". It's also heavily commented.

## `lightbutton`
## `lightbutton_pro`

The blink examples only dealt with output, the lightbutton examples show
how to use Anykey's input facilities (the two buttons). They are
well commented and structured similarly to the `blink` examples, the
first one shows the easiest possible way determine whether a button has
been pressed, the second example is a bit more sosphisticated.

##`smooth`
##`smoothtimer`

Homebrew PWM

## `pwm`
## `pwm_scale`
## `pwm_rgb`

PWM examples using the built in PWM and timer mechanism

## `simple_adc`

simple example demonstrating the ADC

## `cdcleetifier`

Example of how to use the CDC libs to communicate with the Anykey via a
serial port.

## `i2c`

Example demonstrating the i2c lib

## `spiflash`

Example demonstrating use of SPI

## `uart`

Example for using a serial port.

## `usbkeyboard`
## `usbaudio`
## `usbraw`

Further USB profiles.


## `nerdpursuit`

"Practical" use of usb Keyboard as a quiz game.

## `colorbars`
## `vgaterm`

Very well documented example bitbanging a VGA signal. Rather advanced
and not for the faint of heart.


## `cnccontrol`
## `pintest`

Some stuff we haven't gotten around to cleaning up.

## test.sh

A small test script that makes sure all the examples compile.
