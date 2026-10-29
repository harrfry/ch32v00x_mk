# CH32V006 BIT-BANGED USB KEYBOARD EXPERIMENT

This is a 520 line fever dream that somehow crams a symmetric defer per-key, up to 16 layers, and a (somewhat) functional tap hold engine into a bitbanged rv003usb stack.

It currently survives on a 1ms on/9ms silence duty cycle to keep the RC oscillator from drifting into oblivion.

Half of it works, half of it conks out, the nop's are structural. PRs are welcome if you have a logic analyser and a more functional frontal lobe than I do.