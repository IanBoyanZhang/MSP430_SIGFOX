//*****************************************************************************
//! @file       host_cmd.c
//! @brief      AT command interface for UART
//
//  Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/

/**************************************************************************//**
 * @addtogroup UART
 * @{
 ******************************************************************************/

/******************************************************************************
 * INCLUDES
 */
#include "host_cmd.h"
#include "uart_drv.h"
#include "device_config.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "radio.h"
#include "timer.h"
#include "../sigfox_library_api/sigfox.h"
#include "../sigfox_library_api/sigfox_types.h"

/******************************************************************************
 * LOCAL VARIABLES
 */
/* global pointer to storage buffers */
extern unsigned char rf_payload[];
extern unsigned long id;
extern unsigned char key[];
extern unsigned long TxFrequency;
extern unsigned long RxFrequency;
extern unsigned char TxRep;

extern unsigned long* TxCF;
extern unsigned long* RxCF;
extern unsigned char* TxRepeat;

signed short burst_count = 10;		// Default value of number of bursts in test mode
signed short channel = -1;			// Default - channel hopping active
signed short rx_tout = 0;			// Default 0 : no timeout always in RX
unsigned short sequence_number = 0; // Sequence number for rx test

/* internal prototypes */
//unsigned char dataToString(unsigned char *data, char *str, unsigned char length);
unsigned char stringToData(char *str, unsigned char *data, unsigned char length);
unsigned char hexToByte(char *hex);
void byteToHex(unsigned char byte, char *hex);

/* data configuration */
#define CMD_AT_OFFSET             0x00
#define CMD_CMD_OFFSET            0x03
#define CMD_DATA_OFFSET           0x06

#define	CR	0x0D
#define LF  0x0A

/******************************************************************************
 * FUNCTIONS
 */
/**********************************************************************//**
 * @brief  	This function detects and parses the AT command
 *
 * @param  	host_cmd 	is pointer to the command in buffer
 * @param	length 		is the length of host command
 *
 * @return 	Host command status ::
 * 			\li \b	HOST_CMD_FOUND if command detected
 * 			\li \b	HOST_CMD_ERROR if invalid command
 * 			\li \b	HOST_CMD_SUCCESS if command executed correctly
 **************************************************************************/
host_cmd_status_t
parseHostCmd(unsigned char *host_cmd, unsigned char length)
{
	SFX_error_t err;
	host_cmd_status_t ret_cmd;

	unsigned char ul_msg[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char dl_msg[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	char tmp_str[8];
	char ch_id = 0;

	unsigned char ii;                // general purpose counter
	unsigned char cmd_index;         // index to start of command

	unsigned char msg_mask = 0;		 // mask for the uplink message in command
	unsigned char mask_buff = 0;	 // buffer to store previous msg_mask value

	ret_cmd = HOST_CMD_NOT_FOUND;

	// find "AT$" starting token, this algorithm is restricted to upper case only
	ii = 0;
	while(ret_cmd == HOST_CMD_NOT_FOUND)
	{
		// Look for command starting with "AT$"
		if( (host_cmd[ii] == 'A') && (host_cmd[ii+1] == 'T') && (host_cmd[ii+2] == '$') )
		{
			ret_cmd = HOST_CMD_FOUND;
			cmd_index = ii;
		}
		// Move to next location if the command not found
		if(ii++ < length)
		{
			ii++;
		}
		// If command not found
		else
		{
			ret_cmd = HOST_CMD_ERROR;
		}
	}

	// New Line Feed
	uartPutChar(LF);

	// If the command is found,
	if(ret_cmd == HOST_CMD_FOUND)
	{
		// Parse the next token to figure out what the command is
		switch(host_cmd[cmd_index+CMD_CMD_OFFSET])
		{
		case 'S':
			switch(host_cmd[cmd_index+CMD_CMD_OFFSET+1])
			{
			case 'B':
				// Send status bit command
				if ((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '=')
				{
					if(host_cmd[cmd_index+CMD_DATA_OFFSET] == '1')
					{
						if(host_cmd[cmd_index+CMD_DATA_OFFSET+1] == 0x0D)
						{
							// Send bit without ack
							if (SfxSendBit(1, dl_msg, FALSE) == SFX_ERR_NONE)
							{
								// Terminate command with {OK<CR><LF>}
								uartPutStr("OK", 2);
								uartPutChar(CR);
								uartPutChar(LF);
								ret_cmd = HOST_CMD_SUCCESS;
							}
						}
						else if((host_cmd[cmd_index+CMD_DATA_OFFSET+1] == ',') && (host_cmd[cmd_index+CMD_DATA_OFFSET+2] == '1') && (host_cmd[cmd_index+CMD_DATA_OFFSET+3] == 0x0D))
						{
							// Terminate command with {OK<CR><LF>}
							uartPutStr("OK", 2);
							uartPutChar(CR);
							uartPutChar(LF);

							// Send bit with ack
							if (SfxSendBit(1, dl_msg, TRUE) == SFX_ERR_NONE)
							{
								char dl_str[16];
								dataToString((unsigned char*) dl_msg, dl_str, 16);

								// Print downlink message {+RX=<dl_msg><CR><LF>}
								uartPutStr("+RX=",4);
								uartPutStr(dl_str,16);
								uartPutChar(CR);
								uartPutChar(LF);

								ret_cmd = HOST_CMD_SUCCESS;
							}

							// Print {+RX END<CR><LF>}
							uartPutStr("+RX END", 7);
							uartPutChar(CR);
							uartPutChar(LF);
						}
						else
						{
							ret_cmd = HOST_CMD_ERROR;
						}
					}
					else if(host_cmd[cmd_index+CMD_DATA_OFFSET] == '0')
					{
						if(host_cmd[cmd_index+CMD_DATA_OFFSET+1] == 0x0D)
						{
							// Send bit without ack
							if (SfxSendBit(0, dl_msg, FALSE) == SFX_ERR_NONE)
							{
								// Terminate command with {OK<CR><LF>}
								uartPutStr("OK", 2);
								uartPutChar(CR);
								uartPutChar(LF);
								ret_cmd = HOST_CMD_SUCCESS;
							}
						}
						else if((host_cmd[cmd_index+CMD_DATA_OFFSET+1] == ',') && (host_cmd[cmd_index+CMD_DATA_OFFSET+2] == '1') && (host_cmd[cmd_index+CMD_DATA_OFFSET+3] == 0x0D))
						{
							// Terminate command with {OK<CR><LF>}
							uartPutStr("OK", 2);
							uartPutChar(CR);
							uartPutChar(LF);

							// Send bit with ack
							if (SfxSendBit(0, dl_msg, TRUE) == SFX_ERR_NONE)
							{
								char dl_str[16];
								dataToString((unsigned char*) dl_msg, dl_str, 16);

								// Print downlink message {+RX=<dl_msg><CR><LF>}
								uartPutStr("+RX=",4);
								uartPutStr(dl_str,16);
								uartPutChar(CR);
								uartPutChar(LF);

								ret_cmd = HOST_CMD_SUCCESS;
							}
							// Print {+RX END<CR><LF>}
							uartPutStr("+RX END", 7);
							uartPutChar(CR);
							uartPutChar(LF);
						}
						else
						{
							ret_cmd = HOST_CMD_ERROR;
						}
					}
					else
					{
						ret_cmd = HOST_CMD_ERROR;
					}
				}
				else
				{
					ret_cmd = HOST_CMD_ERROR;
				}
				break;
			case 'F':
				// Send frame command
				if ((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '=')
				{
					// Parse command for uplink message
					msg_mask = 0;
					while (((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != ',') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != 0x0D))
					{
						msg_mask++;
					}

					unsigned char ul_size = (msg_mask-1)/2;

					unsigned char tmp_msg[24] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
					unsigned char j;

					// Extract uplink message as a string
					for (j = 0; j<msg_mask; j++)
					{
						tmp_msg[j] = host_cmd[cmd_index+CMD_CMD_OFFSET+3+j];
					}

					// String to Hex convert
					stringToData((char *)tmp_msg, ul_msg, msg_mask);

					// Send a frame with downlink request
					if (((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) == ',') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask+1]) == '1') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask+2]) == 0x0D))
					{
						// Terminate command with {OK<CR><LF>}
						uartPutStr("OK", 2);
						uartPutChar(CR);
						uartPutChar(LF);

						//if(SfxSendFrame(ul_msg, sizeof(ul_msg), dl_msg, TRUE) == SFX_ERR_NONE)
						if(SfxSendFrame(ul_msg, ul_size, dl_msg, TRUE) == SFX_ERR_NONE)
						{
							char dl_str[16];
							dataToString((unsigned char*) dl_msg, dl_str, 16);

							// Print downlink message {+RX=<dl_msg><CR><LF>}
							uartPutStr("+RX=",4);
							uartPutStr(dl_str,16);
							uartPutChar(CR);
							uartPutChar(LF);

							ret_cmd = HOST_CMD_SUCCESS;
						}

						// Print {+RX END<CR><LF>}
						uartPutStr("+RX END", 7);
						uartPutChar(CR);
						uartPutChar(LF);
					}

					// Send uplink only frame
					else if ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) == 0x0D)
					{
						//if(SfxSendFrame(ul_msg, sizeof(ul_msg), NULL, NULL) == SFX_ERR_NONE)
						if(SfxSendFrame(ul_msg, ul_size, NULL, NULL) == SFX_ERR_NONE)
						{
							// Terminate command with {OK<CR><LF>}
							uartPutStr("OK", 2);
							uartPutChar(CR);
							uartPutChar(LF);

							ret_cmd = HOST_CMD_SUCCESS;
						}
					}
					else
					{
						ret_cmd = HOST_CMD_ERROR;
					}
				}
				else
				{
					ret_cmd = HOST_CMD_ERROR;
				}
				break;
			case 'T':
				// UL test mode command
				if ((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '=')
				{
					msg_mask = 0;

					// Parse command for frame count
					if(((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+3]) == '-') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+4]) == '1') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+5]) == ','))
					{
						burst_count = -1;
						msg_mask += 3;
					}
					else
					{
						mask_buff = msg_mask;
						msg_mask++;
						while ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != ',')
						{
							msg_mask++;
						}

						unsigned char j;
						unsigned long tmp_burst_count = 0;

						// Extract frame count value
						for (j = mask_buff; j<(msg_mask-1); j++)
						{
							tmp_burst_count = (tmp_burst_count*10) + (host_cmd[cmd_index+CMD_CMD_OFFSET+3+j]-48);
						}

						burst_count = (signed short) tmp_burst_count;
					}

					// Parse command for channel
					if(((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+3]) == '-') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+4]) == '1') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+5]) == 0x0D))
					{
						channel = -1;
						msg_mask += 3;
					}
					else
					{
						mask_buff = msg_mask;
						msg_mask++;
						while ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != 0x0D)
						{
							msg_mask++;
						}

						unsigned char j;
						unsigned long tmp_channel = 0;

						// Extract frame count value
						for (j = mask_buff; j<(msg_mask-1); j++)
						{
							tmp_channel = (tmp_channel*10) + (host_cmd[cmd_index+CMD_CMD_OFFSET+3+j]-48);
						}

						channel = (signed short) tmp_channel;
					}

					// Execute test mode command
					SfxTxTestMode(burst_count, channel);

					// Terminate command with {OK<CR><LF>}
					uartPutStr("OK", 2);
					uartPutChar(CR);
					uartPutChar(LF);

					ret_cmd = HOST_CMD_SUCCESS;
				}
				else
				{
					ret_cmd = HOST_CMD_ERROR;
				}
				break;
			case 'R':
				// DL test mode command
				if ((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '=')
				{
					msg_mask = 0;

					// Parse command for sequence number
					if(((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+3]) == '-') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+4]) == '1') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+5]) == ','))
					{
						sequence_number = 1;
						msg_mask += 3;
					}
					else
					{
						mask_buff = msg_mask;
						msg_mask++;
						while ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != ',')
						{
							msg_mask++;
						}

						unsigned char j;
						unsigned short tmp_sequence_number = 0;

						// Extract frame count value
						for (j = mask_buff; j<(msg_mask-1); j++)
						{
							tmp_sequence_number = (tmp_sequence_number*10) + (host_cmd[cmd_index+CMD_CMD_OFFSET+3+j]-48);
						}

						sequence_number = tmp_sequence_number;
					}

					// Parse command for channel
					if(((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+3]) == '-') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+4]) == '1') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+5]) == ','))
					{
						channel = -1;
						msg_mask += 3;
					}
					else
					{
						mask_buff = msg_mask;
						msg_mask++;
						while ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != ',')
						{
							msg_mask++;
						}

						unsigned char j;
						unsigned long tmp_channel = 0;

						// Extract frame count value
						for (j = mask_buff; j<(msg_mask-1); j++)
						{
							tmp_channel = (tmp_channel*10) + (host_cmd[cmd_index+CMD_CMD_OFFSET+3+j]-48);
						}

						channel = (signed short) tmp_channel;
					}

					// Parse command for count
					if(((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+3]) == '-') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+4]) == '1') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+5]) == 0x0D))
					{
						rx_tout = 0;
						msg_mask += 3;
					}
					if(((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+3]) == '0') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+msg_mask+4]) == 0x0D))
					{
						rx_tout = 0;
						msg_mask += 2;
					}
					else
					{
						mask_buff = msg_mask;
						msg_mask++;
						while ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != 0x0D)
						{
							msg_mask++;
						}

						unsigned char j;
						unsigned char tmp_tout = 0;

						// Extract frame count value
						for (j = mask_buff; j<(msg_mask-1); j++)
						{
							tmp_tout = (tmp_tout*10) + (host_cmd[cmd_index+CMD_CMD_OFFSET+3+j]-48);
						}

						rx_tout = tmp_tout;
					}

					// Execute RX test command
					SfxRxTestMode(channel, sequence_number, rx_tout);

					// Terminate command with {OK<CR><LF>}
					uartPutStr("OK", 2);
					uartPutChar(CR);
					uartPutChar(LF);
				}
				else
				{
					ret_cmd = HOST_CMD_ERROR;
				}
				break;
			default:
				ret_cmd = HOST_CMD_ERROR;
				break;
			}
			break;
		case 'I':
			switch(host_cmd[cmd_index+CMD_CMD_OFFSET+1])
			{
			case 'D':
				// Device ID inquiry
				if(((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '?') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+3]) == 0x0D))
				{
					unsigned int jj;

					for(jj = 0; jj<8; jj++)
					{
						byteToHex((ch_id & 0xF), &tmp_str[(7-jj)]);
						ch_id = (id>>((jj)*4));
					}
					// Print current UL frequency and terminate with <CR><LF>
					uartPutStr(tmp_str, 8);
					uartPutChar(CR);
					uartPutChar(LF);
				}
				else
				{
					ret_cmd = HOST_CMD_ERROR;
				}
				break;
			case 'F':
				// UL Frequency config
				if((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '=')
				{
					// Parse command for uplink frequency
					msg_mask = 0;
					while ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != 0x0D)
					{
						msg_mask++;
					}

					unsigned char j;
					unsigned long tmp_freq = 0;

					// Extract frequency value
					for (j = 0; j<(msg_mask-1); j++)
					{
						tmp_freq = (tmp_freq*10) + (host_cmd[cmd_index+CMD_CMD_OFFSET+3+j]-48);
					}

					if (tmp_freq)
					{
						volatile unsigned long * Flash_ptrC;
						Flash_ptrC = (unsigned long *) &TxFrequency;

						FCTL3 = FWKEY;				// Clear Lock bit
						FCTL1 = FWKEY + ERASE; 		// Set Erase bit
						*Flash_ptrC = 0;			// Dummy write to erase Flash seg
						FCTL1 = FWKEY + BLKWRT;		// Enable long-word write
						*Flash_ptrC = tmp_freq;		// Write to flash
						FCTL1 = FWKEY;				// Clear WRT bit
						FCTL3 = FWKEY + LOCK;		// Set LOCK bit

						// Reinitialize sigfox api library
						err = SfxClose();
						err = SfxInit();

						// Check if reinitialization was performed correctly
						if (err == SFX_ERR_NONE)
						{
							// Terminate command with {OK<CR><LF>}
							uartPutStr("OK", 2);
							uartPutChar(CR);
							uartPutChar(LF);

							ret_cmd = HOST_CMD_SUCCESS;
						}
						else
						{
							ret_cmd = HOST_CMD_ERROR;
						}
					}
					else
					{
						ret_cmd = HOST_CMD_ERROR;
					}
				}
				else if (((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '?') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+3]) == 0x0D))
				{
					char tmp_str[16];

					// Convert unsigned long to string
					ltoa(TxFrequency, tmp_str);

					// Print current UL frequency and terminate with <CR><LF>
					uartPutStr(tmp_str, 9);
					uartPutChar(CR);
					uartPutChar(LF);
				}
				else
				{
					ret_cmd = HOST_CMD_ERROR;
				}
				break;
			default:
				ret_cmd = HOST_CMD_ERROR;
				break;
			}
			break;
		case 'D':
			switch(host_cmd[cmd_index+CMD_CMD_OFFSET+1])
			{
			case 'R':
				// DL Frequency config
				if((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '=')
				{
					// Parse command for downlink frequency
					msg_mask = 0;
					while ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != 0x0D)
					{
						msg_mask++;
					}

					unsigned char j;
					unsigned long tmp_freq = 0;

					// Extract frequency value
					for (j = 0; j<(msg_mask-1); j++)
					{
						tmp_freq = (tmp_freq*10) + (host_cmd[cmd_index+CMD_CMD_OFFSET+3+j]-48);
					}

					if (tmp_freq)
					{
						volatile unsigned long * Flash_ptrB;
						Flash_ptrB = (unsigned long *) &RxFrequency;

						FCTL3 = FWKEY;				// Clear Lock bit
						FCTL1 = FWKEY + ERASE; 		// Set Erase bit
						*Flash_ptrB = 0;			// Dummy write to erase Flash seg
						FCTL1 = FWKEY + BLKWRT;		// Enable long-word write
						*Flash_ptrB = tmp_freq;		// Write to flash
						FCTL1 = FWKEY;				// Clear WRT bit
						FCTL3 = FWKEY + LOCK;		// Set LOCK bit

						// Reinitialize sigfox api library
						err = SfxClose();
						err = SfxInit();

						// Check if reinitialization was performed correctly
						if (err == SFX_ERR_NONE)
						{
							// Terminate command with {OK<CR><LF>}
							uartPutStr("OK", 2);
							uartPutChar(CR);
							uartPutChar(LF);

							ret_cmd = HOST_CMD_SUCCESS;
						}
						else
						{
							ret_cmd = HOST_CMD_ERROR;
						}
					}
					else
					{
						ret_cmd = HOST_CMD_ERROR;
					}
				}
				else if (((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '?') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+3]) == 0x0D))
				{
					char tmp_str[16];

					// Convert unsigned long to string
					ltoa(RxFrequency, tmp_str);

					// Print current UL frequency and terminate with <CR><LF>
					uartPutStr(tmp_str, 9);
					uartPutChar(CR);
					uartPutChar(LF);
				}
				else
				{
					ret_cmd = HOST_CMD_ERROR;
				}
				break;
			default:
				ret_cmd = HOST_CMD_ERROR;
				break;
			}
			break;
		case 'C':
			switch(host_cmd[cmd_index+CMD_CMD_OFFSET+1])
			{
			case 'W':
				// Continuous wave test mode
				if((host_cmd[cmd_index+CMD_CMD_OFFSET+2]) == '=')
				{
					// Parse command for uplink frequency
					msg_mask = 0;
					while ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) != ',')
					{
						msg_mask++;
					}

					unsigned char j;
					unsigned long tmp_freq = 0;

					// Extract frequency value
					for (j = 0; j<(msg_mask-1); j++)
					{
						tmp_freq = (tmp_freq*10) + (host_cmd[cmd_index+CMD_CMD_OFFSET+3+j]-48);
					}

					if (((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) == ',') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask+1]) == '1') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask+2]) == 0x0D))
					{
						// Start CW
						RADIO_start_unmodulated_cw(tmp_freq);

						// Terminate command with {OK<CR><LF>}
						uartPutStr("OK", 2);
						uartPutChar(CR);
						uartPutChar(LF);
					}
					else if(((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask]) == ',') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask+1]) == '0') && ((host_cmd[cmd_index+CMD_CMD_OFFSET+2+msg_mask+2]) == 0x0D))
					{
						// Stop CW
						RADIO_stop_unmodulated_cw(tmp_freq);

						// Terminate command with {OK<CR><LF>}
						uartPutStr("OK", 2);
						uartPutChar(CR);
						uartPutChar(LF);
					}
					else
					{
						ret_cmd = HOST_CMD_ERROR;
					}
				}
				else
				{
					ret_cmd = HOST_CMD_ERROR;
				}
				break;
			default:
				ret_cmd = HOST_CMD_ERROR;
				break;
			}
			break;
		default:
			ret_cmd = HOST_CMD_ERROR;
			break;
		}
	}
	return ret_cmd;
}

/**********************************************************************//**
 * @brief  Converts bytes stored as ASCII string to int array
 *
 * @param  str		is pointer to the location where string is stored
 * @param  data		is pointer to the buffer where converted data will be stored
 * @param  length	is number of bytes to convert
 *
 * @return \b dd	is the count of bytes converted
 **************************************************************************/
unsigned char
stringToData(char *str, unsigned char *data, unsigned char length)
{
	unsigned char tmp;
	unsigned char ii, dd;

	dd = 0;
	for (ii=0; ii<length; ii=ii+2) {

		// grab the next character in the array
		tmp = *(str + ii);
		// check to see if it is a "space" if so skip it
		if(tmp == ' ') {
			ii++;
			tmp = *(str + ii);
		}
		// check to see if the next byte is a valid hex character if so convert it
		if (((tmp >= '0') && (tmp <= '9')) || ((tmp >= 'A') && (tmp <= 'F'))) {
			data[dd++] =  hexToByte(str + ii);
		}
	}
	return dd;
}

/**********************************************************************//**
 * @brief  Converts HEX numbers to ASCII coded HEX bytes
 *
 * @param  hex		is pointer to the hex number
 *
 * @return \b res	is the converted byte
 **************************************************************************/
unsigned char
hexToByte(char *hex)
{
	unsigned char tmp;
	unsigned char res;
	unsigned char ii;

	res = 0;
	for (ii=0; ii<2; ii++) {
		tmp = *(hex + ii);        // copy over the hex character to decode
		res = res << 4;           // move up the previous result by "4" bit locations
		if (((tmp >= '0') && (tmp <= '9')) || ((tmp >= 'A') && (tmp <= 'F')))
		{
			if (tmp <= '9')
			{
				res += tmp - '0';
			}
			else
			{
				res += tmp - 'A' + 10;
			}
		}
	}
	return res;
}

/**********************************************************************//**
 * @brief  Converts int array to string
 *
 * @param  data		is pointer to the data array to convert
 * @param  str		is pointer to the array where converted string will be stored
 * @param  length	is length of the data array
 *
 * @return \b dd	is number of data bytes converted to string
 **************************************************************************/
unsigned char
dataToString(unsigned char *data, char *str, unsigned char length)
{
	unsigned char ii, dd;

	ii = 0;
	for (dd=0; dd<length; dd++) {
#if 0 // space not needed
		// load in a "space"
		*(str + ii) = ' ';
		ii++;
#endif
		// check to see if the next byte is a valid hex character if so convert it
		byteToHex(data[dd], (str + ii));
		ii = ii + 2;
	}
	return dd;
}

/**********************************************************************//**
 * @brief  Converts ASCII coded HEX byte to Hex number
 *
 * @param  byte		is the ASCII byte to convert
 * @param  hex		is pointer to the array where converted data will be stored
 *
 **************************************************************************/
void
byteToHex(unsigned char byte, char *hex)
{
	unsigned char tmp;
	unsigned char ii;

	tmp = (byte & 0xF0)>>4;
	for (ii=0; ii<2; ii++) {

		if(tmp < 10) {
			hex[ii] = tmp + '0';
		} else {
			hex[ii] = tmp - 10 + 'A';
		}
		tmp = (byte & 0x0F);
	}
	return;
}

/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/
