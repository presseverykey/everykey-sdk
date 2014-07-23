#include "everykey/everykey.h"
#include "midi.h"
#include "pwmaudio.h"
#include "synth.h"

#define LED_PORT 0
#define LED_PIN 7
#define KEY_PORT 0
#define KEY_PIN 1

uint8_t voice = 0;

void SynthTask() {
	int16_t val = Synthesizer_GetNextSample();
	PWMAudio_SetSample(val);

}

void HandleNote(uint8_t channel, uint8_t note, uint8_t velocity) {
	if (velocity) Synthesizer_PlayNote(voice, note, velocity);
	else Synthesizer_StopNote(voice, note);
}


int main(void) {
	every_gpio_set_dir(KEY_PORT, KEY_PIN, INPUT);
	EVERY_GPIO_SETHYSTERESIS(KEY_PORT, KEY_PIN, PULL_UP);

	every_gpio_set_dir(LED_PORT, LED_PIN, OUTPUT);
	every_gpio_write(LED_PORT, LED_PIN, 0);

	MIDI_Init(&HandleNote, &SynthTask);
	Synthesizer_Init();
	PWMAudio_Init();

	SYSCON_StartSystick(719999);	//100 Hz
}

void systick() {
	static bool wasDown = false;
	bool isDown = !every_gpio_read(KEY_PORT, KEY_PIN);
	if (wasDown != isDown) {
		if (isDown) {
			voice = (voice+1) & 0x3;
			static bool ledOn = false;
			ledOn = !ledOn;
			every_gpio_write(LED_PORT, LED_PIN, ledOn);
		}
		wasDown = isDown;
	}
}
