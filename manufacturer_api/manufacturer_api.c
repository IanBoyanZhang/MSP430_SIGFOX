//*****************************************************************************
//! @file       manufacturer_api.c
//! @brief      Manage The SIGFOX library API
//! @author 	SigFox Test and Validation team
//! @version 	0.1
//!
//****************************************************************************/


/**************************************************************************//**
 * @addtogroup LibInterface
 * @{
 ******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#include "msp430.h"
#include "sigfox_demo.h"
#include "radio.h"
#include "device_config.h"
#include "ti_aes_128.h"
#include "transmission.h"
#include "timer.h"
#include "uart_drv.h"
#include "host_cmd.h"
#include "flash_drv.h"
#include "cc112x_spi.h"
#include "hal_spi_rf_trxeb.h"
#include "../sigfox_library_api/sigfox.h"
#include "stdlib.h"
#include "stdio.h"


/******************************************************************************
 * DEFINES
 */
#define RX_FIFO_ERROR       0x11
#define RSSI_OFFSET			102		// RSSI offset for CC112x


/******************************************************************************
 * GLOBAL VARIABLES
 */
e_SystemState SysState; /*!< TX status */

u8 DynamicMemoryTable[SFX_DYNAMIC_MEMORY];

MemoryBlock Table_64bytes[NUMBER_BLOCKS_64BYTES];
MemoryBlock Table_32bytes[NUMBER_BLOCKS_32BYTES];
MemoryBlock Table_30bytes[NUMBER_BLOCKS_30BYTES];
MemoryBlock Table_8bytes[NUMBER_BLOCKS_8BYTES];

/*!
 * \struct Nvram
 * \brief Object stored into non volatile memory (Flash, Eeprom, ...).
 */
struct
{
	u16 Pn9; /*!< Value stored for hopping system */
	u16 SeqNbr; /*!< Number used for manage the frame sequencing */
}Nvram;

/* Needed to get the memory before writing into flash
 * [0]PN9
 * [1]SeqNb
 */
u16 NonVRam[2];


/******************************************************************************
 * LOCAL VARIABLES
 */
static int RSSI = 0;


/******************************************************************************
* FUNCTIONS
*/
/***************************************************************************//**
 *	@brief  	This function is called to initialize the chipset in the correct mode
 *  @param  	e_ChipMode 		is the mode to use (RX or TX). ::te_RxChipMode
 *  @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_init(te_RxChipMode e_ChipMode)
{
	if(e_ChipMode == E_TX_MODE)
	{
		// Initialize the radio in TX mode
		RADIO_init_chip(*TxCF, E_TX_MODE );
	}
	else if (e_ChipMode == E_RX_MODE )
	{
		// Initialize the radio in RX mode
		RADIO_init_chip(*RxCF, E_RX_MODE );
	}
	else
	{
		return  SFX_ERR_INIT;
	}
	return SFX_ERR_NONE;
}


/***************************************************************************//**
 *   @brief    	This function is called to close the chipset usage in Sigfox mode (idle or standby)
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_close(void)
{
	RADIO_close_chip();
	return  SFX_ERR_NONE;
}


/***************************************************************************//**
 *   @brief  	This function is used to change the frequency into the chipset
 *   @param 	frequency 		is the new frequency to apply.
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_change_frequency(u32 frequency)
{
#ifdef RF_DEBUG
	char freq[9];
	ltoa(frequency, freq);
	uartPutStr(freq,9);
	uartPutChar(0x0D);
#endif
	RADIO_change_frequency(frequency);
	return  SFX_ERR_NONE;
}

/***************************************************************************//**
 *   @brief  	This function is used to get several parameters relating to the module.
 *   @param  	vdd_idleptr 	is pointer to the voltage in idle mode
 *   @param  	vdd_txptr 		is pointer to the voltage in tx mode
 *   @param  	tempptr 		is pointer to the temperature
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_get_voltage_temperature(u16 *vdd_idleptr, u16 *vdd_txptr, u16 *tempptr)
{
	// TO BE IMPLEMENTED WITH PROPER VALUES
	*vdd_idleptr = 0xCE4; // 3300mV
	*vdd_txptr = 0xC1C;   // 3100mV
	*tempptr = 0x00FA;    //250/10 degrees Celsius

	return  SFX_ERR_NONE;
}

/***************************************************************************//**
 *   @brief   	Function for data encryption using CRC
 *   @param   	Encrypted_data 		is pointer to the encrypted data
 *   @param   	Data_To_Encrypt 	is pointer to the data to encrypt
 *   @param   	data_len 			is size of the data to decrypt (16 bytes or 32 bytes in our case)
 *   @param   	key 				is the AES key to use to decrypt data
 *   @param   	iv 					is the initialisation vector
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_AES_128_cbc_encrypt(u8 *Encrypted_data, u8 *Data_To_Encrypt,
		u8 data_len, const u8 *key, const u8 *iv)
{
	u8 i, j, blocks;
	u8 cbc[16]= {0x00};


	for (j = 0; j < 16; j++)
		cbc[j] = iv[j];

	blocks = data_len / 16;
	for (i = 0; i < blocks; i++) {
		for (j = 0; j < 16; j++)
			cbc[j] ^= Data_To_Encrypt[j+i*16];

		aes_enc_dec(cbc, key, 0);
		for (j = 0; j < 16; j++)
			Encrypted_data[j+(i*16)] = cbc[j];
	}
	return SFX_ERR_NONE;
}


/***************************************************************************//**
 *   @brief   	Function to manage the memory needed by the library to generate frames
 *   @param   	size 			is the size needed
 *   @return  	\b mem_ptr 		is pointer to the memory allocated
 *******************************************************************************/
u8*
sfx_malloc(u16 size)
{
	u8 i;
	MemoryBlock * mem_blk;
	MemoryBlock * ptr;
	u8 nb_block = 0;
	u8 * mem_ptr = NULL;

	switch(size)
	{
		case 30:
			mem_blk = Table_30bytes;
			nb_block = NUMBER_BLOCKS_30BYTES;
			break;
		case 32:
			mem_blk = Table_32bytes;
			nb_block = NUMBER_BLOCKS_32BYTES;
			break;	
		case 64:
			mem_blk = Table_64bytes;
			nb_block = NUMBER_BLOCKS_64BYTES;
			break;
		case 8:
			mem_blk = Table_8bytes;
			nb_block = NUMBER_BLOCKS_8BYTES;
			break;
		default:
			// No block defined for that size
			break;
	}

	if (nb_block != 0)
	{
		for (i=0; i < nb_block && mem_ptr== NULL; i++)
		{
			ptr =  (mem_blk + i);

			if ( ptr->allocated == FALSE)
			{
				// The memory block is free, we can allocate it
				mem_ptr = ptr->memory_ptr ;
				ptr->allocated = TRUE;
			}
		}
	}

	return mem_ptr;
}


/***************************************************************************//**
 *   @brief   	Function to free up a memory block
 *   @param   	p 			is a pointer to the memory to free
 *   @param 	nb_blocks 	is the number of blocks to free
 *   @param 	table_ptr 	is a pointer to the free memory
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
static SFX_error_t free_memory_block (u8 *p, u8 nb_blocks , MemoryBlock *table_ptr )
{
	u8 i;
	SFX_error_t status = SFX_ERR_INIT;

	for (i=0 ; i< nb_blocks ; i++)
	{
		if ( p == (table_ptr + i)->memory_ptr )
		{
			if ( (table_ptr + i)->allocated == TRUE )
			{
				(table_ptr + i )->allocated = FALSE;
				status = SFX_ERR_NONE;            
			}
			else
			{
				status = SFX_ERR_INIT;
			}
		}
	}
	return status;
}


/***************************************************************************//**
 *   @brief   	This function is used to free a memory space
 *   @param   	p 			is a pointer to the memory to free
 *   @return  	error code ::SFX_error_t
*******************************************************************************/
SFX_error_t
sfx_free(u8 *p)
{
	SFX_error_t status = SFX_ERR_INIT;
	// search at which memory area the pointer is assigned to be able to free the memory
	if ( ( p >= Table_64bytes[0].memory_ptr) && ( p <= Table_64bytes[NUMBER_BLOCKS_64BYTES-1].memory_ptr) )
	{
		status = free_memory_block (p,NUMBER_BLOCKS_64BYTES, Table_64bytes);
	}
	else if ( ( p >= Table_32bytes[0].memory_ptr) && ( p <= Table_32bytes[NUMBER_BLOCKS_32BYTES-1].memory_ptr) )
	{
		status = free_memory_block (p, NUMBER_BLOCKS_32BYTES, Table_32bytes);
	}
	else if ( ( p >= Table_30bytes[0].memory_ptr) && ( p <= Table_30bytes[NUMBER_BLOCKS_30BYTES-1].memory_ptr) )
	{
		status = free_memory_block (p, NUMBER_BLOCKS_30BYTES, Table_30bytes);
	}
	else if ( ( p >= Table_8bytes[0].memory_ptr) && ( p <= Table_8bytes[NUMBER_BLOCKS_8BYTES-1].memory_ptr) )
	{
		status = free_memory_block (p,NUMBER_BLOCKS_8BYTES, Table_8bytes);
	}
	return status;
}


/***************************************************************************//**
 *   @brief 	This function is used to manage the different delay used by the library
 *   @param 	e_TypeDelay 		is the type of delay to call ::te_DelayType
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_delay(te_DelayType e_TypeDelay)
{
	switch(e_TypeDelay)
	{
		case E_RX_DELAY :
			__delay_cycles(10000000);  // 500 ms (20000000_SMCLK * 2)
			break;
		case E_TX_DELAY:
			__delay_cycles(20000000);  // 1000 ms (20000000_SMCLK)
			break;
		case E_OOB_ACK_DELAY:
			__delay_cycles(28000000);  // 1400 ms (20000000_SMCLK * 1.4)
			break;
		default:
			break;
	}
	return SFX_ERR_NONE;
}


/***************************************************************************//**
 *   @brief  	This function is used to interface the library to a non volatile memory.
 *           	following data need to be stored when circuit is in off mode
 *   @param  	e_DataTypeW 	is an enum ::te_DataType for the data type to write
 *   @param  	valueW 			is the data to write into the memory
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_set_nv_mem(te_DataType e_DataTypeW, u16 valueW)
{
	if( e_DataTypeW == E_PN)
	{
		NonVRam[0] = valueW;
	}
	else
	{
		NonVRam[1] = valueW;
	}
	flash_write_wear_level( (unsigned int *)NonVRam, 2);

	return SFX_ERR_NONE;
}

/***************************************************************************//**
 *   @brief  	This function is use to interface the library to a non volatile memory.
 *             	following data need to be stored when circuit is in off mode
 *   @param  	e_DataTypeR 	is an enum ::te_DataType for the data type to get
 *   @param  	valueR 			is a pointer to the data read
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_get_nv_mem(te_DataType e_DataTypeR, u16 *valueR)
{
	//get a value from an index in non volatile memory
	flash_read_wear_level((unsigned int *)NonVRam, 2);
	if( e_DataTypeR == E_PN)
	{
		*valueR = NonVRam[0];
	}
	else
	{
		*valueR = NonVRam[1];
	}

	return SFX_ERR_NONE;
}

/***************************************************************************//**
 *   @brief 	This function manages the transmission in DBPSK to the radio
 *   @param 	message 	is pointer to the data buffer that is be modulated and sent
 *   @param 	size 		is number of bytes to send
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_send(u8 *message, u8 size)
{
	uint8 End_Transmission = FALSE;
	u8 marcStatus;
	SysState = TxStart;

	// Loop till the end of the transmission. Symbols are sent when interrupt is asserted.
	while(End_Transmission == FALSE)
	{
		// Send frame management
		switch(SysState)
		{
			case TxStart:
				TxInit();

				// Power up the radio
				RADIO_start_rf_carrier();

				// Wait till RF stabilized ~ 1 ms
//				__delay_cycles(20000);

				// Start the bitrate timer
				TIMER_bitrate_start();
				cc112xSpiReadReg(CC112X_MARCSTATE, &marcStatus, 1);

				SysState = TxWaiting; //TxWaiting;
				break;

			case TxWaiting:
				// Continue processing - there are still bits to transmit
				break;

			case TxProcessing:
				// Modulate the TX packet
				if(TxProcess(message, size) == 1)
				{
					SysState = TxEnd;
				}
				else
				{
					SysState = TxWaiting;
				}
				break;

			case TxEnd:
				// End of the transmission. Disable the timer interrupt
				TIMER_bitrate_stop();

				// Power down the radio
				RADIO_stop_rf_carrier();
				End_Transmission = TRUE;
				break;

			default:
				End_Transmission = TRUE;
				break;
		}
	}
	return SFX_ERR_NONE;
}



////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// DOWNLINK MODE API //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/***************************************************************************//**
 *   @brief		Function that stop the receive window timer when frame is received
 *******************************************************************************/
void
sfx_StopRxTimeout(void)
{
	// Reset the timeout value
	TIMER0_timeout = FALSE;

	TA0CTL &= ~MC_1;
}

/***************************************************************************//**
 *   @brief   	Start the timeout for waiting the receiving window (20s for Sigfox)
 *   @param   	time_in_seconds
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_StartWaitingTimeout(u16 time_in_seconds)
{
	// Start the timer
	TIMER_downlink_timing_init(time_in_seconds);

	return SFX_ERR_NONE;
}

/***************************************************************************//**
 *   @brief  	This Function waits (20s) before the RX Window
 *           	If there are additionnal handling to be executed for the application,
 *           	this can be done in this function.
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_WaitForTimeoutRx(void)
{
	// Need to wait for the time set in sfx_StartWaitingTimeout
	while( TIMER0_timeout == FALSE )
	{
		// Loop until timeout
	}

	// Reset the timeout value
	TIMER0_timeout = FALSE;

	// Stop the interrupt
	TA0CTL &= ~MC_1;

	return SFX_ERR_NONE;
}


/***************************************************************************//**
 *   @brief 	This function returns the RSSI value from the last Rx frame
 *   @return 	\b Rssi value coded on a signed integer (with cc112x offset).
 *******************************************************************************/
s8
sfx_getrssivalue()
{
	return ((s8)(RSSI));
}

/***************************************************************************//**
 *   @brief     Start the timeout for window reception (25s for SIGFOX)
 *   @param     WindowTime 		is the time for the Rx window
 *   @return  	error code ::SFX_error_t
 *******************************************************************************/
SFX_error_t
sfx_StartRxTimeout(u16 WindowTime)
{
	// Initialize the number of interrupt to wait for
	TIMER_downlink_timing_init( WindowTime );

	return SFX_ERR_NONE;
}

/***************************************************************************//**
 *   @brief  	This function is dedicated to the reception of SigFox frame.
 *           	It loops on the frame reception signal and check that received frame
 *           	is for our device.
 *   @param  	frame 			is the buffer allocated to the reception of a frame
 *   @return 	Waiting Status ::SFX_ext_status
 *******************************************************************************/
SFX_ext_status
sfx_waitframe(u8 *frame)
{
	uint8 rxlastindex;
	uint8 marcStatus;
	SFX_ext_status status = E_FRAME_ERROR;

	// Set radio in RX
	trxSpiCmdStrobe(CC112X_SRX);

	while ((packetSemaphore == ISR_IDLE ) && (TIMER0_timeout == FALSE ))
	{
		_NOP();// Loop - wait for the packet to be received or TIMER0 timeout
	}

	// If a Sigfox frame has been received
	if ( packetSemaphore == ISR_ACTION_REQUIRED)
	{
		unsigned char ii;
		unsigned char rx_last;

		// Read number of bytes in rx fifo
		/* IMPORTANT NOTE : using the register CC112X_NUM_RXBYTES gives wrong values concerning the packet length
		 * DO NOT USE THIS REGISTER, use RXLAST instead which give the last index in the RX FIFO
		 * and flush the FIFO after reading it   */
		rxlastindex = 0;

		// Read 3 times to get around a bug
		for (ii=0; ii<3; ii++)
		{
			cc112xSpiReadReg(CC112X_RXLAST, &rx_last, 1);
			rxlastindex |= rx_last;
		}

		// Check that we have bytes in fifo
		if(rxlastindex != 0)
		{
			// Read marcstate to check for RX FIFO error
			cc112xSpiReadReg(CC112X_MARCSTATE, &marcStatus, 1);

			// Mask out marcstate bits and check if we have a RX FIFO error
			if((marcStatus & 0x1F) == RX_FIFO_ERROR)
			{
				// Flush RX Fifo
				trxSpiCmdStrobe(CC112X_SFRX);

				status = E_FRAME_ERROR;
			}
			// No RX FIFO error
			else
			{
				// Sigfox packet length is 16 bytes. Anything else after that in RXFIFO is useless.
				if(rxlastindex>=16)
				{
					rxlastindex = 16;
				}

				// Read from RX FIFO
				cc112xSpiReadRxFifo(frame, rxlastindex+1);
				cc112xSpiReadReg(CC112X_MARCSTATE, &marcStatus, 1);

				// Once read, Flush RX Fifo
				trxSpiCmdStrobe(CC112X_SFRX);

				// Check CRC ok (CRC_OK: bit7 in second status byte)
				// This assumes status bytes are appended in RX_FIFO (PKT_CFG1.APPEND_STATUS = 1.)
				// If CRC is disabled the CRC_OK field will read 1
				if(frame[rxlastindex] & 0x80)
				{
					// Extract the RSSI value
					RSSI = frame[15];
				}

				status = E_FRAME_RECEIVED;
			}
		}

		// Reset packet semaphore
		packetSemaphore = ISR_IDLE;

		// Go back to receive mode
		trxSpiCmdStrobe(CC112X_SRX);

	}
	else if ( TIMER0_timeout == TRUE )
	{
		// Reset the timeout value
		TIMER0_timeout = FALSE;

		// Stop the interrupt
		TA0CTL &= ~MC_1;

		status = E_FRAME_TIMEOUT;
	}
	else
	{

	}

	return status;
}


/***************************************************************************//**
 *   @brief  	Called to print the received frame in Rx Test mode
 *   @param 	ReplyForm 		is pointer to the buffer containing received data
 *   @param 	RssiValue 		is the value of the RSSI to print
 *******************************************************************************/
void
sfx_PrintRxTestResult(u8* ReplyForm, u8 RssiValue)
{
	char rssi[4];
	int32 rssiValL;
	char msg_str[16] = {0};
	uint8 msg[8] = {0};
	char * msg_str_p = &msg_str[0];
	uint8 ii;

	// Convert ReplyForm message to HEX string
	for(ii=0; ii<8; ii++)
	{
		msg[ii] = ReplyForm[ii];
	}
	dataToString(msg, msg_str, 16);

	// Convert 8 bit signed to 32 bit signed
	if(RssiValue & 0x80) {
		rssiValL = 0xFFFFFF80 | RssiValue;
	} else {
		rssiValL = 0x00000000 | RssiValue;
	}

	// Subtract RSSI offset of CC112x board
	rssiValL = rssiValL - 102;

	// Convert rssi Value to string
	ltoa(rssiValL,rssi);

	// Print the RX test result {RX=<message><CR>}
	uartPutStr("RX=",3);
	uartPutStr(msg_str_p,16);
	uartPutChar(0x0D);
	uartPutChar(0x0A);

	// Print the RSSI value for the message received {RSSI=<rssi_value><CR>}
	uartPutStr("RSSI=",5);
	uartPutStr(rssi,4);
	uartPutChar(0x0D);
	uartPutChar(0x0A);
}


/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/
