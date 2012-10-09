#include "pressanykey/pressanykey.h"

#define X_PORT 0
#define X_ENABLE_PIN 6
#define X_DIR_PIN 2
#define X_STEP_PIN 8

#define Y_PORT 0
#define Y_ENABLE_PIN 9
#define Y_DIR_PIN 10	/* NEED TO SET FUNCTION */
#define Y_STEP_PIN 11	/* NEED TO SET FUNCTION */

#define Z_PORT 1
#define Z_ENABLE_PIN 5
#define Z_DIR_PIN 6
#define Z_STEP_PIN 7

#define W_PORT 2
#define W_ENABLE_PIN 0
#define W_DIR_PIN 1
#define W_STEP_PIN 2

#define LED_PORT 0
#define LED_PIN 7

#define SYSTICK_INTERVAL ((72000000/1000)-1) /* 1KHz */

uint32_t counter;

void main(void) {
	GPIO_SetDir(X_PORT, X_ENABLE_PIN, GPIO_Output);
	GPIO_WriteOutput(X_PORT, X_ENABLE_PIN, false);
	GPIO_SetDir(X_PORT, X_DIR_PIN, GPIO_Output);
	GPIO_WriteOutput(X_PORT, X_DIR_PIN, false);
	GPIO_SetDir(X_PORT, X_STEP_PIN, GPIO_Output);
	GPIO_WriteOutput(X_PORT, X_STEP_PIN, false);

	GPIO_SetDir(Y_PORT, Y_ENABLE_PIN, GPIO_Output);
	GPIO_WriteOutput(Y_PORT, Y_ENABLE_PIN, false);
	GPIO_SetDir(Y_PORT, Y_DIR_PIN, GPIO_Output);
	GPIO_WriteOutput(Y_PORT, Y_DIR_PIN, false);
	GPIO_SetDir(Y_PORT, Y_STEP_PIN, GPIO_Output);
	GPIO_WriteOutput(Y_PORT, Y_STEP_PIN, false);

	GPIO_SetDir(Z_PORT, Z_ENABLE_PIN, GPIO_Output);
	GPIO_WriteOutput(Z_PORT, Z_ENABLE_PIN, false);
	GPIO_SetDir(Z_PORT, Z_DIR_PIN, GPIO_Output);
	GPIO_WriteOutput(Z_PORT, Z_DIR_PIN, false);
	GPIO_SetDir(Z_PORT, Z_STEP_PIN, GPIO_Output);
	GPIO_WriteOutput(Z_PORT, Z_STEP_PIN, false);

	GPIO_SetDir(W_PORT, W_ENABLE_PIN, GPIO_Output);
	GPIO_WriteOutput(W_PORT, W_ENABLE_PIN, false);
	GPIO_SetDir(W_PORT, W_DIR_PIN, GPIO_Output);
	GPIO_WriteOutput(W_PORT, W_DIR_PIN, false);
	GPIO_SetDir(W_PORT, W_STEP_PIN, GPIO_Output);
	GPIO_WriteOutput(W_PORT, W_STEP_PIN, false);

	GPIO_SetDir(LED_PORT, LED_PIN, GPIO_Output);
	GPIO_WriteOutput(LED_PORT, LED_PIN, false);

	counter = 0;
	
	SYSCON_StartSystick(SYSTICK_INTERVAL);
}

void systick() {
	counter++;
	GPIO_WriteOutput(X_PORT, X_ENABLE_PIN, (counter/2000) & 1);
	GPIO_WriteOutput(X_PORT, X_DIR_PIN, (counter/1000) & 1);
	GPIO_WriteOutput(X_PORT, X_STEP_PIN, (counter/50) & 1);

	GPIO_WriteOutput(LED_PORT, LED_PIN, (counter/500) & 1);

}
