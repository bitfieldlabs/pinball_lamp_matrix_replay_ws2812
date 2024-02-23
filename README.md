## Pinball Lamp Matrix Replay
This is a small project to replay recorded pinball lamp matrix patterns to addressable LED strands.
This can be useful e.g. for lighting decorative pinball playfields.

### Setup
This project assumes following setup:
* A raspberry pico board
* 64 WS2812 LEDs representing the pinball matrix lamps 11 - 88
* A power supply which can support all LEDs (up to 3A for 64 WS2812)
* WS2812 data line connected to GPIO 4 of the raspberry pico board

### Pattern recording
The lamp matrix pattern recording was done using a patched version of pinmame which outputs the required data in a format that can be pasted directly into the code.
There are also some python tools to analyze and visualize the recordings. See the [pattern_recording_tools](https://github.com/bitfieldlabs/pinball_lamp_matrix_replay_ws2812/tree/master/pattern_recording_tools) folder.

### Configuration
There are some varibales in the _Setup_ section of main.cpp which can be used for configuration. Not much at the moment.

### Pre-compiled binaries for Raspberry Pico boards
Some pre-compiled binaries can be found in the [u2f](https://github.com/bitfieldlabs/pinball_lamp_matrix_replay_ws2812/tree/master/u2f) folder.

Only WS2812 LEDs are supported by these builds. The data line of the LEDs must be connected to **GPIO 4** of the Raspberry Pico board.

**Make sure to provide enough power on the 5V line to the WS2812 LEDs!** 64 LEDs can consume up to 3A at full brightness!
