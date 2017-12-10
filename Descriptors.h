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
 *  Header file for Descriptors.c.
 */

#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

	/* Includes: */
		#include <LUFA/Drivers/USB/USB.h>

		#include <avr/pgmspace.h>

		#include "Config/AppConfig.h"

	/* Macros: */
		/** Endpoint address of the Audio isochronous streaming data IN endpoint. */
		#define AUDIO_STREAM_OUT_EPADDR           (ENDPOINT_DIR_OUT | 3)
		#define AUDIO_STREAM_IN_EPADDR           (ENDPOINT_DIR_IN | 4)

		/** Endpoint size in bytes of the Audio isochronous streaming data endpoint. */
		#define AUDIO_STREAM_OUT_EPSIZE           32
		#define AUDIO_STREAM_IN_EPSIZE           32

				typedef struct
				{
					USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */
					uint8_t                 Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors,
					                                  *   a value from the \ref Audio_CSInterface_AS_SubTypes_t enum.
					                                  */

					uint16_t                ACSpecification; /**< Binary Coded Decimal value, indicating the supported Audio Class specification version.
					                                          *
					                                          *   \see \ref VERSION_BCD() utility macro.
					                                          */
					uint16_t                TotalLength; /**< Total length of the Audio class-specific descriptors, including this descriptor. */

					uint8_t                 InCollection; /**< Total number of Audio Streaming interfaces linked to this Audio Control interface (must be 1). */
					uint8_t                 InterfaceNumber1; /**< Interface number of the associated Audio Streaming interface. */
					uint8_t                 InterfaceNumber2; /**< Interface number of the associated Audio Streaming interface. */
				} ATTR_PACKED USB_Audio_Descriptor_Interface_AC_2_t;

				
	/* Type Defines: */
		/** Type define for the device configuration descriptor structure. This must be defined in the
		 *  application code, as the configuration descriptor contains several sub-descriptors which
		 *  vary between devices, and which describe the device's usage to the host.
		 */
		typedef struct
		{
			USB_Descriptor_Configuration_Header_t     Config;
			// USB_Descriptor_Interface_Association_t		Audio_IAD;

			// Audio Control Interface
			USB_Descriptor_Interface_t                Audio_ControlInterface;
			USB_Audio_Descriptor_Interface_AC_2_t       Audio_ControlInterface_SPC;
			USB_Audio_Descriptor_InputTerminal_t      Audio_InputTerminal;
			USB_Audio_Descriptor_OutputTerminal_t     Audio_OutputTerminal;
			USB_Audio_Descriptor_InputTerminal_t      Audio_InputTerminal2;
			USB_Audio_Descriptor_OutputTerminal_t     Audio_OutputTerminal2;

			// Audio Streaming Interface
			USB_Descriptor_Interface_t                Audio_Extra_StreamInterface;
			USB_Descriptor_Interface_t                Audio_Out_StreamInterface;
			USB_Audio_Descriptor_Interface_AS_t       Audio_Out_StreamInterface_SPC;
			USB_Audio_Descriptor_Format_t             Audio_AudioFormat;
			USB_Audio_SampleFreq_t                    Audio_AudioFormatSampleRates[1];
			USB_Audio_Descriptor_StreamEndpoint_Std_t Audio_Out_StreamEndpoint;
			USB_Audio_Descriptor_StreamEndpoint_Spc_t Audio_Out_StreamEndpoint_SPC;
			USB_Descriptor_Interface_t                Audio_Extra_StreamInterface2;
			USB_Descriptor_Interface_t                Audio_In_StreamInterface;
			USB_Audio_Descriptor_Interface_AS_t       Audio_In_StreamInterface_SPC;
			USB_Audio_Descriptor_Format_t             Audio_AudioFormat2;
			USB_Audio_SampleFreq_t                    Audio_AudioFormatSampleRates2[1];
			USB_Audio_Descriptor_StreamEndpoint_Std_t Audio_In_StreamEndpoint;
			USB_Audio_Descriptor_StreamEndpoint_Spc_t Audio_In_StreamEndpoint_SPC;
		} USB_Descriptor_Configuration_t;

		/** Enum for the device interface descriptor IDs within the device. Each interface descriptor
		 *  should have a unique ID index associated with it, which can be used to refer to the
		 *  interface from other descriptors.
		 */
		enum InterfaceDescriptors_t
		{
			INTERFACE_ID_AudioControl = 0, /**< Audio control interface descriptor ID */
			INTERFACE_ID_AudioOutStream  = 1, /**< Audio stream interface descriptor ID */
			INTERFACE_ID_AudioInStream  = 2, /**< Audio stream interface descriptor ID */
		};

		/** Enum for the device string descriptor IDs within the device. Each string descriptor should
		 *  have a unique ID index associated with it, which can be used to refer to the string from
		 *  other descriptors.
		 */
		enum StringDescriptors_t
		{
			STRING_ID_Language     = 0, /**< Supported Languages string descriptor ID (must be zero) */
			STRING_ID_Manufacturer = 1, /**< Manufacturer string ID */
			STRING_ID_Product      = 2, /**< Product string ID */
		};

	/* Function Prototypes: */
		uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
		                                    const uint16_t wIndex,
		                                    const void** const DescriptorAddress)
		                                    ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
