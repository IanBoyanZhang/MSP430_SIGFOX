//*****************************************************************************
//! @file       circ_buf.c
//! @brief      Circular buffer control for the UART interface.
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
#include "circ_buf.h"


/******************************************************************************
 * FUNCTIONS
 */
/***************************************************************************//**
 *	@brief  	Find the remaining space left in the circular buffer
 *
 *  @param  	buffer 	is pointer to the circular buffer
 *
 *  @return  	\b rem 	is number of bytes remaining in the buffer
 *******************************************************************************/
unsigned char
circBufRemainder(circ_buffer_t *buffer)
{
	volatile unsigned char rem;

	// Remaining space in the circular buffer
	if(buffer->head_ptr >= buffer->tail_ptr) {
		rem = buffer->size_of_buffer - (buffer->head_ptr - buffer->tail_ptr);
	} else {
		rem = buffer->tail_ptr - buffer->head_ptr;
	}
	return rem;
}


/***************************************************************************//**
 *	@brief  	Add a char the circular buffer
 *
 *  @param  	buffer 	is pointer to the circular buffer
 *  @param  	data 	is the char to be added to the circular buffer
 *******************************************************************************/
void
circBufPutData(circ_buffer_t *buffer, unsigned char data)
{
	while(circBufRemainder(buffer) == 0);

	// Write the char to the buffer
	buffer->buffer[buffer->head_ptr++] = data;

	// Circle the buffer if full
	if(buffer->head_ptr == buffer->size_of_buffer) {
		buffer->head_ptr = 0;
	}
	return;
}


/***************************************************************************//**
 *	@brief  	Get a char from the circular buffer
 *
 *  @param  	buffer 	is pointer to the circular buffer
 *
 *  @return  	\b ret is the char read from the circular buffer
 *******************************************************************************/
unsigned char
circBufGetData(circ_buffer_t *buffer)
{
    unsigned char ret;

    while(circBufRemainder(buffer) == buffer->size_of_buffer);

    // Read the char from the buffer
	ret = buffer->buffer[buffer->tail_ptr++];

	// Circle the buffer if full
	if(buffer->tail_ptr == buffer->size_of_buffer) {
		buffer->tail_ptr = 0;
	}
	return ret;
}

/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/
