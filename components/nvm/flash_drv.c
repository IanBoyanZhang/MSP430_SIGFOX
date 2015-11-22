//*****************************************************************************
//! @file       flash_drv.c
//! @brief      Drivers to read/write in flash
//
//  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
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
* @addtogroup FLASH
* @{
******************************************************************************/

#include "msp430.h"
#include "flash_drv.h"


#if defined (__MSP430F5438A__) || defined (__MSP430F5529__)

#define FLASH_ARRAY_ORIGIN 0x8000
#pragma location=FLASH_ARRAY_ORIGIN
unsigned int flash_array[SIZE_OF_STORAGE_ARRAY];

/**************************************************************************//**
* @brief    Erase then write data on the flash memeory
*
* @param	data	is pointer to the data to write
* @param	index 	is the start location on the flash
* @param	length	is the length of the data segment
******************************************************************************/
void
flash_write_data(unsigned int *data, unsigned int index ,unsigned int length)
{
	// Initialize Flash pointer Seg
	unsigned int *Flash_ptr;
	unsigned int ii;

	Flash_ptr = flash_array;                 // Initialize Flash pointer

	// 5xx Workaround: Disable global interrupt while erasing
	__disable_interrupt();

	FCTL3 = FWKEY;                            // Clear Lock bit
	FCTL1 = FWKEY+WRT;                        // Enable 16 bit write operation
	for(ii=0; ii<length; ii++)
	{
		*(Flash_ptr+ii+index) = data[ii];     // Write to Flash
		while (FCTL3 & BUSY );
	}
	FCTL1 = FWKEY;                            // Clear WRT bit
	FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
	// 5xx Workaround: Re-enable global interrupt after erasing
	__enable_interrupt();

	return;

}

/**************************************************************************//**
* @brief    Erases a segment on the flash memeory
*
* @param	index 	is the start location of the segment
* @param	length	is the length of the data segment
******************************************************************************/
void
flash_erase_segment(unsigned int index, unsigned int length)
{
	// Initialize Flash pointer Seg
	unsigned int *Flash_ptr;
	unsigned int ii;

	// Initialize Flash pointer
	Flash_ptr = flash_array;

	// 5xx Workaround: Disable global interrupt while erasing.
	__disable_interrupt();
	for(ii=0; ii<length; ii=ii+SEGMENT_SIZE)
	{
		FCTL3 = FWKEY;                            // Clear Lock bit
		FCTL1 = FWKEY+ERASE;                      // Set Erase bit
		*(Flash_ptr+ii+index) = 0;                // Dummy write to erase Flash segment
		while (FCTL3 & BUSY );
		FCTL1 = FWKEY;                            // Clear WRT bit
		FCTL3 = FWKEY+LOCK;                       // Set LOCK bit

	}
	// 5xx Workaround: Re-enable the global interrupt after erasing
	__enable_interrupt();

	return;
}


#endif

#if defined (__MSP430G2553__)

#define FLASH_ARRAY_ORIGIN 0xD000
#pragma location=FLASH_ARRAY_ORIGIN
unsigned int flash_array[SIZE_OF_STORAGE_ARRAY];

/**************************************************************************//**
* @brief    Erases a segment on the flash memeory
*
* @param	index 	is the start location of the segment
* @param	length	is the length of the data segment
******************************************************************************/
void
flash_erase_segment(unsigned int index, unsigned int length)
{

	unsigned int *Flash_ptr;                         // local Flash pointer
	unsigned int ii;

	// clock setting
	//------------------------
	//DCOCTL = CALDCO_16MHZ;                  // Set DCO to 16MHz
	//BCSCTL1 = CALBC1_16MHZ;                 // MCLC = SMCLK = DCOCLK = 16MHz
	//BCSCTL1 |= DIVA_0;                      // ACLK = ACLK/1
	//BCSCTL3 = LFXT1S_2;                     // LFXT1 = ACLK = VLO = ~12kHz
	//BCSCTL3 &= ~LFXT1OF;                    // clear LFXT1OF flag, 0

	Flash_ptr = flash_array;                 // Initialize Flash pointer

	// flash memory controller
	// flash memory controller
	FCTL2 = FWKEY + FSSEL_2 + FN5 + FN3;      // SMCLK/40 for flash timing generator
	for(ii=0; ii<length; ii=ii+SEGMENT_SIZE)
	{
		FCTL3 = FWKEY;
		FCTL1 = FWKEY + ERASE;
		*(Flash_ptr+ii) = 0;              // Write to Flash
		while (FCTL3 & BUSY );
		FCTL1 = FWKEY;
		FCTL3 = FWKEY +  LOCK;
	}
	return;
}

/**************************************************************************//**
* @brief    Erase then write data on the flash memeory
*
* @param	data	is pointer to the data to write
* @param	index 	is the start location on the flash
* @param	length	is the length of the data segment
******************************************************************************/
void
flash_write_data(unsigned int *data, unsigned int index ,unsigned int length)
{

	unsigned int *Flash_ptr;               // Initialize Flash pointer Seg D
	unsigned int ii;

	Flash_ptr = flash_array;                 // Initialize Flash pointer

	// flash memory controller
	FCTL2 = FWKEY + FSSEL_2 + FN5 + FN3;      // SMCLK/40 for flash timing generator
	FCTL3 = FWKEY;
	FCTL1 = FWKEY + WRT;
	for(ii=0; ii<length; ii++)
	{
		*(Flash_ptr+ii+index) = data[ii];              // Write to Flash
		while (FCTL3 & BUSY );
	}
	FCTL1 = FWKEY;
	FCTL3 = FWKEY + LOCK;
	return;
}

#endif

/**************************************************************************//**
* @brief    Reads data of the given length starting from the index location
*
* @param	data	is the pointer to store the read the data
* @param	index	is the start location of the data
* @param	length	is the length of the data to read
******************************************************************************/
void
flash_read_data(unsigned int *data, unsigned int index ,unsigned int length)
{

	unsigned int *Flash_ptr;               // Initialize Flash pointer
	unsigned int ii;

	Flash_ptr = flash_array;                 // Initialize Flash pointer

	for(ii=0; ii<length; ii++) {
		data[ii] = *(Flash_ptr+ii+index);              // Write to Flash
	}
	return;
}


/**************************************************************************//**
* @brief    Writes data after the last occupied location and returns the updated
* 			value for the last occupied location
*
* @param	data	is the pointer to the data that will be written to the flash
* @param	length	is the length of the data that will be written to the flash
*
* @return   \b flash_write_count 	is the last occupied location
******************************************************************************/
unsigned int
flash_write_wear_level(unsigned int *data, unsigned int length)
{

	unsigned int ii, index;
	unsigned int flash_write_count;

	flash_read_data(&flash_write_count, 0 ,1);
	switch(flash_write_count){
	// the case of a un-initialized array, brandnew device
	case 0xFFFF:
		flash_write_count = 1;
		flash_write_data(&flash_write_count, 0 ,1);
		flash_write_data(data, 1 ,length);
		break;
		// the case of having reached 50000 writes, reset the counter to 1
	case 0xC350:
		flash_write_count = 1;
		flash_erase_segment(0, SIZE_OF_STORAGE_ARRAY);
		flash_write_data(&flash_write_count, 0 ,1);
		flash_write_data(data, 1 ,length);
		break;
		// Normal operation, find the end of the current array, store new data
	default:
		// search the array for the end of the flash storage array, look for 0xFFFF
		for(ii=0;ii<SIZE_OF_STORAGE_ARRAY;ii=ii+length+1)
		{
			flash_read_data(&flash_write_count, ii ,1);
			if(flash_write_count == 0xFFFF)
			{
				flash_read_data(&flash_write_count, 0 ,1);
				index = ii;
				ii = SIZE_OF_STORAGE_ARRAY;
			}
		}
		// before writing to segment, check to see if we are out of bound
		if(index+length+1 > SIZE_OF_STORAGE_ARRAY )
		{
			// if we have reached the end, delete the entire array, increment counter and restart
			flash_read_data(&flash_write_count, 0 ,1);
			flash_erase_segment(0, SIZE_OF_STORAGE_ARRAY);
			flash_write_count++;
			flash_write_data(&flash_write_count, 0 ,1);
			flash_write_data(data, 1 ,length);
		} else
		{
			// Normal write to Flash array
			flash_write_data(&flash_write_count, index ,1);
			flash_write_data(data, index+1 ,length);
		}
		break;
	}
	return flash_write_count;
}


/**************************************************************************//**
* @brief    Reads data from flash and returns the the last occupied location
*
* @param	data	is the pointer to to store the read data
* @param	length	is the length of the data to read
*
* @return   \b flash_write_count 	is the last occupied location
******************************************************************************/
unsigned int
flash_read_wear_level(unsigned int *data, unsigned int length)
{
	unsigned int ii, index;
	unsigned int flash_write_count;

	// check to see if there is any data in the array before starting the search
	flash_read_data(&flash_write_count, 0 ,1);
	if (flash_write_count == 0xFFFF) {
		return 0;
	}

	// search for the end last block to be occupied
	for(ii=0;ii<SIZE_OF_STORAGE_ARRAY;ii=ii+length+1) {
		flash_read_data(&flash_write_count, ii ,1);
		if(flash_write_count == 0xFFFF) {
			flash_read_data(&flash_write_count, 0 ,1);
			index = ii-(length+1);
			ii = SIZE_OF_STORAGE_ARRAY;
		}
	}
	// read the data from the last occupied location and return
	flash_read_data(&flash_write_count, index ,1);
	flash_read_data(data, index+1 ,length);

	return flash_write_count;
}

/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
