# MIDISynth

This example illustrates software MIDI parsing and PWM audio generation. It
implements a very simple four-voice synthesizer with different waveforms. The
application assumes MIDI input at P2_2 and outputs audio at P0_10. The button
can be used to switch waveforms.

The MIDI IN circuit should consist of a MIDI port with a suitable optoisolator
and the accompanying pullup resistor.

As a simple Audio out circuit, we recommend two stages: First, a voltage divider
(e.g. 1K attached to the out pin, then 1K to 3.3V and 1K to GND) and second, an
OP AMP in a low pass configuration (-3dB around 15KHz, just experiment a bit).
The output can then drive a Line input or mid-to-high-impedance headphones
(depending on your OP AMP).

Audio PWM uses 16 Bit timer 0 (CT16B0), set at full speed with a reload value of
512, resulting in 9 bits resolution with a PWM frequency around 140kHz, well
abobe the audible range (even if your lowpass is not steep enough to filter
everything, it will be likely sound ok). MIDI parsing is handled by 16 Bit timer
1 (CT16B1), set to 31250Hz (MIDI bit clock frequency). The same timer is used
to update the audio samples.

The resulting audio quality does not meet audiophile standards, but is good
enough for simple audio tasks and to have lots of fun. Just connect a MIDI
keyboard, headphones and start playing.
