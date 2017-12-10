/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the AudioOutput demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "ArduinoAudio.h"

/** LUFA Audio Class driver interface configuration and state information. This structure is
 *  passed to all Audio Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_Audio_Device_t Speaker_Audio_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber   = INTERFACE_ID_AudioControl,
				.StreamingInterfaceNumber = INTERFACE_ID_AudioOutStream,
				.DataOUTEndpoint          =
					{
						.Address          = AUDIO_STREAM_OUT_EPADDR,
						.Size             = AUDIO_STREAM_OUT_EPSIZE,
						.Banks            = 2,
					}
			},
	};

 USB_ClassInfo_Audio_Device_t Mic_Audio_Interface =
 	{
 		.Config =
 			{
 				.ControlInterfaceNumber   = INTERFACE_ID_AudioControl,
 				.StreamingInterfaceNumber = INTERFACE_ID_AudioInStream,
 				.DataINEndpoint          =
 					{
 						.Address          = AUDIO_STREAM_IN_EPADDR,
 						.Size             = AUDIO_STREAM_IN_EPSIZE,
 						.Banks            = 2,
 					},
 			},
 	};

/** Current audio sampling frequency of the streaming audio endpoint. */
static uint32_t CurrentAudioSampleFrequency = 8000;
static uint32_t baud = 500000;

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	GlobalInterruptEnable();

	for (;;)
	{
		Audio_Device_USBTask(&Speaker_Audio_Interface);
		Audio_Device_USBTask(&Mic_Audio_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#endif

	/* Hardware Initialization */
	Serial_Init(baud, false);
	LEDs_Init();
	USB_Init();

	/* Sample reload timer initialization */
	TIMSK0  = (1 << OCIE0A);
	OCR0A   = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);
	TCCR0A  = (1 << WGM01);  // CTC mode
	TCCR0B  = (1 << CS01);   // Fcpu/8 speed
}

/** ISR to handle sending the sample over USART to the atmega328 */
ISR(TIMER0_COMPA_vect, ISR_BLOCK)
{
	uint8_t PrevEndpoint = Endpoint_GetCurrentEndpoint();

	/* Check that the USB bus is ready for the next sample to read */
	if (Audio_Device_IsSampleReceived(&Speaker_Audio_Interface))
	{
		/* Retrieve the signed 16-bit left and right audio samples, convert to 8-bit */
		int8_t LeftSample_8Bit  = (Audio_Device_ReadSample16(&Speaker_Audio_Interface) >> 8);
		int8_t RightSample_8Bit = (Audio_Device_ReadSample16(&Speaker_Audio_Interface) >> 8);

		/* Mix the two channels together to produce a mono, 8-bit sample */
		int8_t MixedSample_8Bit = (((int16_t)LeftSample_8Bit + (int16_t)RightSample_8Bit) >> 1);
 
		if(UCSR1A & (1<<UDRE1)) { 
			//turn on LED 1 when we actually send a sample over USART for debug purposes
			LEDs_TurnOnLEDs(LEDS_LED1);
			UDR1 = MixedSample_8Bit ^ (1 << 7);
		}
	}

	// Endpoint_SelectEndpoint(Mic_Audio_Interface.Config.DataINEndpoint.Address);
	
	// LEDs_SetAllLEDs(!(Speaker_Audio_Interface.State.InterfaceEnabled) ? LEDMASK_USB_READY : LEDS_NO_LEDS);
	/* Check that the USB bus is ready for the next sample to write */
	// if (Audio_Device_IsReadyForNextSample(&Mic_Audio_Interface))
	// {
	// 		LEDs_TurnOnLEDs(LEDS_LED1);
	// 	int16_t AudioSample;
	// 	static uint8_t SquareWaveSampleCount;
	// 	static int16_t CurrentWaveValue;

	// 	/* In test tone mode, generate a square wave at 1/256 of the sample rate */
	// 	if (SquareWaveSampleCount++ == 0xFF)
	// 		CurrentWaveValue ^= 0x8000;
			
	// 	AudioSample = CurrentWaveValue;
	// 	Audio_Device_WriteSample16(&Mic_Audio_Interface, 0);
	// }

	Endpoint_SelectEndpoint(PrevEndpoint);
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{

	/* Sample reload timer initialization */
	TIMSK0  = (1 << OCIE0A);
	OCR0A   = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);
	TCCR0A  = (1 << WGM01);  // CTC mode
	TCCR0B  = (1 << CS01);   // Fcpu/8 speed
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	/* Stop the sample reload timer */
	TCCR0B = 0;
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= Audio_Device_ConfigureEndpoints(&Speaker_Audio_Interface);
	ConfigSuccess &= Audio_Device_ConfigureEndpoints(&Mic_Audio_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDS_LED2 : LEDS_NO_LEDS);
}
void EVENT_USB_Device_UnhandledControlRequest(void) {
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	Audio_Device_ProcessControlRequest(&Speaker_Audio_Interface);
	Audio_Device_ProcessControlRequest(&Mic_Audio_Interface);
}

/** Audio class driver callback for the setting and retrieval of streaming endpoint properties. This callback must be implemented
 *  in the user application to handle property manipulations on streaming audio endpoints.
 *
 *  When the DataLength parameter is NULL, this callback should only indicate whether the specified operation is valid for
 *  the given endpoint index, and should return as fast as possible. When non-NULL, this value may be altered for GET operations
 *  to indicate the size of the retrieved data.
 *
 *  \note The length of the retrieved data stored into the Data buffer on GET operations should not exceed the initial value
 *        of the \c DataLength parameter.
 *
 *  \param[in,out] AudioInterfaceInfo  Pointer to a structure containing an Audio Class configuration and state.
 *  \param[in]     EndpointProperty    Property of the endpoint to get or set, a value from Audio_ClassRequests_t.
 *  \param[in]     EndpointAddress     Address of the streaming endpoint whose property is being referenced.
 *  \param[in]     EndpointControl     Parameter of the endpoint to get or set, a value from Audio_EndpointControls_t.
 *  \param[in,out] DataLength          For SET operations, the length of the parameter data to set. For GET operations, the maximum
 *                                     length of the retrieved data. When NULL, the function should return whether the given property
 *                                     and parameter is valid for the requested endpoint without reading or modifying the Data buffer.
 *  \param[in,out] Data                Pointer to a location where the parameter data is stored for SET operations, or where
 *                                     the retrieved data is to be stored for GET operations.
 *
 *  \return Boolean \c true if the property get/set was successful, \c false otherwise
 */
bool CALLBACK_Audio_Device_GetSetEndpointProperty(USB_ClassInfo_Audio_Device_t* const AudioInterfaceInfo,
                                                  const uint8_t EndpointProperty,
                                                  const uint8_t EndpointAddress,
                                                  const uint8_t EndpointControl,
                                                  uint16_t* const DataLength,
                                                  uint8_t* Data)
{
	/* Check the requested endpoint to see if a supported endpoint is being manipulated */
	if (&AudioInterfaceInfo == &Speaker_Audio_Interface
	  && EndpointAddress == Speaker_Audio_Interface.Config.DataOUTEndpoint.Address)
	{
		/* Check the requested control to see if a supported control is being manipulated */
		if (EndpointControl == AUDIO_EPCONTROL_SamplingFreq)
		{
			switch (EndpointProperty)
			{
				case AUDIO_REQ_SetCurrent:
					/* Check if we are just testing for a valid property, or actually adjusting it */
					if (DataLength != NULL)
					{
						/* Set the new sampling frequency to the value given by the host */
						CurrentAudioSampleFrequency = (((uint32_t)Data[2] << 16) | ((uint32_t)Data[1] << 8) | (uint32_t)Data[0]);
  
						/* Adjust sample reload timer to the new frequency */
						OCR0A = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);
					}
  
					return true;
				case AUDIO_REQ_GetCurrent:
					/* Check if we are just testing for a valid property, or actually reading it */
					if (DataLength != NULL)
					{
						*DataLength = 3;
  
						Data[2] = (CurrentAudioSampleFrequency >> 16);
						Data[1] = (CurrentAudioSampleFrequency >> 8);
						Data[0] = (CurrentAudioSampleFrequency &  0xFF);
					}
  
					return true;
			}
		}
	}
  //
	// /* Check the requested endpoint to see if a supported endpoint is being manipulated */
	if (&AudioInterfaceInfo == &Mic_Audio_Interface
	  && EndpointAddress == Mic_Audio_Interface.Config.DataINEndpoint.Address)
	{
		/* Check the requested control to see if a supported control is being manipulated */
		if (EndpointControl == AUDIO_EPCONTROL_SamplingFreq)
		{
			switch (EndpointProperty)
			{
				case AUDIO_REQ_SetCurrent:
					/* Check if we are just testing for a valid property, or actually adjusting it */
					if (DataLength != NULL)
					{
						/* Set the new sampling frequency to the value given by the host */
						CurrentAudioSampleFrequency = (((uint32_t)Data[2] << 16) | ((uint32_t)Data[1] << 8) | (uint32_t)Data[0]);
  
						/* Adjust sample reload timer to the new frequency */
						OCR0A = ((F_CPU / 8 / CurrentAudioSampleFrequency) - 1);
					}
  
					return true;
				case AUDIO_REQ_GetCurrent:
					/* Check if we are just testing for a valid property, or actually reading it */
					if (DataLength != NULL)
					{
						*DataLength = 3;
  
						Data[2] = (CurrentAudioSampleFrequency >> 16);
						Data[1] = (CurrentAudioSampleFrequency >> 8);
						Data[0] = (CurrentAudioSampleFrequency &  0xFF);
					}
  
					return true;
			}
		}
	}

	return false;
}

/** Audio class driver callback for the setting and retrieval of streaming interface properties. This callback must be implemented
 *  in the user application to handle property manipulations on streaming audio interfaces.
 *
 *  When the DataLength parameter is NULL, this callback should only indicate whether the specified operation is valid for
 *  the given entity and should return as fast as possible. When non-NULL, this value may be altered for GET operations
 *  to indicate the size of the retrieved data.
 *
 *  \note The length of the retrieved data stored into the Data buffer on GET operations should not exceed the initial value
 *        of the \c DataLength parameter.
 *
 *  \param[in,out] AudioInterfaceInfo  Pointer to a structure containing an Audio Class configuration and state.
 *  \param[in]     Property            Property of the interface to get or set, a value from Audio_ClassRequests_t.
 *  \param[in]     EntityAddress       Address of the audio entity whose property is being referenced.
 *  \param[in]     Parameter           Parameter of the entity to get or set, specific to each type of entity (see USB Audio specification).
 *  \param[in,out] DataLength          For SET operations, the length of the parameter data to set. For GET operations, the maximum
 *                                     length of the retrieved data. When NULL, the function should return whether the given property
 *                                     and parameter is valid for the requested endpoint without reading or modifying the Data buffer.
 *  \param[in,out] Data                Pointer to a location where the parameter data is stored for SET operations, or where
 *                                     the retrieved data is to be stored for GET operations.
 *
 *  \return Boolean \c true if the property GET/SET was successful, \c false otherwise
 */
bool CALLBACK_Audio_Device_GetSetInterfaceProperty(USB_ClassInfo_Audio_Device_t* const AudioInterfaceInfo,
                                                   const uint8_t Property,
                                                   const uint8_t EntityAddress,
                                                   const uint16_t Parameter,
                                                   uint16_t* const DataLength,
                                                   uint8_t* Data)
{
	/* No audio interface entities in the device descriptor, thus no properties to get or set. */
	// Audio_ClassRequests_t
	return false;
}
