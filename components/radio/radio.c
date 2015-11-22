//*****************************************************************************
//! @file       radio.c
//! @brief      Manage the low level modulation and configuration of the TI chipset
//
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
 * @addtogroup Radio
 * @{
 ******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#include "stdbool.h"
#include "device_config.h"
#include "modulation_table.h"
#include "trx_rf_int.h"
#include "cc112x_spi.h"
#include "radio.h"
#ifdef RF_DEBUG
#include "uart_drv.h"
#include "host_cmd.h"
#endif
#include "hal_spi_rf_trxeb.h"
#include "../../sigfox_library_api/sigfox.h"

/******************************************************************************
* MACROS
*/
// Fast write on SPI for modulation
#define trx8BitWrite(addrByte, Data)\
{ \
\
  /* Pull CS_N low and wait for SO to go low before communication starts */ \
  TRXEM_SPI_BEGIN();\
  /* send register address byte */ \
  TRXEM_SPI_TX(RADIO_BURST_ACCESS|RADIO_WRITE_ACCESS|addrByte);\
  TRXEM_SPI_WAIT_DONE();\
  /* Storing chip status */\
  TRXEM_SPI_TX(Data);\
  TRXEM_SPI_WAIT_DONE();\
  TRXEM_SPI_END();\
}

// Fast write to extended registers on SPI for modulation
#define trx16BitWrite(extAddr, regAddr, Data)\
{ \
\
  /* Pull CS_N low and wait for SO to go low before communication starts */\
  TRXEM_SPI_BEGIN();\
  /* send extended address byte with access type bits set */ \
  TRXEM_SPI_TX(RADIO_BURST_ACCESS|RADIO_WRITE_ACCESS|extAddr);\
  TRXEM_SPI_WAIT_DONE();\
  TRXEM_SPI_TX(regAddr);\
  TRXEM_SPI_WAIT_DONE();\
  /* communicate len number of bytes*/\
  TRXEM_SPI_TX(Data);\
  TRXEM_SPI_WAIT_DONE();\
  TRXEM_SPI_END();\
}

/******************************************************************************
* DEFINES
*/

/* Modulation constants (FREQOFF) */
#if defined(RF_XTAL_FREQ_40MHZ)

#define FOFF1	             0x02  // 0x0258 = 600 Value computed to be around
#define FOFF0	             0x58  // the middle of the register value

#define FREQ_STEP_LOW_FOFF1  0x00  // 0x00C8 = 200
#define FREQ_STEP_LOW_FOFF0  0xC8  // delta_f = 600 - 200 = 400 rf step

#define FREQ_STEP_HIGH_FOFF1 0x03  // 0x03E8 = 1000
#define FREQ_STEP_HIGH_FOFF0 0xE8  // delta_f = 1000 - 600 = 400 rf step

#define FOFF1_ETSI			 0x00  // 0x007F = 127
#define FOFF0_ETSI			 0x7F  // the middle of the register value

#elif defined(RF_XTAL_FREQ_32MHZ)

#define FOFF1	             0x02  // 0x02EE = 750 Value computed to be around
#define FOFF0	             0x58  // the middle of the register value

#define FREQ_STEP_LOW_FOFF1  0x00  // 0x00FA = 250
#define FREQ_STEP_LOW_FOFF0  0x64  // delta_f = 750 - 250 = 500 rf step

#define FREQ_STEP_HIGH_FOFF1 0x04  // 0x04E2 = 1250
#define FREQ_STEP_HIGH_FOFF0 0x4C  // delta_f = 1250 - 750 = 500 rf step

#define FOFF1_ETSI			 0x00  // 0x007F = 127
#define FOFF0_ETSI			 0x7F  // the middle of the register value
#endif

/* Delay constants. (Depend on MCU SMCLK value and SPI speed) */
#define MODULATION_DELAY_CYCLES_600bps		57		// Calibrate the shape of 600 bps spectrum
#define MODULATION_DELAY_CYCLES_100bps		1000		// Calibrate the shape of 100 bps spectrum
#define PHASE_ACCUMULATION_DELAY_CYCLES		320		// Calibrate time to accumulate the 180 degree phase change. Depends on FOFFx values declared above.

/* Hardware specific constants to calculate frequency register settings */
#if defined(RF_XTAL_FREQ_40MHZ)
#define STEP_MULTIPLICATOR  0.0065536  	// Pre-calculated frequency coefficient : float(((2^16)*4)/40000000)
#define FINE_STEP_REGISTER	0.026214	// Pre-calculated frequency coefficient : float(((2^18)*4)/40000000)
#define STPMoverFSR			0.25		// Pre-calculated frequency coefficient : 0.0065536/0.026214
#elif defined(RF_XTAL_FREQ_32MHZ)
#define STEP_MULTIPLICATOR	0.008192	// Pre-calculated frequency coefficient : float(((2^16)*4)/32000000)
#define FINE_STEP_REGISTER	0.032768	// Pre-calculated frequency coefficient : float(((2^18)*4)/32000000)
#define STPMoverFSR			0.25		// Pre-calculated frequency coefficient : 0.008192/0.032768
#endif


/******************************************************************************
 * LOCAL VARIABLES
 */
static bool b_Diff;


/******************************************************************************
 * FUNCTION PROTOTYPE
 */
static void RADIO_rx_packet_interrupt_handler(void);


/******************************************************************************
* FUNCTIONS
*/
/**************************************************************************//**
 * @brief       ISR for packet handling in RX. Sets packet semaphore
 *              and clears isr flag.
 *******************************************************************************/
static void
RADIO_rx_packet_interrupt_handler(void)
{
	// Set packet semaphore
	packetSemaphore = ISR_ACTION_REQUIRED;

	// Clear isr flag
	trxClearIntFlag();
}


/**************************************************************************//**
 *  @brief		This function initializes the RF Chip
 *
 *  @param 		ul_CentralFrequency	is the new frequency (in Hz) to program the chip
 *
 *	@param 		e_ChipMode 			is the mode ( RX or TX ) see ::te_RxChipMode
 ******************************************************************************/
void
RADIO_init_chip(u32 ul_CentralFrequency, te_RxChipMode e_ChipMode)
{
	uint8 writeByte;
	uint16 i;

	// Reset radio
	trxSpiCmdStrobe(CC112X_SRES);

	// Set the radio in IDLE mode
	trxSpiCmdStrobe(CC112X_SIDLE);

	// Program the proper registers depending on the RF mode ( RX / TX )
	if ( e_ChipMode == E_TX_MODE )
	{
		// Write registers of the radio chip for TX mode
		for( i = 0; i < (sizeof(HighPerfModeTx)/sizeof(registerSetting_t)); i++)
		{
			writeByte = HighPerfModeTx[i].data;
			cc112xSpiWriteReg(HighPerfModeTx[i].addr, &writeByte, 1);
		}
	}
	else if ( e_ChipMode == E_RX_MODE )
	{
		// Write registers of the radio chip for RX mode
		for( i = 0; i < (sizeof(HighPerfModeRx)/sizeof(registerSetting_t)); i++)
		{
			writeByte = HighPerfModeRx[i].data;
			cc112xSpiWriteReg(HighPerfModeRx[i].addr, &writeByte, 1);
		}
		// Configure ISR to signal PKT_SYNC_RXTX - asserted when sync word has been received
		trxIsrConnect(&RADIO_rx_packet_interrupt_handler);

		// Enable interrupt from GPIO_3
		trxEnableInt();
	}

	// Send the frequency value to the chip
	RADIO_change_frequency(ul_CentralFrequency);


#ifdef CC1190_PA_LNA
	// Enable PA/LNA according to the e_ChipMode
	if (e_ChipMode == E_TX_MODE )
	{
		// configure txon
		RF_PA_EN_PxOUT |= RF_PA_EN_PIN;
		RF_LNA_EN_PxOUT &= ~RF_LNA_EN_PIN;
	}
	else if (e_ChipMode == E_RX_MODE )
	{
		// configure rxon
		RF_PA_EN_PxOUT &= ~RF_PA_EN_PIN;
		RF_LNA_EN_PxOUT |= RF_LNA_EN_PIN;
	}
#endif
}


/**************************************************************************//**
 *  @brief 		This function puts the radio in Idle mode
 ******************************************************************************/
void
RADIO_close_chip(void)
{
	trxSpiCmdStrobe(CC112X_SIDLE);
}


/**************************************************************************//**
 *  @brief 		This function allows to change the central frequency used by the chip
 *
 *  @param 		ul_Freq is the new frequency to use
 *
 *  @note		RF frequency Programming : From the CC1125 documentation,
 * 				frequencies have to be programmed following the relations
 *
 *	@note 	 	Freq_rf = Freq_vco / LO_Divider [Hz]
 *  @note		Freq_vco = FREQ * Fxosc/2^16 + FREOFF * Fxosc/2^18  [Hz]
 *
 *  @note		With LO_Divider = 4 and Fxosc = 40 MHz or 32 MHz, we can simplify the equation to:
 *
 *  @note		\li FREQ = 0.0065536 * Freq_rf - 0.25 FREQOFF for 40 MHz XTAL
 *  @note		\li FREQ = 0.008192 * Freq_rf - 0.25 FREQOFF for 32 MHz XTAL
 ******************************************************************************/
void
RADIO_change_frequency(unsigned long ul_Freq)
{
	uint8 tuc_Frequence[3];
	unsigned long CalibFrequency = 0;
	unsigned long freq_rf;
	unsigned long freq_reg_value;
	uint16 offset_reg_value;
	uint8 writeByte[2];

	// Sending the FREQOFF value
	writeByte[0] = FOFF0;
	writeByte[1] = FOFF1;
	cc112xSpiWriteReg(CC112X_FREQOFF0, &writeByte[0],1);
	cc112xSpiWriteReg(CC112X_FREQOFF1, &writeByte[1],1);

	// adding a calibration offset if it's necessary
	freq_rf = ul_Freq + CalibFrequency;

	// adding Frequency register offset value to compute new frequency value
	offset_reg_value = (u16)(FOFF1<<8) + (u16)(FOFF0);
	freq_reg_value   = (u32)(STEP_MULTIPLICATOR * freq_rf ) - (u32)( STPMoverFSR * offset_reg_value );

	// save the value into table
	tuc_Frequence[0]= (u8) (freq_reg_value & 0x000000FF);
	tuc_Frequence[1]= (u8) ((freq_reg_value & 0x0000FF00)>> 8u);
	tuc_Frequence[2]= (u8) ((freq_reg_value & 0x00FF0000)>> 16u);

	// send frequency registers value to the chip
	cc112xSpiWriteReg(CC112X_FREQ2, &tuc_Frequence[2],1);
	cc112xSpiWriteReg(CC112X_FREQ1, &tuc_Frequence[1],1);
	cc112xSpiWriteReg(CC112X_FREQ0, &tuc_Frequence[0],1);

#ifdef RF_DEBUG_ADV
	uint8 frOff[2];
	uint8 fr[3];
	char freqoff[4];
	char freq[6];
	cc112xSpiReadReg(CC112X_FREQOFF1, &frOff[0], 1);
	cc112xSpiReadReg(CC112X_FREQOFF0, &frOff[1], 1);
	cc112xSpiReadReg(CC112X_FREQ2, &fr[0], 1);
	cc112xSpiReadReg(CC112X_FREQ1, &fr[1], 1);
	cc112xSpiReadReg(CC112X_FREQ0, &fr[2], 1);
	dataToString(frOff, freqoff,2);
	uartPutStr(freqoff,4);
	uartPutChar(',');
	dataToString(fr, freq, 3);
	uartPutStr(freq,6);
	uartPutChar(0x0D);
#endif
}


/**************************************************************************//**
 *  @brief 		This function produces the modulation (PA + Freq).
 *         		It is called only when a '0' bit is encountered in the frame.
 *
 *  @note		BPSK modulation: we need to accumulate phase ( 180 degrees )
 *  			during a defined time with the following relation :
 *
 * 	@note 	  	2*pi*delta_frequency = ( delta_phase / delta_time )
 *
 * 	@note		\li delta_phase for a BPSK modulation has to be pi ( 180 degrees )
 * 	@note		\li delta_frequency will be set to 15240 Hz ( 400 * 38.1 Hz ( RF Step for Fxosc = 40MHz ) )
 *  	 										( 500 * 30.5 Hz ( RF Step for Fxosc = 32MHz) )
 * 	@note		\li delta_time will then be compute to 32.8 us which is 656 ticks - for MSP430 clock at 20 Mhz
 *
 *  @note       To ensure there is no frequency drift, we alternate
 *  			the delta frequency: One time we increase the frequency,
 *  			the other we decrease it.
 *  @note       During the modulation time, the PA is OFF
 ******************************************************************************/
void
RADIO_modulate(void)
{
#if (defined(MODE_FCC) || defined(MODE_ETSI))
	s16 count;
	uint8 writeByte_FOFF0;
	uint8 writeByte_FOFF1;

	if (b_Diff == false)
	{
		b_Diff = true;
		// Frequency step down
		writeByte_FOFF1 = FREQ_STEP_LOW_FOFF1;
		writeByte_FOFF0 = FREQ_STEP_LOW_FOFF0;
	}
	else
	{
		b_Diff = false;
		// Frequency step high
		writeByte_FOFF1 = FREQ_STEP_HIGH_FOFF1;
		writeByte_FOFF0 = FREQ_STEP_HIGH_FOFF0;
	}
	// deacrease PA
	for (count = (NB_PTS_PA-1); count >= 0; count--)
	{
		// Write the PA ramp levels to PA_CFG2 register
		trx8BitWrite(CC112X_PA_CFG2, Table_Pa_600bps[NB_PTS_PA-count-1]);

		// Wait after changing PA level to reduce spurrs
#if defined(MODE_FCC)
		__delay_cycles(MODULATION_DELAY_CYCLES_600bps);
#elif defined(MODE_ETSI)
		__delay_cycles(MODULATION_DELAY_CYCLES_100bps);
#endif
	}

	// Program the frequency offset
	cc112xSpiWriteReg(CC112X_FREQOFF1, &writeByte_FOFF1, 1);
	cc112xSpiWriteReg(CC112X_FREQOFF0, &writeByte_FOFF0, 1);

	/* IMPORTANT NOTE
	 *
	 * As explained above in the Modulation section,
	 * a  __delay_cycles(656);  instruction should take place here
	 * to ensure a phase accumation of pi.
	 *
	 * But after accounting for SPI write delays, __delay_cycles(215);
	 * was used for best results. This value of delay cycles was
	 * obtained experimentally. This value can be used to calibrate
	 * the modulation for best possible quality ( SNR ) of the BPSK
	 * signal. The quality of BPSK modulation might change for different
	 * compiler optimization settings and different MCU clock frequencies.
	 * Thus, requiring new calibration for this delay value.
	 */
	__delay_cycles(PHASE_ACCUMULATION_DELAY_CYCLES);

	writeByte_FOFF1 = FOFF1;
	cc112xSpiWriteReg(CC112X_FREQOFF1, &writeByte_FOFF1, 1);
	writeByte_FOFF0 = FOFF0;
	cc112xSpiWriteReg(CC112X_FREQOFF0, &writeByte_FOFF0, 1);


	// increase PA
	for (count = NB_PTS_PA-1; count >= (0); count--)
	{
		// Write the PA ramp levels to PA_CFG2 register
		trx8BitWrite(CC112X_PA_CFG2, Table_Pa_600bps[count]);

		// Wait after changing PA level to reduce spurrs
#if defined(MODE_FCC)
		__delay_cycles(MODULATION_DELAY_CYCLES_600bps);
#elif defined(MODE_ETSI)
		__delay_cycles(MODULATION_DELAY_CYCLES_100bps);
#endif
	}

#elif defined(MODE_ETSI_OPT)
	s16 count;
	uint8 u8_FreqValue;

	if(b_Diff == true)
	{
		b_Diff = false;

		for (count = 0; count < (NB_POINTS); count++)
		{
			// Modulate using PA and FREQOFF
			u8_FreqValue = FOFF0_ETSI + CC1120_etsi_profile[count][1];

            trx8BitWrite(CC112X_PA_CFG2, CC1120_etsi_profile[count][0]);
            trx16BitWrite((uint8)(CC112X_FREQOFF0 >> 8), (uint8)(CC112X_FREQOFF0 & 0x00FF), u8_FreqValue);
            __delay_cycles(58);
		}
	}
	else
	{
		b_Diff = true;

		for (count = 0; count < (NB_POINTS); count++)
		{
			// Modulate using PA and FREQOFF
			u8_FreqValue = FOFF0_ETSI - CC1120_etsi_profile[count][1];

			trx8BitWrite(CC112X_PA_CFG2, CC1120_etsi_profile[count][0]);
            trx16BitWrite((uint8)(CC112X_FREQOFF0 >> 8), (uint8)(CC112X_FREQOFF0 & 0x00FF), u8_FreqValue);
            __delay_cycles(58);
		}
	}

#endif
}


/**************************************************************************//**
 *  @brief this function starts the oscillator, and generates the ramp-up
 ******************************************************************************/
void
RADIO_start_rf_carrier(void)
{
	int16 countStart;
	uint8 writeByte;

	writeByte = 0x00;
	cc112xSpiWriteReg(CC112X_PA_CFG2, &writeByte, 1);
	trxSpiCmdStrobe(CC112X_STX);
	writeByte = 0x00;
	cc112xSpiWriteReg(CC112X_PA_CFG2, &writeByte, 1);

#if (defined(MODE_FCC) || defined(MODE_ETSI))
	// Ramp up the PA
	for (countStart = NB_PTS_PA-1; countStart >= (0); countStart--)
	{
		cc112xSpiWriteReg(CC112X_PA_CFG2, (uint8*) &Table_Pa_600bps[countStart], 1);
		__delay_cycles(320);
	}
#elif defined(MODE_ETSI_OPT)
	// Ramp up the PA
	for (countStart = MID_NB_PTS; countStart < (NB_POINTS); countStart++)
	{
		// Modulate using PA and FREQOFF
		cc112xSpiWriteReg(CC112X_PA_CFG2, (uint8*) &CC1120_etsi_profile[countStart][0], 1);
		__delay_cycles(320);
	}
#endif

	writeByte = 63;
	cc112xSpiWriteReg(CC112X_PA_CFG2, &writeByte, 1);
}


/**************************************************************************//**
 *  @brief This function stops the radio and produces the ramp down
 ******************************************************************************/
void
RADIO_stop_rf_carrier(void)
{
	uint16 count_stop;
	uint8 writeByte;

#if (defined(MODE_FCC) || defined(MODE_ETSI	))
	// Ramp down the PA
	for (count_stop = 0; count_stop < (NB_PTS_PA); count_stop++)
	{
		cc112xSpiWriteReg(CC112X_PA_CFG2, (uint8*) &Table_Pa_600bps[count_stop], 1);
		__delay_cycles(320);
	}
#elif defined(MODE_ETSI_OPT)
	// Ramp down the PA
	for (count_stop = 0; count_stop < (MID_NB_PTS); count_stop++)
	{
		// Modulate using PA and FREQOFF
		cc112xSpiWriteReg(CC112X_PA_CFG2, (uint8*) &CC1120_etsi_profile[count_stop][0], 1);
		__delay_cycles(320);
	}
#endif

	writeByte = 0;
	cc112xSpiWriteReg(CC112X_PA_CFG2, &writeByte, 1);
	trxSpiCmdStrobe(CC112X_SIDLE);
	writeByte = 0;
	cc112xSpiWriteReg(CC112X_PA_CFG2, &writeByte, 1);
}


/**************************************************************************//**
 *  @brief 		This function configures the cc112x for continuous wave (CW)
 *  			transmission at the given frequency and starts the TX.
 *
 *  @param 		ul_Freq 		is the frequency to use
 ******************************************************************************/
void
RADIO_start_unmodulated_cw(unsigned long ul_Freq)
{
	// Initialize the radio in TX mode
	RADIO_init_chip(ul_Freq, E_TX_MODE);

	// Start TX carrier wave
	RADIO_start_rf_carrier();
}


/**************************************************************************//**
 *  @brief 		This function turns off the CW transmission at the given frequency
 *
 *  @param 		ul_Freq 		is the frequency to use
 ******************************************************************************/
void
RADIO_stop_unmodulated_cw(unsigned long ul_Freq)
{
	// Stop TX carrier wave
	RADIO_stop_rf_carrier();

	// Reinitialize the radio in TX mode
	RADIO_init_chip(ul_Freq, E_TX_MODE );
}

/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/
