#include "timer.h"
#include "memorymap.h"

void Timer_Enable(TimerId timer, bool on) {
	uint32_t mask = SYSCON_SYSAHBCLKCTRL_CT16B0 << timer;
	if (on) SYSCON->SYSAHBCLKCTRL |= mask;
	else SYSCON->SYSAHBCLKCTRL &=  ~mask;
}

uint32_t Timer_GetValue(TimerId timer) {
	return TIMER[timer].TC;
}

void Timer_SetPrescale(TimerId timer, uint32_t prescale) {
	TIMER[timer].PR = prescale;
}

void Timer_SetMatchValue(TimerId timer, uint8_t matchIdx, uint32_t value) {
	TIMER[timer].MR[matchIdx] = value;
}

void Timer_SetMatchBehaviour(TimerId timer, uint8_t matchIdx, uint8_t behaviour) {
	uint8_t shift = 3 * matchIdx;
	uint32_t clear = 0x07 << shift;
	uint32_t set = ((uint32_t)behaviour) << shift;
	TIMER[timer].MCR = (TIMER[timer].MCR & (~clear)) | set;
}

void Timer_Start(TimerId timer) {
	TIMER[timer].TCR = TIMER_CEN;
}

void Timer_Stop(TimerId timer) {
	TIMER[timer].TCR = 0;
}

void Timer_Reset(TimerId timer) {
	uint32_t running = TIMER[timer].TCR & TIMER_CEN;
	TIMER[timer].TCR = TIMER_CRESET;
	TIMER[timer].TCR = running;
}

uint32_t Timer_GetInterruptMask(TimerId timer) {
	return TIMER[timer].IR;
}

void Timer_ClearInterruptMask(TimerId timer, uint32_t mask) {
	TIMER[timer].IR = mask;
}

void Timer_EnablePWM(TimerId timer, uint8_t pwmIdx, bool enable) {
	if (enable)	TIMER[timer].PWMC |= 1<<pwmIdx;
	else TIMER[timer].PWMC &= ~(1<<pwmIdx);
}

void Timer_SetCaptureMode(TimerId timer, bool captureRising, bool captureFalling, bool triggerInterrupt) {
	TIMER[timer].CCR = (captureRising ? CAP0RE : 0) | (captureFalling ? CAP0FE : 0) | (triggerInterrupt ? CAP0I : 0);
}

/** returns the capture value
	@param timer timer to read (CT16B0, CT16B1, CT32B0 or CT32B1)
	@return the current capture value */
uint32_t Timer_GetCaptureValue(TimerId timer) {
	return TIMER[timer].CR0;
}


