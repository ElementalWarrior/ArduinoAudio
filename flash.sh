#!/bin/sh

dfu-programmer atmega16u2 erase
dfu-programmer atmega16u2 flash ArduinoAudio.hex
dfu-programmer atmega16u2 start
