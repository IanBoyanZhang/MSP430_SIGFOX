//*****************************************************************************
//! @file       lcd_trxeb.c
//! @brief      TrxEB specific implementation for DOGM128-6 LCD display driver.
//!
//! Revised     $Date: 2013-04-19 15:52:03 +0200 (to, 19 apr 2013) $
//! Revision    $Revision: 7290 $
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
#ifndef __MSP430F5438A__
#define LCD_EXCLUDE
#endif

#ifndef LCD_EXCLUDE


/**************************************************************************//**
* @addtogroup LCD
* @{
******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#include "bsp.h"
#include "lcd_dogm128_6.h"


/******************************************************************************
* DEFINES
*/
/* Port to use for display */
#define LCD_CS_A0_OUT           P9OUT
#define LCD_CS_A0_DIR           P9DIR
#define LCD_CS_A0_SEL           P9SEL

#define LCD_RST_OUT             P7OUT
#define LCD_RST_DIR             P7DIR
#define LCD_RST_SEL             P7SEL

#define LCD_SPI_BUS_SEL         P9SEL

#define LCD_PWR_OUT             P7OUT
#define LCD_PWR_DIR             P7DIR
#define LCD_PWR_SEL             P7SEL
#define LCD_PWR_DS              P7DS

//! Condition for SPI TX-buffer to be ready for new data
#define LCD_TX_BUF_READY()      (UCB2IFG & UCTXIFG)

//! Condition for SPI transmission to be completed.
#define LCD_TX_BUSY()           (UCB2STAT & UCBUSY)

//! Macro for asserting LCD CSn (set low)
#define LCD_SPI_BEGIN()         (LCD_CS_A0_OUT &= ~(BSP_LCD_CS_N))

//! Macro for deasserting LCD CSn (set high)
#define LCD_SPI_END()           (LCD_CS_A0_OUT |= BSP_LCD_CS_N)

//! Macro for setting LCD mode signal low (command)
#define LCD_MODE_SET_CMD()      (LCD_CS_A0_OUT &= ~BSP_LCD_MODE)

//! Macro for setting LCD mode signal (data)
#define LCD_MODE_SET_DATA()     (LCD_CS_A0_OUT |= BSP_LCD_MODE)

//
//! LCD initialization command sequence
//
static const char lcdInitCmd[] =
{
    0x40,   /*Display start line 0                    */
    0xa1,   /*ADC reverse, 6 oclock viewing direction */
    0xc0,   /*Normal COM0...COM63                     */
    0xa6,   /*Display normal, not mirrored            */
    0xa2,   /*Set Bias 1/9 (Duty 1/65)                */
    0x2f,   /*Booster, Regulator and Follower On      */
    0xf8,   /*Set internal Booster to 4x              */
    0x00,   /*                                        */
    0x27,   /*Contrast set                            */
    0x81,   /*                                        */
    0x16,   /* <- use value from LCD-MODULE .doc guide*/
    /*    for better contrast (not 0x10)      */
    0xac,   /*No indicator                            */
    0x00,   /*                                        */
    0xaf,   /*Display on                              */
    0xb0,   /*Page 0 einstellen                       */
    0x10,   /*High-Nibble of column address           */
    0x00    /*Low-Nibble of column address            */
};


/******************************************************************************
 * LOCAL VARIABLES
 */


/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */
static void lcdSendArray(char* pcArray, uint16_t ui16Size);


/**************************************************************************//**
* @brief    Initializes the LCD. This function assumes that the SPI
*           interface has already been initialized using e.g. bspIoSpiInit().
*           It must be run before you can use the LCD.
*
* @return   None
******************************************************************************/
void
lcdInit(void)
{
    //
    // IO pins
    //
    LCD_CS_A0_SEL &= ~(BSP_LCD_CS_N | BSP_LCD_MODE);
    LCD_RST_SEL   &= ~(BSP_LCD_RST);
    LCD_PWR_SEL   &= ~(BSP_LCD_PWR);

    //
    // Output
    //
    LCD_CS_A0_DIR |= (BSP_LCD_CS_N | BSP_LCD_MODE);
    LCD_RST_DIR   |= (BSP_LCD_RST);
    LCD_PWR_DIR   |= (BSP_LCD_PWR);

    //
    // Set BSP_LCD_PWR=1 with high drive strength
    //
    LCD_PWR_OUT |= (BSP_LCD_PWR);
    LCD_PWR_DS  |= (BSP_LCD_PWR);

    //
    // Set RSTn=0, A0=1, CSn=1
    //
    LCD_CS_A0_OUT |= (BSP_LCD_CS_N | BSP_LCD_MODE);
    LCD_RST_OUT   &= ~(BSP_LCD_RST);

    //
    // Wait ~ 100 ms (@ 16 MHz) and clear reset
    //
    __delay_cycles(1600000);
    LCD_RST_OUT   |= (BSP_LCD_RST);

    //
    // Send init command sequence
    //
    lcdSendCommand((char*)lcdInitCmd, sizeof(lcdInitCmd));
}


/**************************************************************************//**
* @brief    Clears the LCD display. This function acts directly on the display.
*
* @return   None
******************************************************************************/
void
lcdClear(void)
{
    uint16_t ui16Length;
    uint8_t ui8Page;

    //
    // For each page
    //
    for(ui8Page = 0; ui8Page < LCD_PAGE_ROWS; ui8Page++)
    {
        //
        // Set pointer
        //
        lcdGotoXY(0, ui8Page);

        ui16Length = LCD_COLS;

        //
        // Tell LCD data is transferred. Assert CSn
        //
        LCD_MODE_SET_DATA();
        LCD_SPI_BEGIN();

        //
        // Write data
        //
        while(ui16Length--)
        {
            //
            // Wait for Transmit buffer ready
            //
            while(!LCD_TX_BUF_READY())
            {
            }

            //
            // Send data control
            //
            UCB2TXBUF = 0x00;
        }

        //
        //  Wait for transfer to complete
        //
        while(LCD_TX_BUSY())
        {
        }

        //
        // Deassert CSn
        //
        LCD_SPI_END();
    }

    //
    // Clean IN FIFO
    //
    UCB2IFG &= ~UCRXIFG;
}


/**************************************************************************//**
* @brief    Initializes the LCD SPI interface to the max allowed speed based
*           on the system clock speed set by bspInit(). Quicker for
*           reconfiguring the SPI interface than lcdInit() if the LCD display
*           has already been initialized.
*
* @return   None
******************************************************************************/
void
lcdSpiInit(void)
{
    bspIoSpiInit(BSP_LCD_SPI, (bspSysClockSpeedGet() * 1000000));
}


/**************************************************************************//**
* @brief       Sends an array of length \e ui8Len of commands to the LCD
*              controller.
*
* @param       pcCmd    The array of commands to be sent to the LCD
* @param       ui8Len   Number of elements/bytes in command
*
* @return      none
******************************************************************************/
void
lcdSendCommand(const char *pcCmd, uint8_t ui8Len)
{
    //
    // Assert CSn, indicate command (A0 low), send bytes, deassert CSn
    //
    LCD_SPI_BEGIN();
    LCD_MODE_SET_CMD();
    lcdSendArray((char*)pcCmd, ui8Len);
    LCD_SPI_END();
}


/**************************************************************************//**
* @brief       Sends an array of \e ui8Len size of data to the LCD.
*
* @param       pcCmd    The array of commands to be sent to the LCD
* @param       ui8Len   Number of elements/bytes in command
*
* @return      none
******************************************************************************/
void
lcdSendData(const char *pcCmd, uint16_t ui16Len)
{
    //
    // Assert CSn, indicate data (A0 high), send bytes, deassert CSn)
    //
    LCD_SPI_BEGIN();
    LCD_MODE_SET_DATA();
    lcdSendArray((char*)pcCmd, ui16Len);
    LCD_SPI_END();
}


/**************************************************************************//**
* @brief    Function updates the LCD display by creating an animated transition
*           between two displays. Two animations exists, \b LCD_SLIDE_LEFT and
*           \b LCD_SLIDE_RIGHT whichs slides the new screen in leftwards or
*           rightwards, respectively.
*
* @details  When changing the image on the display with lcdBuffer functions,
*           the buffer is changed immediately. The changes will take affect
*           on the LCD when lcdSendBuffer or lcdSendBufferAnimated are used.
*           lcdSendBuffer will change the display to show the new buffer
*           instantanously. lcdSendBufferAnimated on the other side, will make a
*           smooth transition into showing the new buffer. In order for
*           halLcdAnimate to know what to animate from, it takes in an
*           argument \e pcFromBuffer. It should point to an address containing
*           what was stored on the LCD buffer last time lcdSendBuffer or
*           lcdSendBufferAnimated was used, or in other words, what is presently
*           on the display. This way, lcdSendBufferAnimated will not take any
*           memory unless used.
*
* @details  Example how to think:
*           1. Send a buffer to the display using e.g. lcdSendBuffer.
*           2. Manipulate a second buffer using lcdBuffer functions.
*           3. Run lcdSendBufferAnimated to update display with a
*              smooth transition from the initial to the second buffer.
*
* @param    pcToBuffer      Address to buffer with new display content
* @param    pcFromBuffer    Address to buffer with existing display content
* @param    iMotion         Which animation to use for transition. Can be one
*                           of the following:
*                           \li \b eLcdSlideLeft
*                           \li \b eLcdSlideRight
*
* @return   None
******************************************************************************/
void
lcdSendBufferAnimated(const char *pcToBuffer, const char *pcFromBuffer,
                      tLcdMotion iMotion)
{
    volatile uint32_t ui32Cnt;
    char pcPageData[LCD_COLS];
    uint8_t ui8Offset, ui8PageIndex, ui8I, ui8WaitCycles;
    char *pcToBuf = (char *)pcToBuffer;
    char *pcFromBuf = (char *)pcFromBuffer;
    uint8_t ui8WaitCyclesMax = (bspSysClockSpeedGet() / 1000000UL);

    //
    // Buffers are the same, do not animate
    //
    if(pcToBuffer == pcFromBuffer)
    {
        lcdSendBuffer(pcToBuffer);
        return;
    }

#ifndef LCD_NO_DEFAULT_BUFFER
    //
    // Use default buffer if null pointers
    //
    if(!pcToBuf)
    {
        pcToBuf = lcdDefaultBuffer;
    }
    else if(!pcFromBuf)
    {
        pcFromBuf = lcdDefaultBuffer;
    }
#endif // LCD_NO_DEFAULT_BUFFER

    for(ui8Offset = 0; ui8Offset <= LCD_COLS; ui8Offset += 4)
    {
        //
        // For each page
        //
        for(ui8PageIndex = 0; ui8PageIndex < 8; ui8PageIndex++)
        {
            //
            //  Assigning data to this page from both buffers
            //
            for(ui8I = 0; ui8I < LCD_COLS; ui8I++)
            {
                if(iMotion == eLcdSlideLeft)
                {
                    if(ui8I + ui8Offset < LCD_COLS)
                    {
                        pcPageData[ui8I] = *(pcFromBuf +                      \
                                             (ui8PageIndex * LCD_COLS +          \
                                              ui8I + ui8Offset));
                    }
                    else
                    {
                        pcPageData[ui8I] = *(pcToBuf +                        \
                                             (ui8PageIndex * LCD_COLS +          \
                                              ui8I + ui8Offset - LCD_COLS));
                    }
                }
                else
                {
                    if(ui8I - ui8Offset >= 0)
                    {
                        pcPageData[ui8I] = *(pcFromBuf +                      \
                                             (ui8PageIndex * LCD_COLS +          \
                                              ui8I - ui8Offset));
                    }
                    else
                    {
                        pcPageData[ui8I] = *(pcToBuf +                        \
                                             (ui8PageIndex * LCD_COLS +          \
                                              ui8I - ui8Offset + LCD_COLS));
                    }
                }
            }

            //
            // Set pointer to start of row/page and send page
            //
            lcdGotoXY(0, ui8PageIndex);
            lcdSendData(pcPageData, LCD_COLS);
        }

        //
        // Active state wait (adds ~1.5ms)
        //
        ui8WaitCycles = 0;
        do
        {
            __delay_cycles(1500);
        }
        while(ui8WaitCycles++ < ui8WaitCyclesMax);
    }
}

/******************************************************************************
* LOCAL FUNCTIONS
*/
/**************************************************************************//**
* @brief    Sends an array of length i of either data or commands (depending
*           on A0) on SPI to the LCD controller. Used as in the
*           functions lcdSendCommand and lcdSendData.
*
* @param    pcArray     The array of data or commands to be sent to the LCD
* @param    ui16Size    Number of elements/bytes in pArray
*
* @return   none
******************************************************************************/
static void
lcdSendArray(char* pcArray, uint16_t ui16Size)
{
    while(ui16Size--)
    {
        //
        // Wait for TX buffe to be ready, send control data and increment
        // data pointer.
        //
        while(!LCD_TX_BUF_READY());
        UCB2TXBUF = *pcArray;
        pcArray++;
    }

    //
    // Wait for transmission to complete before emptying RX buffer
    //
    while(LCD_TX_BUSY());
    UCB2IFG &= ~UCRXIFG;
}


/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
#endif // #ifndef LCD_EXCLUDE

