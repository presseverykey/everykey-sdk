#include "nvic.h"

void NVIC_SetMask(volatile uint32_t* base, NVIC_INTERRUPT_INDEX interrupt) {
	uint8_t reg = interrupt / 32;
	uint32_t mask = 1 << (interrupt % 32);
	base[reg] = mask;
}

bool NVIC_GetMask(volatile uint32_t* base, NVIC_INTERRUPT_INDEX interrupt) {
	uint8_t reg = interrupt / 32;
	uint32_t mask = 1 << (interrupt % 32);
	return (base[reg] & mask) ? true : false;
}

void NVIC_EnableInterrupt(NVIC_INTERRUPT_INDEX interrupt) {
	NVIC_SetMask(&(NVIC->ISER0),interrupt);
}

void NVIC_DisableInterrupt(NVIC_INTERRUPT_INDEX interrupt) {
	NVIC_SetMask(&(NVIC->ICER0),interrupt);
}

bool NVIC_IsInterruptEnabled(NVIC_INTERRUPT_INDEX interrupt) {
	return NVIC_GetMask(&(NVIC->ISER0),interrupt);
}

void NVIC_SetInterruptPending(NVIC_INTERRUPT_INDEX interrupt) {
	NVIC_SetMask(&(NVIC->ISPR0),interrupt);
}

void NVIC_ClearInterruptPending(NVIC_INTERRUPT_INDEX interrupt) {
	NVIC_SetMask(&(NVIC->ICPR0),interrupt);
}

bool NVIC_IsInterruptPending(NVIC_INTERRUPT_INDEX interrupt) {
	return NVIC_GetMask(&(NVIC->ISPR0),interrupt);
}

bool NVIC_IsInterruptActive(NVIC_INTERRUPT_INDEX interrupt) {
	return NVIC_GetMask(&(NVIC->IABR0),interrupt);
}

void NVIC_SetInterruptPriority(NVIC_INTERRUPT_INDEX interrupt, uint8_t prio) {
	uint8_t reg = interrupt / 4;
	uint8_t shift = 8 * (interrupt % 4);
	volatile uint32_t* base = &(NVIC->IPR0);
	uint32_t val = base[reg];
	val &= ~(0xff << shift);
	val |= prio << shift;
	base[reg] = val;
}

uint8_t NVIC_GetInterruptPriority(NVIC_INTERRUPT_INDEX interrupt) {
	uint8_t reg = interrupt / 4;
	uint8_t shift = 8 * (interrupt % 4);
	volatile uint32_t* base = &(NVIC->IPR0);
	return (base[reg] >> shift) & 0xff;
}

void NVIC_TriggerInterrupt(NVIC_INTERRUPT_INDEX interrupt) {
	NVIC->STIR = interrupt;
}

void CopyDefaultVectorTable(void* to) {
	memcpy(to, 0, sizeof(VECTOR_TABLE));
}

void UseVectorTable(void* base) {
	// TODO ************
}

