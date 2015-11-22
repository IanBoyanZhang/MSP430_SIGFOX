//*****************************************************************************
//! @file       bsp_led.c
//! @brief      LED board support package for MSP430F5438A on SmartRF TrxEB.
//!
//! Revised     $Date: 2013-04-11 20:13:51 +0200 (to, 11 apr 2013) $
//! Revision    $Revision: 9716 $
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
#ifndef BSP_LED_EXCLUDE


/**************************************************************************//**
* @addtogroup BSP
* @{
******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#include "bsp_led.h"

#if defined(__MSP430F5438A__)

/******************************************************************************
* DEFINES
*/
#define BSP_LED_SEL             P4SEL
#define BSP_LED_DIR             P4DIR
#define BSP_LED_OUT             P4OUT


/******************************************************************************
* FUNCTIONS
*/
/**************************************************************************//**
* @brief    Initialize GPIO pins connected to LEDs.
*           LEDs are initialized to be off.
*
* @return   None
******************************************************************************/
void
bspLedInit(void)
{
    //
    // Set as GPIO output high
    //
    BSP_LED_OUT |= BSP_LED_ALL;
    BSP_LED_SEL &= ~BSP_LED_ALL;
    BSP_LED_DIR |= BSP_LED_ALL;
}


/**************************************************************************//**
* @brief    Sets LED(s) specified by \e ui8Leds. Must be run after bspLedInit().
*
* @param    ui8Leds      OR'ed bitmask of LEDs (for example \b BSP_LED_1).
*
* @return   None
******************************************************************************/
void
bspLedSet(uint8_t ui8Leds)
{
    //
    // Set pin(s) low
    //
    BSP_LED_OUT &= ~(ui8Leds & BSP_LED_ALL);
}


/**************************************************************************//**
* @brief    Clears LED(s) specified by \e ui8Leds.
*           Must be run after bspLedInit().
*
* @param    ui8Leds      OR'ed bitmask of LEDs (for example \b BSP_LED_1).
*
* @return   None
******************************************************************************/
void
bspLedClear(uint8_t ui8Leds)
{
    //
    // Set pin(s) high
    //
    BSP_LED_OUT |= (ui8Leds & BSP_LED_ALL);
}

/**************************************************************************//**
* @brief    Toggles LED(s) specified by \e ui8Leds.
*           Must be run after bspLedInit().
*
* @param    ui8Leds      OR'ed bitmask of LEDs (for example \b BSP_LED_1).
*
* @return   None
******************************************************************************/
void
bspLedToggle(uint8_t ui8Leds)
{
    //
    // Toggle pin(s)
    //
    BSP_LED_OUT ^= (ui8Leds & BSP_LED_ALL);
}

#elif defined(__MSP430F5529__)

/******************************************************************************
* DEFINES
*/
#define BSP_LED1_SEL             P1SEL
#define BSP_LED1_DIR             P1DIR
#define BSP_LED1_OUT             P1OUT

#define BSP_LED2_SEL             P4SEL
#define BSP_LED2_DIR             P4DIR
#define BSP_LED2_OUT             P4OUT

/******************************************************************************
* FUNCTIONS
*/
/**************************************************************************//**
* @brief    Initialize GPIO pins connected to LEDs.
*           LEDs are initialized to be off.
*
* @return   None
******************************************************************************/
void
bspLedInit(void)
{
    //
    // Set as GPIO output high
    //
    BSP_LED1_OUT |= BSP_LED_1;
    BSP_LED1_SEL &= ~BSP_LED_1;
    BSP_LED1_DIR |= BSP_LED_1;

    BSP_LED2_OUT |= BSP_LED_2;
    BSP_LED2_SEL &= ~BSP_LED_2;
    BSP_LED2_DIR |= BSP_LED_2;
}


/**************************************************************************//**
* @brief    Sets LED(s) specified by \e ui8Leds. Must be run after bspLedInit().
*
* @param    ui8Leds      OR'ed bitmask of LEDs (for example \b BSP_LED_1).
*
* @return   None
******************************************************************************/
void
bspLedSet(uint8_t ui8Leds)
{
    //
    // Set pin(s) low
    //
	if (ui8Leds == BSP_LED_1) {
		BSP_LED1_OUT |= (BSP_LED_1);
	}
	else if (ui8Leds == BSP_LED_2) {
		BSP_LED2_OUT |= (BSP_LED_2);
	}
}


/**************************************************************************//**
* @brief    Clears LED(s) specified by \e ui8Leds.
*           Must be run after bspLedInit().
*
* @param    ui8Leds      OR'ed bitmask of LEDs (for example \b BSP_LED_1).
*
* @return   None
******************************************************************************/
void
bspLedClear(uint8_t ui8Leds)
{
    //
    // Set pin(s) high
    //
	if (ui8Leds == BSP_LED_1) {
		BSP_LED1_OUT &= ~(BSP_LED_1);
	}
	else if (ui8Leds == BSP_LED_2) {
		BSP_LED2_OUT &= ~(BSP_LED_2);
	}
}

/**************************************************************************//**
* @brief    Toggles LED(s) specified by \e ui8Leds.
*           Must be run after bspLedInit().
*
* @param    ui8Leds      OR'ed bitmask of LEDs (for example \b BSP_LED_1).
*
* @return   None
******************************************************************************/
void
bspLedToggle(uint8_t ui8Leds)
{
    //
    // Toggle pin(s)
    //
	if (ui8Leds == BSP_LED_1) {
		BSP_LED1_OUT ^= (BSP_LED_1);
	}
	else if (ui8Leds == BSP_LED_2) {
		BSP_LED2_OUT ^= (BSP_LED_2);
	}
}

#endif
/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
#endif // #ifndef BSP_LED_EXCLUDE
