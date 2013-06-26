#include "anykey/anykey.h"
#include "typing.h"
#include "sm.h"

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
  init_game();
  GPIO_WriteOutput(LED_PORT, LED_PIN, false);
  for (i=5000000; i!=0; --i) {}
  SYSCON_StartSystick(SYSTICK_CNTR);
}

void systick(void) {
  static bool     key_state = false;
  static uint32_t count     = 0x00;

  key_state = GPIO_ReadInput(KEY_PORT, KEY_PIN);

  do_nerd_sm(!key_state, count);

  count++;
  //if ((count % 100)==0) {
  //  type("date\n");
  //}
}

