//*****************************************************************************
//! @file       bsp_key.c
//! @brief      Key board support package for MSP430F5438A on TrxEB.
//!             Key presses can be handled using polling or interrupts, this
//!             can be switched run-time. The user may register custom ISRs
//!             using the bspKeyIntRegister() function.
//!
//!             If a custom ISR is registered, it will be called prior to
//!             starting the watchdog timer.
//!
//!             If \c BSP_KEY_NO_ISR is defined, key debounce will be
//!             implemented using active state debounce (ISR not possible).
//!             Functions this define is included to allow that the watchdog
//!             interrupt vector is not occupied by the key handler. When
//!             \c BSP_KEY_NO_ISR is defined, bspKeyPushed() and bspKeyGetDir()
//!             will poll the GPIO pins connected to the keys. Interrupt
//!             related functions will do nothing.
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
#ifndef BSP_KEY_EXCLUDE


/**************************************************************************//**
* @addtogroup BSP
* @{
******************************************************************************/

#define BSP_KEY_NO_ISR

/******************************************************************************
* INCLUDES
*/
#include "bsp.h"
#include "bsp_key.h"
#ifndef BSP_KEY_NO_ISR
#include "io_pin_int.h"         // Access to GPIO pin specific ISRs
#endif // BSP_KEY_NO_ISR


/******************************************************************************
* DEFINES
*/

#if defined(__MSP430F5438A__)
//! Number of keys on board.
#define BSP_KEY_COUNT       5
//! Active wait debounce macro
#define BSP_KEY_DEBOUNCE(expr)  { uint16_t i; for (i=0; i<500; i++) {         \
                                  if (!(expr)) i = 0; } }
#define BSP_KEY_DIR             P2DIR
#define BSP_KEY_SEL             P2SEL
#define BSP_KEY_OUT             P2OUT
#define BSP_KEY_IN              P2IN
#define BSP_KEY_REN             P2REN       //!< Resistor enable register
#define BSP_KEY_IE              P2IE        //!< Interrupt enable register
#define BSP_KEY_IFG             P2IFG       //!< Interrupt flag register

#elif defined(__MSP430F5529__)
//! Number of key on board.
#define BSP_KEY_COUNT		2
//! Active wait debounce macro
#define BSP_KEY_DEBOUNCE(expr)  { uint16_t i; for (i=0; i<500; i++) {         \
                                  if (!(expr)) i = 0; } }

#define BSP_KEY_1_DIR             P2DIR
#define BSP_KEY_1_SEL             P2SEL
#define BSP_KEY_1_OUT             P2OUT
#define BSP_KEY_1_IN              P2IN
#define BSP_KEY_1_REN             P2REN       //!< Resistor enable register

#define BSP_KEY_2_DIR             P1DIR
#define BSP_KEY_2_SEL             P1SEL
#define BSP_KEY_2_OUT             P1OUT
#define BSP_KEY_2_IN              P1IN
#define BSP_KEY_2_REN             P1REN       //!< Resistor enable register

#endif

/******************************************************************************
* LOCAL VARIABLES AND FUNCTION PROTOTYPES
*/
static uint8_t ui8BspKeyMode;
static volatile uint8_t bspKeysPressed;
static volatile uint8_t bspKeyIntDisabledMask;

#ifndef BSP_KEY_NO_ISR
static void (*bspKeysIsrTable[BSP_KEY_COUNT])(void);
static void bspKeyPushedISR(void);
__interrupt void bspKeyWdtISR(void);
#endif // BSP_KEY_NO_ISR


/******************************************************************************
* FUNCTIONS
*/
/**************************************************************************//**
* @brief    This function initializes key GPIO as input pullup and disables
*           interrupts. If \e ui8Mode is \b BSP_KEY_MODE_POLL, key presses are
*           handled using polling and active state debounce. Functions starting
*           with \b bspKeyInt then do nothing.
*
*           If \e ui8Mode is \b BSP_KEY_MODE_ISR, key presses are handled by
*           interrupts, and debounce is implemented using the watchdog timer.
*
*           @note If \b BSP_KEY_NO_ISR is defined, parameter \e ui8Mode
*           is ignored and key presses are handled using polling and active
*           state debounce. In this case the watchdog timer interrupt vector
*           is not assigned to an interrupt handler.
*
* @param    ui8Mode is the operating; it must be one of the following:
*                   \li \b BSP_KEY_MODE_POLL for polling-based handling
*                   \li \b BSP_KEY_MODE_ISR for interrupt-based handling
******************************************************************************/
void
bspKeyInit(uint8_t ui8Mode)
{
    //
    // Store mode
    //
#ifndef BSP_KEY_NO_ISR
    ui8BspKeyMode = ui8Mode;
#else
    ui8BspKeyMode = BSP_KEY_MODE_POLL;
#endif


#if defined(__MSP430F5428A__)
    //
    // Initialize keys as GPIO input pullup:
    //
    BSP_KEY_SEL &= ~(BSP_KEY_ALL);
    BSP_KEY_DIR &= ~(BSP_KEY_ALL);
    BSP_KEY_OUT |= BSP_KEY_ALL;
    BSP_KEY_REN |= BSP_KEY_ALL;
#elif defined(__MSP430F5529__)
    //
    // Initialize keys as GPIO input pullup:
    //
    BSP_KEY_1_SEL &= ~(BSP_KEY_1);
    BSP_KEY_1_DIR &= ~(BSP_KEY_1);
    BSP_KEY_1_OUT |= BSP_KEY_1;
    BSP_KEY_1_REN |= BSP_KEY_1;

    BSP_KEY_2_SEL &= ~(BSP_KEY_2);
    BSP_KEY_2_DIR &= ~(BSP_KEY_2);
    BSP_KEY_2_OUT |= BSP_KEY_2;
    BSP_KEY_2_REN |= BSP_KEY_2;
#endif

#ifndef BSP_KEY_NO_ISR
    if(ui8BspKeyMode == BSP_KEY_MODE_ISR)
    {
        //
        // Disable interrupts on key pins and clear interrupt flags
        //
        BSP_KEY_IE  &= ~(BSP_KEY_ALL);
        BSP_KEY_IFG &= ~(BSP_KEY_ALL);

        //
        // Connect bspKeyPushedISR() to key pins
        //
        ioPinIntRegister(IO_PIN_PORT_2, BSP_KEY_ALL, &bspKeyPushedISR);

        //
        // Set trigger type
        //
        ioPinIntTypeSet(IO_PIN_PORT_2, BSP_KEY_ALL, IO_PIN_RISING_EDGE);
    }
#endif // BSP_KEY_NO_ISR
}


/**************************************************************************//**
* @brief    This function returns a bitmask of keys pushed.
*
* @note     If keys are handled using polling (\b BSP_KEY_MODE_POLL), the
*           returned bitmask will never contain a combination of multiple key
*           bitmasks, for example, (\b BSP_KEY_LEFT |\b BSP_KEY_UP).
*           Furthermore, in this case argument \e ui8ReadMask is ignored.
*
* @param    ui8ReadMask     is a bitmask of keys to read. Read keys are cleared
*                           and new key presses can be registered. Use
*                           \b BSP_KEY_ALL to read status of all keys.
*
* @return   Returns bitmask of pushed keys
******************************************************************************/
uint8_t
bspKeyPushed(uint8_t ui8ReadMask)
{
    if(ui8BspKeyMode == BSP_KEY_MODE_POLL)
    {
        //
        // Polling mode.
        //
#ifdef __MSP430F5438A__
        //
        // Get key state bitmask
        //
        uint_fast8_t ui8Pins = ((~BSP_KEY_IN) & BSP_KEY_ALL);

        //
        // Return the first key pressed
        //
        if(ui8Pins & BSP_KEY_SELECT)
        {
            BSP_KEY_DEBOUNCE(BSP_KEY_IN & BSP_KEY_SELECT);
            return (BSP_KEY_SELECT);
        }
        else if(ui8Pins & BSP_KEY_LEFT)
        {
            BSP_KEY_DEBOUNCE(BSP_KEY_IN & BSP_KEY_LEFT);
            return (BSP_KEY_LEFT);
        }
        else if(ui8Pins & BSP_KEY_RIGHT)
        {
            BSP_KEY_DEBOUNCE(BSP_KEY_IN & BSP_KEY_RIGHT);
            return (BSP_KEY_RIGHT);
        }
        else if(ui8Pins & BSP_KEY_UP)
        {
            BSP_KEY_DEBOUNCE(BSP_KEY_IN & BSP_KEY_UP);
            return (BSP_KEY_UP);
        }
        else if(ui8Pins & BSP_KEY_DOWN)
        {
            BSP_KEY_DEBOUNCE(BSP_KEY_IN & BSP_KEY_DOWN);
            return (BSP_KEY_DOWN);
        }
#elif defined (__MSP430F5529__)

        //
        // Check if the key is pressed
        //
        if((~BSP_KEY_1_IN) & BSP_KEY_1)
        {
        	BSP_KEY_DEBOUNCE(BSP_KEY_1_IN & BSP_KEY_1);
            return (BSP_KEY_SELECT);
        }
        else if((~BSP_KEY_2_IN) & BSP_KEY_2)
        {
        	BSP_KEY_DEBOUNCE(BSP_KEY_2_IN & BSP_KEY_2);
        	return (BSP_KEY_UP);
        }
#endif

        //
        // No keys pressed
        //
        return (0);
    }
#ifndef BSP_KEY_NO_ISR
    else
    {
        uint_fast8_t ui8Bm = 0;
        //
        // Disable global interrupts
        //
        uint16_t ui16IntState = __get_interrupt_state();
        __disable_interrupt();

        //
        // Critical section
        //
        ui8Bm = bspKeysPressed;
        bspKeysPressed &= ~ui8ReadMask;

        //
        // Re-enable interrupt if initially enabled, and return key bitmask
        //
        __set_interrupt_state(ui16IntState);
        return (ui8Bm);
    }
#else
    else
    {
        //
        // If we get here, something is configured wrong (ISR mode chosen _and_
        // BSP_KEY_NO_ISR defined)
        //
        return (0);
    }
#endif // BSP_KEY_NO_ISR
}

#ifdef __MSP430F5438A__
/**************************************************************************//**
* @brief    This function reads the directional event. If multiple keys are
*           registered as "pressed", this function will only return the
*           directional event of the first key. Remaining key events will
*           be ignored. \sa bspKeyPushed()
*
* @return   Returns \b BSP_KEY_EVT_LEFT if LEFT key has been pressed.
* @return   Returns \b BSP_KEY_EVT_RIGHT if RIGHT key has been pressed.
* @return   Returns \b BSP_KEY_EVT_UP if UP key has been pressed.
* @return   Returns \b BSP_KEY_EVT_DOWN if DOWN key has been pressed.
* @return   Returns \b BSP_KEY_EVT_NONE if no key has been pressed.
******************************************************************************/
uint8_t
bspKeyGetDir(void)
{
    //
    // Get bitmask of keys pushed
    //
    uint_fast8_t ui8Bitmask = bspKeyPushed(BSP_KEY_ALL);

    if(ui8Bitmask & BSP_KEY_LEFT)
    {
        return (BSP_KEY_EVT_LEFT);
    }
    else if(ui8Bitmask & BSP_KEY_RIGHT)
    {
        return (BSP_KEY_EVT_RIGHT);
    }
    else if(ui8Bitmask & BSP_KEY_UP)
    {
        return (BSP_KEY_EVT_UP);
    }
    else if(ui8Bitmask & BSP_KEY_DOWN)
    {
        return (BSP_KEY_EVT_DOWN);
    }
    else
    {
        return (BSP_KEY_EVT_NONE);
    }
}
#endif

/**************************************************************************//**
* @brief    This function registers a custom ISR to keys specified by
*           \e ui8Keys.
*
* @note     If bspKeyInit() was initialized with argument \b BSP_KEY_MODE_POLL,
*           this function does nothing.
*
* @param    ui8Keys     is an ORed bitmask of keys (for example BSP_KEY_1).
* @param    pfnHandler  is a void function pointer to ISR.
*
* @return   None
******************************************************************************/
void
bspKeyIntRegister(uint8_t ui8Keys, void (*pfnHandler)(void))
{
#ifndef BSP_KEY_NO_ISR
    if(ui8BspKeyMode == BSP_KEY_MODE_ISR)
    {
        if(ui8Keys & BSP_KEY_SELECT)
        {
            bspKeysIsrTable[0] = pfnHandler;
        }
        if(ui8Keys & BSP_KEY_LEFT)
        {
            bspKeysIsrTable[1] = pfnHandler;
        }
        if(ui8Keys & BSP_KEY_RIGHT)
        {
            bspKeysIsrTable[2] = pfnHandler;
        }
        if(ui8Keys & BSP_KEY_UP)
        {
            bspKeysIsrTable[3] = pfnHandler;
        }
        if(ui8Keys & BSP_KEY_DOWN)
        {
            bspKeysIsrTable[4] = pfnHandler;
        }
    }
#endif // BSP_KEY_NO_ISR
}


/**************************************************************************//**
* @brief    This function clears the custom ISR from keys specified by
*           \e ui8Keys.
*
* @note     If bspKeyInit() was initialized with argument \b BSP_KEY_MODE_POLL,
*           this function does nothing.
*
* @param    ui8Keys     is an ORed bitmask of keys (for example BSP_KEY_1).
*
* @return   None
******************************************************************************/
void
bspKeyIntUnregister(uint8_t ui8Keys)
{
#ifndef BSP_KEY_NO_ISR
    bspKeyIntRegister(ui8Keys, 0);
#endif // BSP_KEY_NO_ISR
}


/**************************************************************************//**
* @brief    This function enables interrupts on specified key GPIO pins.
*
* @note     If bspKeyInit() was initialized with argument \b BSP_KEY_MODE_POLL,
*           this function does nothing.
*
* @param    ui8Keys     is an ORed bitmask of keys (for example BSP_KEY_1).
*
* @return   None
******************************************************************************/
void
bspKeyIntEnable(uint8_t ui8Keys)
{
#ifndef BSP_KEY_NO_ISR
    if(ui8BspKeyMode == BSP_KEY_MODE_ISR)
    {
        //
        // Enable interrupt for pins:
        //
        ioPinIntEnable(IO_PIN_PORT_2, (ui8Keys & BSP_KEY_ALL));
    }
#endif
}


/**************************************************************************//**
* @brief    This function disables interrupts on specified key GPIOs.
*
* @note     If bspKeyInit() was initialized with argument \b BSP_KEY_MODE_POLL,
*           this function does nothing.
*
* @param    ui8Keys     is an ORed bitmask of keys (for example BSP_KEY_1).
*
* @return   None
******************************************************************************/
void
bspKeyIntDisable(uint8_t ui8Keys)
{
#ifndef BSP_KEY_NO_ISR
    if(ui8BspKeyMode == BSP_KEY_MODE_ISR)
    {
        //
        // Disable interrupt for pins:
        //
        ioPinIntDisable(IO_PIN_PORT_2, (ui8Keys & BSP_KEY_ALL));
    }
#endif
}


/**************************************************************************//**
* @brief    This function clears interrupt flags on selected key GPIOs.
*
* @note     If bspKeyInit() was initialized with argument \b BSP_KEY_MODE_POLL,
*           this function does nothing.
*
* @param    ui8Keys     is an ORed bitmask of keys (for example BSP_KEY_1).
*
* @return   None
******************************************************************************/
void
bspKeyIntClear(uint8_t ui8Keys)
{
#ifndef BSP_KEY_NO_ISR
    if(ui8BspKeyMode == BSP_KEY_MODE_ISR)
    {
        //
        // Clear interrupt flag for selected pins
        //
        ioPinIntClear(IO_PIN_PORT_2, (ui8Keys & BSP_KEY_ALL));
    }
#endif
}


/******************************************************************************
* LOCAL FUNCTIONS
*/
/**************************************************************************//**
* @brief    Interrupt Service Routine for an activated directional key.
*           Stores the pin where the interrupt occured, disables the interrupt
*           on that pin and starts the debouncing by use of WDT.
*
* @return   None
******************************************************************************/
#ifndef BSP_KEY_NO_ISR
static void
bspKeyPushedISR(void)
{
    uint8_t ui8IrqBm;
    uint16_t ui16IntState;

    //
    // Disable WDT interrupt and stop WDT

    SFRIE1 &= ~WDTIE;
    WDTCTL = WDTPW + WDTHOLD;

    //
    // Critical section
    //
    ui16IntState = __get_interrupt_state();
    __disable_interrupt();

    //
    // Get interrupt flags for keys with interrupt enabled, and store info
    //
    ui8IrqBm = (BSP_KEY_IFG & BSP_KEY_ALL);
    ui8IrqBm &= BSP_KEY_IE;
    bspKeysPressed |= ui8IrqBm;
    bspKeyIntDisabledMask |= ui8IrqBm;

    //
    // End critical section, set interrupt state back to previous
    //
    __set_interrupt_state(ui16IntState);

    //
    // Disable interrupts on keys where interrupt flag was set
    //
    ioPinIntDisable(IO_PIN_PORT_2, bspKeyIntDisabledMask);

    //
    // Run custom ISR if any (unrolled for speed)
    //
    if((ui8IrqBm & BSP_KEY_SELECT) && (bspKeysIsrTable[0] != 0))
    {
        (*bspKeysIsrTable[0])();
    }
    if((ui8IrqBm & BSP_KEY_LEFT) && (bspKeysIsrTable[1] != 0))
    {
        (*bspKeysIsrTable[1])();
    }
    if((ui8IrqBm & BSP_KEY_RIGHT) && (bspKeysIsrTable[2] != 0))
    {
        (*bspKeysIsrTable[2])();
    }
    if((ui8IrqBm & BSP_KEY_UP) && (bspKeysIsrTable[3] != 0))
    {
        (*bspKeysIsrTable[3])();
    }
    if((ui8IrqBm & BSP_KEY_DOWN) && (bspKeysIsrTable[4] != 0))
    {
        (*bspKeysIsrTable[4])();
    }

    //
    // Clear pending WDT interrupt flag, start WDT in timer mode (250 ms
    // interval counter when ACLK is 32768 Hz), and enable WDT interrupts.
    //
    SFRIFG1 &= ~WDTIFG;
    WDTCTL = WDTPW + WDTSSEL_1 + WDTTMSEL + WDTCNTCL + WDTIS_5;
    SFRIE1 |= WDTIE;
}


/**************************************************************************//**
* @brief    Interrupt Service Routine for an activated key.
*           Stores the pin where the interrupt occured, disables the interrupt
*           on that pin and starts the debouncing by use of WDT.
*
* @return   None
******************************************************************************/
#pragma vector=WDT_VECTOR
__interrupt void
bspKeyWdtISR(void)
{
    //
    // Clear WDT interrupt flag, disable interrupts and stop WDT
    //
    SFRIFG1 &= ~WDTIFG;
    SFRIE1 &= ~WDTIE;
    WDTCTL = WDTPW + WDTHOLD;

    //
    // Clear pending interrupts
    //
    ioPinIntClear(IO_PIN_PORT_2, bspKeyIntDisabledMask);

    //
    // Re-enable the pin interrupts
    //
    ioPinIntEnable(IO_PIN_PORT_2, bspKeyIntDisabledMask);

    //
    // Clear bitmask of which keys are disabled. Atomic, no interrupts need be
    // disabled.
    //
    bspKeyIntDisabledMask = 0;
}
#endif // BSP_KEY_NO_ISR


/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
#endif // #ifndef BSP_KEY_EXCLUDE
