//*****************************************************************************
//! @file       device_config.h
//! @brief      Hardware definitions for the compiler
//!
//!
//!
//	Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
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
 * @addtogroup Config
 * @{
 ******************************************************************************/
#ifndef HWCONFIG
#define HWCONFIG


/*!
 * \brief Frequency Standard. Available options:
 * \li \b	MODE_FCC
 * \li \b	MODE_ETSI
 */
#define MODE_FCC

/*!
 * \brief CC112X EVM Selection. Available options:
 *  \li \b CC1125EM_CAT1_868
 *  \li \b CC1120_BOOSTERPACK
 *  \li \b OTHER for custom boards
 */
#define CC1120_BOOSTERPACK


/*!
 * \brief Demo Control Interface. Available options:
 * 	\li \b AT_CMD for AT-commands over UART to control from a host
 * 	\li \b PB_KEY for Push Button triggered interface controlled by MSP430
 * \note Both interfaces can be defined simultaneously.
 */
#define PB_KEY
#define AT_CMD

/*!
 * \brief This is the value of the external oscillator connected to CC112X
 *	between XOSC_Q1(Pin 30) and XOSC_Q2(Pin31). Choose from the following
 *	values depending on hardware configuration.
 *	\li \b	RF_XTAL_FREQ_40MHZ
 *	\li \b	RF_XTAL_FREQ_32MHZ
 */
#if defined(CC1125EM_CAT1_868)
#define RF_XTAL_FREQ_40MHZ	/*<! This board has a 40MHz XTAL*/
#elif defined(CC1120_BOOSTERPACK)
#define RF_XTAL_FREQ_32MHZ	/*<! This board has a 32MHz XTAL */
#define CC1190_PA_LNA		/*<! Boosterpack has CC1190 PA/LNA */
#elif defined(OTHER)		/* Other hardware -> manually define XTAL value */
// define the XTAL frequency used in the custom board
#define RF_XTAL_FREQ_40MHZ
// define if using CC1190 PA/LNA
#define CC1190_PA_LNA
#else
#error RF_XTAL_FREQ must be defined in apps/device_config.h
#endif

/*
 * 	example:
 * 	For		tranmit frequency 902.8MHz, receive frequency 904MHz
 *		#define		ftx		902800000
 *		#define 	frx		904000000
 */
/*!
 *  \def ftx
 * 	\brief TX carrier frequency in Hz
 */
/*!
 *  \def frx
 *  \brief RX carrier frequency in Hz
 */
#if defined(MODE_FCC)
#define ftx		902200000
#define frx		905200000
#elif defined(MODE_ETSI)
#define ftx		868130000
#define frx		869525000
#else
#error incorrect Frequency Standard defined in apps/device_config.h
#endif


/*!
 * \brief The RF_DEBUG flag will display the TX and RX frequency values on UART.
 * 		  The RF_DEGUG_ADV will display the frequency register values on UART.
 */
//#define RF_DEBUG
//#define RF_DEBUG_ADV


#endif //HWCONFIG

/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/
