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



#ifndef TIMER_H
#define TIMER_H


/******************************************************************************
 * FUNCTION PROTOTYPES
 */
void TIMER_bitrate_init(void);
void TIMER_bitrate_start(void);
void TIMER_bitrate_stop(void);
void TIMER_downlink_timing_init( uint16 time_in_seconds );
void TIMER_downlink_timing_stop ( void );
__interrupt void TIMER1_A0_ISR(void);
__interrupt void TIMER0_A0_ISR(void);

extern unsigned char TIMER0_timeout;

#endif // TIMER_H


