# ArduinoAudio
A project to utilize [LUFA](http://www.fourwalledcubicle.com/LUFA.php) in conjuntion with an Arduino UNO r3 to create a USB audio device interface. 

It works by passing audio data from the computer to the pins on the arduino. Used LUFA to have the Arduino bootloader (ATMEGA16U2) register as a USB audio device, and pass the audio data to the ATMEGA328.

In the end the project was successful, when connecting headphones to the output pins, audio could be heard and understood, however was not exactly high fidelity audio.
