/*
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#define TX_UART_BUFFER_SIZE 60
#define RX_UART_BUFFER_SIZE 60

#define F2_UART_INTF_USCIA0  0xA0
#define F5_UART_INTF_USCIA0  0xA1
#define F5_UART_INTF_USCIA1  0xA2
#define FR5_UART_INTF_USCIA0 0xA3

/********************************************************************************
* Select which port will be used for interface to UART
* Valid selections are:
* F2_UART_INTF_USCIA0    LaunchPAD_G2553
* F5_UART_INTF_USCIA1    MSP430_TRXEB development kit
* F5_UART_INTF_USCIA1    LaunchPAD_F5529
* FR5_UART_INTF_USCIA0   LaunchPAD_FR5969
*******************************************************************************/
#if defined (__MSP430G2553__)
#define UART_SER_INTF  F2_UART_INTF_USCIA0
#endif

#if (defined (__MSP430F5438A__) || defined (__MSP430F5529__))
#define UART_SER_INTF  F5_UART_INTF_USCIA1
#endif


#if UART_SER_INTF == F2_UART_INTF_USCIA0        // Interface to UART
  #define UART_PORT_OUT      P1OUT
  #define UART_PORT_SEL      P1SEL
  #define UART_PORT_SEL2     P1SEL2
  #define UART_PORT_DIR      P1DIR
  #define UART_PORT_REN      P1REN
  #define UART_PIN_TXD       BIT2
  #define UART_PIN_RXD       BIT1
#elif UART_SER_INTF == F5_UART_INTF_USCIA0
  #define UART_PORT_OUT      P3OUT
  #define UART_PORT_SEL      P3SEL
  #define UART_PORT_DIR      P3DIR
  #define UART_PORT_REN      P3REN
  #define UART_PIN_TXD       BIT4
  #define UART_PIN_RXD       BIT5
#elif UART_SER_INTF == F5_UART_INTF_USCIA1
    #if defined(__MSP430F5438A__)
    #define UART_PORT_OUT      P5OUT
    #define UART_PORT_SEL      P5SEL
    #define UART_PORT_DIR      P5DIR
    #define UART_PORT_REN      P5REN
    #define UART_PIN_TXD       BIT6
    #define UART_PIN_RXD       BIT7
  #elif defined(__MSP430F5529__)
    #define UART_PORT_OUT      P4OUT
    #define UART_PORT_SEL      P4SEL
    #define UART_PORT_DIR      P4DIR
    #define UART_PORT_REN      P4REN
    #define UART_PIN_TXD       BIT4
    #define UART_PIN_RXD       BIT5
  #endif
#elif UART_SER_INTF == FR5x_UART_INTF_USCIA0
  #define UART_PORT_OUT      P2OUT
  #define UART_PORT_SEL0     P2SEL0
  #define UART_PORT_SEL1     P2SEL1
  #define UART_PORT_DIR      P2DIR
  #define UART_PORT_REN      P2REN
  #define UART_PIN_TXD       BIT0
  #define UART_PIN_RXD       BIT1
#endif

#define NO_END_OF_LINE_DETECTED   0
#define END_OF_LINE_DETECTED      1
#define UART_ECHO_OFF            10
#define UART_ECHO_ON             11

/*
 *  macro function for entering critical section (basically saving the
 *  current global maskable interrupt flag
 */
#define ENTER_CRITICAL_SECTION(int_flag)    do {                      \
                               int_flag = __get_SR_register() & GIE;  \
                               __bic_SR_register(GIE);                \
                               } while(0)

/*
 *  macro function for leaving critical section (restoring the global
 *  maskable interrupt flag from the las
 */
#define LEAVE_CRITICAL_SECTION(int_flag)    do {                      \
                               __bis_SR_register(int_flag);           \
                               }while(0)

/****************************************************************
 *    Low level functions to control the UART driver
 ***************************************************************/
void halUartInit(void);
void halUartDeinit(void);
void halUartStartTx(void);

/****************************************************************
 *    Main application level functions
 ***************************************************************/
void uartPutChar(char character);
unsigned charUartGetChar(void);
void uartGetStr(char *str, unsigned char length);
void uartPutStr(char *str, unsigned char length);

/****************************************************************
 *    Control functions of the UART driver
 ***************************************************************/
unsigned char uartGetTxBufSpace(void);
unsigned char uartGetRxStrLength(void);
unsigned char uartGetRxEndOfStr(void);
void uartResetRxEndOfStr(void);

/****************************************************************
 *  Enable and disable echoing of all RX'ed trafic to the TX
 ***************************************************************/
void uartDrvToggleEcho(void);
