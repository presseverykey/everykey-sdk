
#ifndef _PS2_SCANCODES_
#define _PS2_SCANCODES_

#include "anykey/anykey.h"

/*unless otherwise noted, the KEYCODE_ constants are the USB keyboard usages. */

#define KEYCODE_A 0x04
#define KEYCODE_B 0x05
#define KEYCODE_C 0x06
#define KEYCODE_D 0x07
#define KEYCODE_E 0x08
#define KEYCODE_F 0x09
#define KEYCODE_G 0x0A
#define KEYCODE_H 0x0B
#define KEYCODE_I 0x0C
#define KEYCODE_J 0x0D
#define KEYCODE_K 0x0E
#define KEYCODE_L 0x0F
#define KEYCODE_M 0x10
#define KEYCODE_N 0x11
#define KEYCODE_O 0x12
#define KEYCODE_P 0x13
#define KEYCODE_Q 0x14
#define KEYCODE_R 0x15
#define KEYCODE_S 0x16
#define KEYCODE_T 0x17
#define KEYCODE_U 0x18
#define KEYCODE_V 0x19
#define KEYCODE_W 0x1A
#define KEYCODE_X 0x1B
#define KEYCODE_Y 0x1C
#define KEYCODE_Z 0x1D
#define KEYCODE_1 0x1E
#define KEYCODE_2 0x1F
#define KEYCODE_3 0x20
#define KEYCODE_4 0x21
#define KEYCODE_5 0x22
#define KEYCODE_6 0x23
#define KEYCODE_7 0x24
#define KEYCODE_8 0x25
#define KEYCODE_9 0x26
#define KEYCODE_0 0x27
#define KEYCODE_RETURN 0x28
#define KEYCODE_ESCAPE 0x29
#define KEYCODE_BACKSPACE 0x2A
#define KEYCODE_TAB 0x2B
#define KEYCODE_SPACE 0x2C
#define KEYCODE_MINUS 0x2D
#define KEYCODE_EQUAL 0x2E
#define KEYCODE_SQUAREBRACKET_OPEN 0x2F
#define KEYCODE_SQUAREBRACKET_CLOSE 0x30
#define KEYCODE_BACKSLASH 0x31
#define KEYCODE_SEMICOLON 0x33
#define KEYCODE_APOSTROPH 0x34
#define KEYCODE_ACCENTGRAVE 0x35
#define KEYCODE_COLON 0x36
#define KEYCODE_PERIOD 0x37
#define KEYCODE_SLASH 0x38
#define KEYCODE_CAPSLOCK 0x39
#define KEYCODE_F1 0x3A
#define KEYCODE_F2 0x3B
#define KEYCODE_F3 0x3C
#define KEYCODE_F4 0x3D
#define KEYCODE_F5 0x3E
#define KEYCODE_F6 0x3F
#define KEYCODE_F7 0x40
#define KEYCODE_F8 0x41
#define KEYCODE_F9 0x42
#define KEYCODE_F10 0x43
#define KEYCODE_F11 0x44
#define KEYCODE_F12 0x45

#define KEYCODE_PRINTSCREEN 0x46
#define KEYCODE_SCROLLLOCK 0x47
#define KEYCODE_PAUSE 0x48
#define KEYCODE_INSERT 0x49
#define KEYCODE_HOME 0x4A
#define KEYCODE_PAGEUP 0x4B
#define KEYCODE_DELETEFORWARD 0x4C
#define KEYCODE_END 0x4D
#define KEYCODE_PAGEDOWN 0x4E
#define KEYCODE_RIGHTARROW 0x4F
#define KEYCODE_LEFTARROW 0x50
#define KEYCODE_DOWNARROW 0x51
#define KEYCODE_UPARROW 0x52

#define KEYCODE_KEYPAD_NUMLOCK 0x53
#define KEYCODE_KEYPAD_DIV 0x54
#define KEYCODE_KEYPAD_MULT 0x55
#define KEYCODE_KEYPAD_MINUS 0x56
#define KEYCODE_KEYPAD_PLUS 0x57
#define KEYCODE_KEYPAD_ENTER 0x58
#define KEYCODE_KEYPAD_1 0x59
#define KEYCODE_KEYPAD_2 0x5A
#define KEYCODE_KEYPAD_3 0x5B
#define KEYCODE_KEYPAD_4 0x5C
#define KEYCODE_KEYPAD_5 0x5D
#define KEYCODE_KEYPAD_6 0x5E
#define KEYCODE_KEYPAD_7 0x5F
#define KEYCODE_KEYPAD_8 0x60
#define KEYCODE_KEYPAD_9 0x61
#define KEYCODE_KEYPAD_0 0x62
#define KEYCODE_KEYPAD_PERIOD 0x63

#define KEYCODE_F13_WAKE 0x68
#define KEYCODE_F14_SLEEP 0x69
#define KEYCODE_F15_POWER 0x6a

#define KEYCODE_LEFTCONTROL 0xE0
#define KEYCODE_LEFTSHIFT 0xE1
#define KEYCODE_LEFTALT 0xE2
#define KEYCODE_LEFTGUI 0xE3
#define KEYCODE_RIGHTCONTROL 0xE4
#define KEYCODE_RIGHTSHIFT 0xE5
#define KEYCODE_RIGHTALT 0xE6
#define KEYCODE_RIGHTGUI 0xE7

/* The following key codes are not defined in the USB keyboard page. They either
 have no correspondance at all or they are defined in other usage pages. We
 want to be able to transmit all keys in one byte, so they diverge from the USB spec. All those constants are prefixed wirh KEYCODE_MEDIA_ and have usage values starting at 0xEA, which are not defined in the USB spec, so we don't collide. */

#define KEYCODE_MEDIA_WWWSEARCH 0xEA
#define KEYCODE_MEDIA_WWWFAVORITES 0xEB
#define KEYCODE_MEDIA_WWWREFRESH 0xEC
#define KEYCODE_MEDIA_WWWSTOP 0xED
#define KEYCODE_MEDIA_WWWFORWARD 0xEE
#define KEYCODE_MEDIA_WWWBACK 0xEF
#define KEYCODE_MEDIA_WWWHOME 0xF0
#define KEYCODE_MEDIA_PREVIOUSTRACK 0xF1
#define KEYCODE_MEDIA_VOLUMEDOWN 0xF2
#define KEYCODE_MEDIA_MUTE 0xF3
#define KEYCODE_MEDIA_CALCULATOR 0xF4
#define KEYCODE_MEDIA_APPS 0xF5
#define KEYCODE_MEDIA_VOLUMEUP 0xF6
#define KEYCODE_MEDIA_PLAYPAUSE 0xF7
#define KEYCODE_MEDIA_STOP 0xF8
#define KEYCODE_MEDIA_MYCOMPUTER 0xF9
#define KEYCODE_MEDIA_EMAIL 0xFA
#define KEYCODE_MEDIA_NEXTTRACK 0xFB
#define KEYCODE_MEDIA_MEDIASELECT 0xFC

/* codes are sorted in ascending length, ascending numeric order */
extern const uint8_t ps2ScanCodesMode2[];

#endif
