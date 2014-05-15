/** CNC setup */

#ifndef _CONFIG_
#define _CONFIG_

#include "everykey/everykey.h"
#include "cnctypes.h"

/* pin mapping:
 
 D-SUB pin	LPC Port/pin
 1/C0		1_5	
 2/D0		0_8
 3/D1		0_9
 4/D2		0_1
 5/D3		2_0
 6/D4		1_7
 7/D5		1_6
 8/D6		0_11
 9/D7		0_6
 14/LF		2_11
 16/NIN		2_2
 17/NSP		2_3
 
 X axis: 300mm (Head along the portal)
 Y axis: 400mm (Portal along the table)
 
 Mapping of controller
 Function	D-SUB pin
 X en		14
 X dir		17
 X step		16
 Y en
 Y dir		5
 Y step		4
 Z en
 Z dir		7
 Z step		6
 W en
 W dir
 W step
 Spindle	1
 
 Other enable ports are 8,9,?
 
*/

#define LED_PORT 0
#define LED_PIN 7

extern const Axis axes[NUM_AXES];

#define SPINDLE_PORT 1
#define SPINDLE_PIN 5

#define HEARTBEAT_HZ 10000
#define SPINDLE_PWM_RESOLUTION 100

#define ENABLE_IS_LOW_ACTIVE true
#define SPINDLE_IS_LOW_ACTIVE true

/** size of command queue */
#define CQ_LENGTH 10

/** Number of bits in positions considered to be below one phyiscal step.
 Positions have a fixed point representation, with SUBSTEP_BITS fractional bits.
 Microstepping is considered to be one step. For example, 8 bits means that changes
 in the lower 8 bits of positions do not cause output changes to the steppers.
 This leaves us space for dealing with speed, for example. */
#define SUBSTEP_BITS 8

#endif

