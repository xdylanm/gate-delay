# Assembly Guide

## Populate Back Side SMT Components

1. Schottky diodes D2, D3, D6, D7
2. Ferrite beads (or 10R): R20, R21
3. BJTs Q1-Q3
4. SMT LED D1
5. ICs U1-U3
6. SMT capacitors C2, C1, C5-C8
7. SMT resistors R1-R19

## Populate Front Side THT Components

1. IDC header J6
2. Electrolytic capacitors C3 & C4
3. Programming header J5
4. Mono jacks J1-J4
5. Potentiometers RV1 & RV2

Populate red 3mm LEDs D4 & D5 with face plate mounted. Leads need to be formed to a 90 degree bend; watch the orientation.

## Programming and Test

Open the firmware project with PlatformIO and connect a USBISP to the programming header on the PCB (check the orientation relative to the ground pin). Flash the firmware.

Test by sending gate pulses to the input. The LEDs will visually indicate the timing of the delayed pulses, or an oscilloscope can be used to check shorter delays.

## BOM

[Download (.csv)](assets/bom.csv)

{%include-markdown "assets/bom.md"%}