//*****************************************************************************
//! @file       timer.c
//! @brief      Timer Management.
//!				Two timers are used in this project.
//!       \li \e Timer1 is used to generate bit rate ( fcc : 600bps / etsi : 100bps )
//!       \li \e Timer0 is used to ensure SigFox Downlink protocol timings are under control
//!              \li \c 20 s Waiting time after the 1st INITIATE_DOWNLINK Uplink Frame
//!              \li \c 25 s Reception windows to get the Downling frame
//!
//****************************************************************************/
 

/**************************************************************************//**
* @addtogroup Timer
* @{
******************************************************************************/


/******************************************************************************
 * INCLUDES
 */
#include "hal_spi_rf_trxeb.h"
#include "msp430.h"
#include "sigfox_demo.h"
#include "sigfox_types.h"
#include "device_config.h"


/******************************************************************************
 * DEFINES
 */


/******************************************************************************
 * LOCAL VARIABLES
 */
extern e_SystemState SysState;
u8 TIMER0_timeout = FALSE;

static u8 interrupt_count = 0;
static u8 nb_interrupt_to_wait_for = 0 ;


/******************************************************************************
 * FUNCTIONS
 */
/**************************************************************************//**
*   @brief 		Initialize the Timer uses to produce the signal bitrate
*          \li 	FCC bit rate  : 600bps => 1 bit each 1.66 ms
*          \li 	ETSI bit rate : 100bps => 1 bit each 10 ms
******************************************************************************/
void
TIMER_bitrate_init(void)
{
	//
	// Watchdog Timer Control Register
	//
	WDTCTL = WDTPW + WDTHOLD;	/*!< Watchdog Password, Timer hold */

	//
	// Timer Register Configuration
	//
	TA1CCTL0 = CCIE;	// CCR0 interrupt enabled
#if defined(MODE_FCC)
	TA1CCR0  = 39990;		// count depends on the MCU clock set to SMLK to create the bitrate (1.66 ms )
	TA1CTL   = TASSEL_2 + TACLR; // Choose the SMCLK countmode, clear the timer counter
#elif defined(MODE_ETSI)
	TA1CCR0  = 29982;		// count depends on the MCU clock set to SMLK to create the bitrate (10 ms )
	TA1CTL   = TASSEL_2 + ID_3 + TACLR; // Choose the SMCLK countmode, Devide the clock by 8, clear the timer counter
#endif
}


/**************************************************************************//**
*   @brief 	This timer will set the base interrupt for the downlink timing
*   @note   To have a common interrupt base for the 20s and the 25s timing,
*          	we choose the ACLK clock of 32kHz
******************************************************************************/
void
TIMER_downlink_timing_init( uint16 time_in_seconds )
{
	//
	// Timer0_A5 Capture/Compare 0
	//
	TA0CCR0 =  32700;	// Count 1 second

	//
	// Timer Control Register
	//
	TA0CTL |= TASSEL_1 + TACLR;	// ACLK (32kHz) Count mode and Clear the timer counter

	//
	// Timer Capture/Compare Control 0 Register
	//
	TA0CCTL0 |= CCIE;	// TAxCCR0 interrupt enabled

    //
	// Activate the timer
    //
	TA0CTL |= MC_1;		// Timer A mode control: 1 - Up to TAxCCR0

    //
	// Initialize the number of interrupt to wait for
    //
	nb_interrupt_to_wait_for = time_in_seconds;

	//
	// Reset the counter
	//
	interrupt_count = 0;
}


/**************************************************************************//**
 *   @brief This timer will stop the base interrupt for the downlink timing
 ******************************************************************************/
void
TIMER_downlink_timing_stop ( void )
{
    //
	// Reset the timeout value
    //
	TIMER0_timeout = FALSE;

	//
	// Deactivate Timer0
	//
    TA0CTL &= ~MC_1;
	TA0CTL |= TACLR;
}


/***************************************************************************//**
*   @brief  Start the bitrate Timer
*******************************************************************************/
void
TIMER_bitrate_start(void)
{
	//
	// Activate Timer 1
	//
	TA1CTL |= MC_1;

	TA1R = 45; // Set the timer counter to 45
}


/***************************************************************************//**
*   @brief Stop the bitrate timer
*******************************************************************************/
void
TIMER_bitrate_stop(void)
{
	//
	// Deactivate Timer1
	//
	TA1CTL &= ~MC_1;
    TA1CTL |= TACLR;
}


/***************************************************************************//**
*   @brief  Timer1 interrupt : a new bit has to be sent to the Radio
*           Change the system state status to processing
*******************************************************************************/
#pragma vector=TIMER1_A0_VECTOR
__interrupt void
TIMER1_A0_ISR(void)
{
	//
	//update processing flag
	//
	SysState = TxProcessing;
}


/***************************************************************************//**
*   @brief  Timer0 interrupt : this interrupt will be used either for
*           the 20s and 25 seconds Downlink timings
*******************************************************************************/
#pragma vector=TIMER0_A0_VECTOR
__interrupt void
TIMER0_A0_ISR(void)
{
	//
	// Increment the base downlink interrupt counter
	//
	interrupt_count++;

        if (interrupt_count == nb_interrupt_to_wait_for)
        {
		TIMER0_timeout = TRUE;	
	
		//
		// Reset the counter
		//
		interrupt_count = 0;
		nb_interrupt_to_wait_for = 0;		
        }
}



/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
