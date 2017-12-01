#!/bin/sh

sudo dfu-programmer atmega16u2 erase
sudo dfu-programmer atmega16u2 flash AudioOutput.hex
sudo dfu-programmer atmega16u2 start
