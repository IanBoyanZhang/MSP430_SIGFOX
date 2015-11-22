//*****************************************************************************
//! @file       uart_drv.c
//! @brief      UART drivers for MSP430F5xxx
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
#include  "msp430.h"
#include "circ_buf.h"
#include "uart_drv.h"


/******************************************************************************
 * LOCAL VARIABLES
 */
/*! allocate two circular buffer structures, for for tx and one for rx */
circ_buffer_t uart_tx_buf;
circ_buffer_t uart_rx_buf;

/*! allocate space for the uart itself, these buffers will be linked to
 * circular buffer the structures above                               */

char tx_buf[TX_UART_BUFFER_SIZE] = {0};
char rx_buf[RX_UART_BUFFER_SIZE] = {0};

unsigned char uart_state = UART_ECHO_OFF;
unsigned char rx_str_length;
unsigned char rx_end_of_str;


/******************************************************************************
 * FUNCTIONS
 */
/**********************************************************************//**
 * @brief  	Writes a char to the tx buffer
 *
 * @param	character 	is the char to write
 **************************************************************************/
void
uartPutChar(char character)
{
	// Write character to the uart_tx_buf
	circBufPutData(&uart_tx_buf, character);

	// Force the isr to be active
	halUartStartTx();
	return;
}


/**********************************************************************//**
 * @brief  	Writes a string to the tx buffer
 *
 * @param  	str 		is pointer to the string
 * @param	length 		is the lenght of the string
 **************************************************************************/
void
uartPutStr(char *str, unsigned char length)
{
	unsigned char ii;

	// Write str to the uart_tx_buf
	for(ii=0; ii< length; ii++) {
		circBufPutData(&uart_tx_buf, str[ii]);
	}

	// Force the isr to be active
	halUartStartTx();
	return;
}


/**********************************************************************//**
 * @brief  	Reads a char from the rx buffer
 *
 * @return 	\b character 	read from the \e uart_rx_buf
 **************************************************************************/
unsigned char
uartGetChar(void)
{
	return(circBufGetData(&uart_rx_buf));
}


/**********************************************************************//**
 * @brief  	Reads string from the rx buffer
 *
 * @param  	str 		is pointer to the location where string will be stored
 * @param 	length 		is the length of the string
 **************************************************************************/
void
uartGetStr(char *str, unsigned char length)
{
	unsigned char ii;

	// Read the string from uart_rx_buf and update str
	for(ii=0; ii<length; ii++) {
		str[ii] = circBufGetData(&uart_rx_buf);
	}

	return;
}


/**********************************************************************//**
 * @brief  	Calculates the length of the rx string
 *
 * @return 	\b rx_str_length
 **************************************************************************/
unsigned char
uartGetRxStrLength(void)
{
	return(rx_str_length);
}


/**********************************************************************//**
 * @brief  	Finds the end of the rx string
 *
 * @return 	\b rx_end_of_str
 **************************************************************************/
unsigned char
uartGetRxEndOfStr(void)
{
	return(rx_end_of_str);
}


/**********************************************************************//**
 * @brief  	Resets the end of the rx string
 **************************************************************************/
void
uartResetRxEndOfStr(void)
{
	rx_end_of_str = NO_END_OF_LINE_DETECTED;
	return;
}


/**********************************************************************//**
 * @brief  	Toggles UART echo
 **************************************************************************/
void
uartDrvToggleEcho(void)
{
	if(uart_state == UART_ECHO_OFF)
	{
		uart_state = UART_ECHO_ON;
	}
	else
	{
		uart_state = UART_ECHO_OFF;
	}
	return;
}


#if UART_SER_INTF == F5_UART_INTF_USCIA0        // Interface to UART
/**********************************************************************//**
 * @brief  Initializes the serial communications peripheral and GPIO ports 
 **************************************************************************/
void
halUartInit(void)
{

	uart_rx_buf.buffer = rx_buf;     // provide link to data structure to circ buffer
	uart_rx_buf.size_of_buffer = RX_UART_BUFFER_SIZE;

	uart_tx_buf.buffer = tx_buf;     // provide link to data structure to circ buffer
	uart_tx_buf.size_of_buffer = TX_UART_BUFFER_SIZE;

	uart_tx_buf.head_ptr++;			 // increment the head_ptr in the tx buffer

	rx_end_of_str = NO_END_OF_LINE_DETECTED;
	rx_str_length = 0;

	UART_PORT_SEL |= UART_PIN_RXD + UART_PIN_TXD;
	UART_PORT_DIR |= UART_PIN_TXD;
	UART_PORT_DIR &= ~UART_PIN_RXD;

	UCA0CTL1 |= UCSWRST;                        // Reset State
	UCA0CTL1 |= UCSSEL_1;                       // ACLK
	UCA0CTL0 = UCMODE_0;
	UCA0CTL0 &= ~UC7BIT;                        // 8bit char

	UCA0BR0 = 3;                                // 9600 bits per second
	UCA0BR1 = 0;

	UCA0MCTL = UCBRS1 + UCBRS0;                 // Modulation UCBRSx = 3
	UCA0CTL1 &= ~UCSWRST;

	UCA0IE |= UCRXIE;                           // Enable USCI_A0 RX interrupt

	__bis_SR_register(GIE);                     // Enable Interrupts
}


/***********************************************************************//**
 * @brief  Disables the serial communications peripheral and clears the GPIO
 *         settings, reset the interupts.
 **************************************************************************/
void
halUartDeinit(void)
{
	UCA0IE &= ~UCRXIE;
	UCA0IE &= ~UCTXIE;
	UCA0CTL1 = UCSWRST;                          //Reset State
	UART_PORT_SEL &= ~( UART_PIN_RXD + UART_PIN_TXD );
	UART_PORT_DIR |= UART_PIN_TXD;
	UART_PORT_DIR |= UART_PIN_RXD;
	UART_PORT_OUT &= ~(UART_PIN_TXD + UART_PIN_RXD);
}


/**********************************************************************//**
 * @brief  Start the TX ISR, it will automatically stop when FIFO is empty
 **************************************************************************/
void
halUartStartTx(void)
{
	UCA0IE |= UCTXIE;                           // Enable USCI_A0 TX interrupt
}


/**********************************************************************//**
 * @brief  UART ISR
 **************************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void
USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
	char tmp_uart_data;
	switch(__even_in_range(UCA0IV,4))
	{
	case USCI_NONE:
		break;                             // Vector 0 - no interrupt
	case USCI_UCRXIFG:                                   // Vector 2 - RXIFG
		tmp_uart_data = UCA0RXBUF;

		if(circBufRemainder(&uart_rx_buf) > 0) {
			circBufPutData(&uart_rx_buf, tmp_uart_data);
		}
		if(uart_state == UART_ECHO_ON) {
			uartPutChar(tmp_uart_data);
		}
		// if its a "return" then activate main-loop
		if(tmp_uart_data == 13) {
			rx_end_of_str = END_OF_LINE_DETECTED;
			rx_str_length = uart_rx_buf.size_of_buffer - circBufRemainder(&uart_rx_buf);
			__bic_SR_register_on_exit(LPM3_bits);
		}
		break;
	case USCI_UCTXIFG:
		// check if there is more data to send
		if(circBufRemainder(&uart_tx_buf) < uart_tx_buf.size_of_buffer) {
			UCA0TXBUF = circBufGetData(&uart_tx_buf);
		} else {
			UCA0IE &= ~UCTXIE;                        // Disable USCI_A0 TX interrupt
		}

		break;                             // Vector 4 - TXIFG
	default: break;
	}
}

#elif UART_SER_INTF == F5_UART_INTF_USCIA1        // Interface to UART
/**********************************************************************//**
 * @brief  Initializes the serial communications peripheral and GPIO ports
 **************************************************************************/
void
halUartInit(void)
{

	uart_rx_buf.buffer = rx_buf;     // provide link to data structure to circ buffer
	uart_rx_buf.size_of_buffer = RX_UART_BUFFER_SIZE;

	uart_tx_buf.buffer = tx_buf;     // provide link to data structure to circ buffer
	uart_tx_buf.size_of_buffer = TX_UART_BUFFER_SIZE;

	// BUG FIX: missing first char in the uart TX
	uart_tx_buf.head_ptr++;			 // increment the head_ptr in the tx buffer

	rx_end_of_str = NO_END_OF_LINE_DETECTED;
	rx_str_length = 0;

	UART_PORT_SEL |= UART_PIN_RXD + UART_PIN_TXD;
	UART_PORT_DIR |= UART_PIN_TXD;
	UART_PORT_DIR &= ~UART_PIN_RXD;

	UCA1CTL1 |= UCSWRST;                        // Reset State
	UCA1CTL1 |= UCSSEL_1;                       // ACLK
	UCA1CTL0 = UCMODE_0;
	UCA1CTL0 &= ~UC7BIT;                        // 8bit char

	UCA1BR0 = 3;                                // 9600 bits per second
	UCA1BR1 = 0;

	UCA1MCTL = UCBRS1 + UCBRS0;                 // Modulation UCBRSx = 3
	UCA1CTL1 &= ~UCSWRST;

	UCA1IE |= UCRXIE;                           // Enable USCI_A0 RX interrupt

	__bis_SR_register(GIE);                     // Enable Interrupts
}


/***********************************************************************//**
 * @brief  Disables the serial communications peripheral and clears the GPIO
 *         settings, reset the interupts.
 **************************************************************************/
void
halUartDeinit(void)
{
	UCA1IE &= ~UCRXIE;
	UCA1IE &= ~UCTXIE;
	UCA1CTL1 = UCSWRST;                          //Reset State
	UART_PORT_SEL &= ~( UART_PIN_RXD + UART_PIN_TXD );
	UART_PORT_DIR |= UART_PIN_TXD;
	UART_PORT_DIR |= UART_PIN_RXD;
	UART_PORT_OUT &= ~(UART_PIN_TXD + UART_PIN_RXD);
}


/**********************************************************************//**
 * @brief 	Start the TX ISR, it will automatically stop when FIFO is empty
 **************************************************************************/
void
halUartStartTx(void)
{
	if((UCA1IE & UCTXIE) == 0)
	{
		UCA1IE |= UCTXIE;                           // Enable USCI_A1 TX interrupt
		UCA1TXBUF = circBufGetData(&uart_tx_buf);
	}
}


/**********************************************************************//**
 * @brief  UART ISR
 **************************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
	char tmp_uart_data;
	switch(__even_in_range(UCA1IV,4))
	{
	case USCI_NONE:
		break;                             // Vector 0 - no interrupt
	case USCI_UCRXIFG:                                   // Vector 2 - RXIFG
		tmp_uart_data = UCA1RXBUF;

		if(tmp_uart_data != 0x0A)			// Ignor Line Feed
		{
			if(circBufRemainder(&uart_rx_buf) > 0)
			{
				circBufPutData(&uart_rx_buf, tmp_uart_data);
			}
			if(uart_state == UART_ECHO_ON)
			{
				uartPutChar(tmp_uart_data);
			}
			// if its a "return" then activate main-loop
			if(tmp_uart_data == 13)
			{
				rx_end_of_str = END_OF_LINE_DETECTED;
				rx_str_length = uart_rx_buf.size_of_buffer - circBufRemainder(&uart_rx_buf);
				__bic_SR_register_on_exit(LPM3_bits);
			}
		}
		break;
	case USCI_UCTXIFG:
		// check if there is more data to send
		if(circBufRemainder(&uart_tx_buf) < uart_tx_buf.size_of_buffer)
		{
			UCA1TXBUF = circBufGetData(&uart_tx_buf);
		}
		else
		{
			UCA1IE &= ~UCTXIE;                        // Disable USCI_A0 TX interrupt
		}
		break;                             // Vector 4 - TXIFG
	default:
		break;
	}
}

#elif UART_SER_INTF == F2_UART_INTF_USCIA0        // Interface to UART 

/**********************************************************************//**
 * @brief  Initializes the serial communications peripheral and GPIO ports 
 *         to communicate with the TUSB3410.
 * 
 * @param  none
 * 
 * @return none
 **************************************************************************/
void halUartInit(void)
{
	uart_rx_buf.buffer = rx_buf;     // provide link to data structure to circ buffer
	uart_rx_buf.size_of_buffer = RX_UART_BUFFER_SIZE;

	uart_tx_buf.buffer = tx_buf;     // provide link to data structure to circ buffer
	uart_tx_buf.size_of_buffer = TX_UART_BUFFER_SIZE;

	uart_tx_buf.head_ptr++;			 // increment the head_ptr in the tx buffer

	rx_end_of_str = NO_END_OF_LINE_DETECTED;
	rx_str_length = 0;

	UART_PORT_SEL  |= UART_PIN_RXD + UART_PIN_TXD;
	UART_PORT_SEL2 |= UART_PIN_RXD + UART_PIN_TXD;
	UART_PORT_DIR |= UART_PIN_TXD;
	UART_PORT_DIR &= ~UART_PIN_RXD;

	UCA0CTL1 |= UCSWRST;                        //Reset State
	UCA0CTL0 = UCMODE_0;
	UCA0CTL0 &= ~UC7BIT;                        // 8bit char
	UCA0CTL1 |= UCSSEL_1;                       // ACLK
	UCA0BR0 = 0x03;                             // 9600 bits per second
	UCA0BR1 = 0x00;                             //
	UCA0MCTL = UCBRS0 + UCBRS1;                 // Modulation UCBRSx = 3

	UCA0CTL1 &= ~UCSWRST;
	IE2 |= UCA0RXIE;                            // Enable USCI_A0 RX interrupt

	__bis_SR_register(GIE);                     // Enable Interrupts
}

/***********************************************************************//**
 * @brief  Disables the serial communications peripheral and clears the GPIO
 *         settings, reset the interupts.
 **************************************************************************/
void halUartDeinit(void)
{
	UC0IE &= ~UCA0RXIE;
	UC0IE &= ~UCA0TXIE;
	UCA0CTL1 = UCSWRST;                                       //Reset State
	UART_PORT_SEL &= ~( UART_PIN_RXD + UART_PIN_TXD );
	UART_PORT_DIR |= UART_PIN_TXD;
	UART_PORT_DIR |= UART_PIN_RXD;
	UART_PORT_OUT &= ~(UART_PIN_TXD + UART_PIN_RXD);
}

/**********************************************************************//**
 * @brief  Start the TX ISR, it will automatically stop when FIFO is empty
 **************************************************************************/
void halUartStartTx(void)
{
	IE2 |= UCA0TXIE;                            // Enable USCI_A0 TX interrupt
	return;
}



/**********************************************************************//**
 * @brief  UART ISR
 **************************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI0RX_ISR)))
#endif
void USCI0RX_ISR(void)
{
	char tmp_uart_data;

	tmp_uart_data = UCA0RXBUF;

	if(circBufRemainder(&uart_rx_buf) > 0) {
		circBufPutData(&uart_rx_buf, tmp_uart_data);
	}
	if(uart_state == UART_ECHO_ON) {
		uartPutChar(tmp_uart_data);
	}
	// if its a "return" then activate main-loop
	if(tmp_uart_data == 13) {
		rx_end_of_str = END_OF_LINE_DETECTED;
		rx_str_length = uart_rx_buf.size_of_buffer - circBufRemainder(&uart_rx_buf);
		__bic_SR_register_on_exit(LPM3_bits);
	}
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0TX_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI0TX_ISR)))
#endif
void USCI0TX_ISR(void)
{
	// check if there is more data to send
	if(circBufRemainder(&uart_tx_buf) < uart_tx_buf.size_of_buffer) {
		UCA0TXBUF = circBufGetData(&uart_tx_buf);
	} else {
		IE2 &= ~UCA0TXIE;                          // Disable USCI_A0 TX interrupt
	}
}

#endif

/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/
