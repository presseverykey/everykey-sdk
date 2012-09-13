#include "pressanykey/pressanykey.h"
#include "typing.h"

#define LED_PORT 0
#define LED_PIN  7

#define KEY_PORT 1
#define KEY_PIN 4

// systick timer interval.
// systick interrupt is triggered everytime this counter reaches 0
// s. Usermanual §17.7
// results in 10ms SYSTICK interval

# define SYSTICK_CNTR 0x000AFC7F


void main (void) {
  volatile int i;
  GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
  GPIO_SetDir(KEY_PORT, KEY_PIN , GPIO_Input);

  GPIO_SETPULL(KEY_PORT, KEY_PIN, IOCON_IO_PULL_UP);
  init_keyboard();
  for (i=500000; i!=0; --i) {}
  GPIO_WriteOutput(LED_PORT, LED_PIN, true);
  SYSCON_StartSystick(SYSTICK_CNTR);
}

void systick(void) {
  static bool     key_state = false;
//  static uint32_t count     = 0x00;

  key_state = GPIO_ReadInput(KEY_PORT, KEY_PIN);

  //GPIO_WriteOutput(0,7,key_state);

  //if ((count != 0) && (0 == (count % 100))) {
  if (!key_state) {
    type("Hello world!");
    //type("\x17\x17\x17\xff");
  }

   // GPIO_WriteOutput(LED_PORT, LED_PIN, key_state);
    //do_nerd_sm(key_state, count++);

  //count++;
}

