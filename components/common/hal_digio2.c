//*****************************************************************************
//! @file       hal_digio2.c
//! @brief      MSP430F5xxx Interrpts
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
* @addtogroup Interrupts
* @{
******************************************************************************/

/******************************************************************************
* INCLUDES
*/
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_board.h"
#include "hal_int.h"
#include "hal_digio.h"
#include "hal_digio2.h"

/******************************************************************************
* LOCAL VARIABLES
*/
static ISR_FUNC_PTR port1_isr_tbl[8] = {0};
static ISR_FUNC_PTR port2_isr_tbl[8] = {0};


/*******************************************************************************//**
 * @brief	connects ISR to GPIO interrupt
 *
 * @param	io		contains the port and pin number of the GPIO ::digio
 * @param	func	is pointer to the ISR function ::ISR_FUNC_PTR
 *
 * @return	HAL_DIGIO_OK	if successfully executed
 * @return	HAL_DIGIO_ERROR	if port does not supprt GPIO interrupts
 **********************************************************************************/
uint8
halDigio2IntConnect(digio io, ISR_FUNC_PTR func)
{
    istate_t volatile key;
    HAL_INT_LOCK(key);
    switch (io.port)
    {
        case 1: port1_isr_tbl[io.pin] = func; break;
        case 2: port2_isr_tbl[io.pin] = func; break;
        default: HAL_INT_UNLOCK(key); return(HAL_DIGIO_ERROR);
    }
    halDigio2IntClear(io);
    HAL_INT_UNLOCK(key);
    return(HAL_DIGIO_OK);
}


/*******************************************************************************//**
 * @brief	Enables interrupts on a GPIO port
 *
 * @param	io		contains the port and pin number of the GPIO ::digio
 *
 * @return	HAL_DIGIO_OK	if successfully executed
 * @return	HAL_DIGIO_ERROR	if port does not supprt GPIO interrupts
 **********************************************************************************/
uint8
halDigio2IntEnable(digio io)
{
    uint8 pin_bitmask = (1 << io.pin);
    switch (io.port)
    {
        case 1: P1IE |= pin_bitmask; break;
        case 2: P2IE |= pin_bitmask; break;
        default: return(HAL_DIGIO_ERROR);
    }
    return(HAL_DIGIO_OK);
}


/*******************************************************************************//**
 * @brief	Disables interrupts on a GPIO port
 *
 * @param	io		contains the port and pin number of the GPIO ::digio
 *
 * @return	HAL_DIGIO_OK	if successfully executed
 * @return	HAL_DIGIO_ERROR	if port does not supprt GPIO interrupts
 **********************************************************************************/
uint8 halDigio2IntDisable(digio io)
{
    uint8 pin_bitmask = (1 << io.pin);
    switch (io.port)
    {
        case 1: P1IE &= ~pin_bitmask; break;
        case 2: P2IE &= ~pin_bitmask; break;
        default: return(HAL_DIGIO_ERROR);
    }
    return(HAL_DIGIO_OK);
}


/*******************************************************************************//**
 * @brief	Clears interrupt flags on a GPIO port
 *
 * @param	io		contains the port and pin number of the GPIO ::digio
 *
 * @return	HAL_DIGIO_OK	if successfully executed
 * @return	HAL_DIGIO_ERROR	if port does not supprt GPIO interrupts
 **********************************************************************************/
uint8 halDigio2IntClear(digio io)
{
    uint8 pin_bitmask = (1 << io.pin);
    switch (io.port)
    {
        case 1: P1IFG &= ~pin_bitmask; break;
        case 2: P2IFG &= ~pin_bitmask; break;
        default: return(HAL_DIGIO_ERROR);
    }
    return(HAL_DIGIO_OK);
}


/*******************************************************************************//**
 * @brief	Configures GPIO interrupt for \e RISING_EDGE or \e FALLING_EDGE
 *
 * @param	io		contains the port and pin number of the GPIO ::digio
 * @param	edge	is type of the edge that triggers GPIO interrupt
 *
 * @return	HAL_DIGIO_OK	if successfully executed
 * @return	HAL_DIGIO_ERROR	if port does not supprt GPIO interrupts
 **********************************************************************************/
uint8 halDigio2IntSetEdge(digio io, uint8 edge)
{
    uint8 pin_bitmask = (1 << io.pin);
    switch(edge)
    {
        case HAL_DIGIO_INT_FALLING_EDGE:
            switch(io.port)
            {
                case 1: P1IES |= pin_bitmask; break;
                case 2: P2IES |= pin_bitmask; break;
                default: return(HAL_DIGIO_ERROR);
            }
            break;

         case HAL_DIGIO_INT_RISING_EDGE:
            switch(io.port)
            {
                case 1: P1IES &= ~pin_bitmask; break;
                case 2: P2IES &= ~pin_bitmask; break;
                default: return(HAL_DIGIO_ERROR);
            }
            break;

         default:
            return(HAL_DIGIO_ERROR);
    }
    return(HAL_DIGIO_OK);
}


/*******************************************************************************//**
 * @brief	PORT1 Interrupt Service Routine
 **********************************************************************************/
#pragma vector=PORT1_VECTOR
__interrupt void port1_ISR(void)
{
    register uint8 i;
    if (P1IFG)
    {
        for (i = 0; i < 8; i++)
        {
            register const uint8 bitmask = 1 << i;
            if ((P1IFG & bitmask) && (P1IE & bitmask) && (port1_isr_tbl[i] != 0))
            {
                (*port1_isr_tbl[i])();
                P1IFG &= ~bitmask;
            }
        }
        __low_power_mode_off_on_exit();
    }
}


/*******************************************************************************//**
 * @brief	PORT2 Interrupt Service Routine
 **********************************************************************************/
#pragma vector=PORT2_VECTOR
__interrupt void port2_ISR(void)
{
    register uint8 i;
    if (P2IFG)
    {
        for (i = 0; i < 8; i++)
        {
            register const uint8 bitmask = 1 << i;
            if ((P2IFG & bitmask) && (P2IE & bitmask) && (port2_isr_tbl[i] != 0))
            {
                (*port2_isr_tbl[i])();
                P2IFG &= ~bitmask;
            }
        }
        __low_power_mode_off_on_exit();
    }
}

/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
