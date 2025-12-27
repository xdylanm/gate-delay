# Design Overview

## Functional Choices

There are a few behaviours in the gate delay that could be interesting:

* Exponential or linear delay control: the YuSynth design chooses exponential, which makes sense to me.
* CV control over gate delay.
* Maintain the length of the gate pulse: neither the YuSynth nor Dopefer A-162 have this option. The A-162 enables manual control over the ouput pulse length.
* Maintain the amplitude of the gate pulse (assuming it's constant). The YuSynth module includes a slew out.
* Alternate trigger delay mode or "convert to trigger" input: convert an incoming signal to a fixed-duration pulse.

## Design Options

There are a few different approaches that I've considered:

1.  Pure analog delay. This circuit delays the gate pulse in time while
    maintaining its length with exponential CV (and manual offset) for
    the delay time. It might be possible to match the gate amplitude by
    incorporating a S/H circuit, but that seems like it's going too
    far. More details are given in the section [Analog Gate Delay](design/gate_delay_theory.md)
2.  Use a tiny microcontroller. This is the smart way to do it. This is
    elaborated in the section [IC Gate Delay](design/ic_gate_delay.md)
3.  Use a 555. To maintain the pulse duration, two 555's are required,
    and the circuit ends up looking quite similar to the pure analog
    verison.

The pure analog delay is an interesting design exercise. With the IC delay, delayed sequences of pulses are possible. Additionally, by going to a single circuit, it's more straightforward to add improved signal conditioning for the CV input (attenuation and offset) and a trigger output while staying in a 4HP form. 