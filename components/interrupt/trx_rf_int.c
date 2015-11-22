//*****************************************************************************
//! @file trx_rf_int.c  
//!
//! @brief  Implementation file for radio interrupt interface 
//!          functions on Port 1, pin 7. The ISR is defined elsewhere
//!          and connected to the interrupt vector real time.
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
 * @addtogroup Interrupts
 * @{
 ******************************************************************************/

/******************************************************************************
 * INCLUDES
 */
#include "hal_spi_rf_trxeb.h"
#include <msp430.h>
#include "hal_types.h"
#include "hal_defs.h"
#include "trx_rf_int.h"
#include "hal_digio2.h"

/******************************************************************************
* CONSTANTS
*/

// Interrupt port and pin
#if defined (__MSP430F5438A__)
#define TRXEM_INT_PORT 1
#define TRXEM_INT_PIN  7
#define TRXEM_INT_PORT_IN P1IN
#elif defined (__MSP430F5529__)
#define TRXEM_INT_PORT 2
#define TRXEM_INT_PIN  3
#define TRXEM_INT_PORT_IN P2IN
#endif

/******************************************************************************
 * FUNCTIONS
 */
/***************************************************************************//**
 * @brief       Connects an ISR function to PORT1 interrupt vector and 
 *              configures the interrupt to be a high-low transition. 
 *
 * @param       pF is function pointer to ISR
 ******************************************************************************/
void
trxIsrConnect(ISR_FUNC_PTR pF)
{
  //uint8 pin_bitmask;
  digio io;
  io.port = TRXEM_INT_PORT; 
  io.pin  = TRXEM_INT_PIN;

  // Assigning ISR function
  halDigio2IntConnect(io, pF);

  halDigio2IntSetEdge(io, HAL_DIGIO_INT_FALLING_EDGE);
  return;
}


/***************************************************************************//**
 * @brief       Clears sync interrupt flag
 ******************************************************************************/
void
trxClearIntFlag(void)
{
  digio io;
  io.port = TRXEM_INT_PORT; 
  io.pin  = TRXEM_INT_PIN;
  halDigio2IntClear(io);
  return;
}


/***************************************************************************//**
 * @brief       Enables sync interrupt 
 ******************************************************************************/
void
trxEnableInt(void)
{
  digio io;
  io.port = TRXEM_INT_PORT; 
  io.pin  = TRXEM_INT_PIN;
  halDigio2IntEnable(io);
  return;
}


/***************************************************************************//**
 * @brief       Disables sync interrupt
 ******************************************************************************/
void
trxDisableInt(void)
{
  digio io;
  io.port = TRXEM_INT_PORT; 
  io.pin  = TRXEM_INT_PIN;
  halDigio2IntDisable(io);
  return;
}


/**************************************************************************//**
 * @brief       Reads the value of the sync pin.
 *
 * @return      status of the interrupt pin
 ******************************************************************************/
uint8
trxSampleSyncPin(void)
{
  return ((TRXEM_INT_PORT_IN & (0x01<<TRXEM_INT_PIN))>>TRXEM_INT_PIN);
}

/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/
