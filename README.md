# df-firmware

Directionizer is a student project by Mason Ahner, Jared Beller, and John Fiorini for the spring 2021 session of ECE 2804 at Virginia Tech. The "directionizer" is a handheld device for locating a 1 kHz audio beacon. This repository contains software for identifying the relative direction of the beacon using the device's three onboard microphones.

## Building

This program is designed to run on either an [Arduino Uno](https://www.arduino.cc/en/Main/ArduinoBoardUno) or the [df-pcb processing board](https://github.com/directionizer/df-pcb). Look at the [df-pcb schematics](https://github.com/directionizer/df-pcb/tree/master/schematic) for wiring directions. [Arduino CLI](https://arduino.github.io/arduino-cli/latest/) or [Arduino IDE](https://www.arduino.cc/en/software/) is required for building this project. Using Arduino CLI, compilation can be done with:
```bash
arduino-cli compile --fqbn arduino:avr:uno df-firmware
```

## Licensing

Copyright (C) 2021  Mason Ahner, Jared Beller, and John Fiorini

This program is released under the [GNU General Public License, Version 3](https://www.gnu.org/licenses/gpl-3.0.txt) or later. See [LICENSE](LICENSE) for more information.
