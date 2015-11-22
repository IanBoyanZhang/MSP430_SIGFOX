//*****************************************************************************
//! @file       sigfox.h
//! @brief      Definitions and function prototypes of SIGFOX library API
//! @author 	SigFox Test and Validation team
//! @version 	0.1
//!
//****************************************************************************/


#ifndef SIGFOX1_H_
#define SIGFOX1_H_


/**************************************************************************//**
 * @addtogroup LibInterface
 * @{
 ******************************************************************************/


#include "sigfox_types.h"
#include <stdbool.h>

#define MAX_CUSTOMER_DATA_LENGTH 12

/*****************************
 * \enum SFX_error_t
 * \brief  error constants
 *****************************/
typedef enum{
    SFX_ERR_NONE 		= 0, 	/*!< no error. */
    SFX_ERR_INIT		= 1,	/*!< manufacturer API init error.*/
    SFX_ERR_ID_KEY 		= 2,	/*!< ID or key error.*/
    SFX_ERR_STATEMACHINE= 3,	/*!< State machine error.*/
    SFX_ERR_SIZE		= 4,	/*!< Size is more than biggest frame available.*/
    SFX_ERR_SEND 		= 5,	/*!< manufacturer send error.*/
    SFX_ERR_GETVOLT 	= 6,	/*!< manufacturer get_voltage_temp error.*/
    SFX_ERR_CLOSE       = 7,    /*!< Close problem occurs.*/
	SFX_ERR_API         = 8,    /*!< error code from external function*/
	SFX_ERR_PN          = 9,    /*!< error code PN9 Get or set*/
	SFX_ERR_FREQCY      =10,    /*!< error code FREQUENCY*/
	SFX_ERR_FRAME		=11,	/*!< error in frame build*/
	SFX_ERR_DELAY		=12,	/*!< error from delay function*/
	SFX_ERR_CBACK		=13,
	SFX_ERR_TIME		=14,
	SFX_ERR_FREQ 		=15,
	SFX_ERR_RECEIVE		=16
} SFX_error_t;


typedef enum {
	E_FRAME_ERROR = 0,
	E_FRAME_RECEIVED  = 1,
	E_FRAME_TIMEOUT = 2
}SFX_ext_status;

/********************************
 * \enum te_DataType
 * \brief Data type for Nvram access
 *******************************/
typedef enum {
	E_PN = 0,
	E_SEQ_CPT = 1,
	E_CPT_FSEND = 2
}te_DataType;

/********************************
 * \enum te_RxChipMode
 * \brief functionnal mode for Rf chip
 *******************************/
typedef enum {
	E_TX_MODE = 0,
	E_RX_MODE = 1
}te_RxChipMode;

/********************************
 * \enum te_DelayType
 * \brief Delay type
 * \      E_RX : delay inter frames in TX/RX (send frame with ack=1) : (=500ms)
 * \      E_TX : delay inter frames in TX only (0-2000ms)
 * \      E_OOB : delay between frame reception and send followed out of band message (1400ms-4000ms)
 *******************************/
typedef enum {
	E_RX_DELAY = 0,   
	E_TX_DELAY = 1, 
	E_OOB_ACK_DELAY
}te_DelayType;

/********************************
 * \enum SFX_state_t
 * \brief State Machine constants
 *******************************/
typedef enum {
    SFX_STATE_IDLE = 0, // uninitialized
    SFX_STATE_READY = 1,
    SFX_STATE_TX = 2, // TX on going
	SFX_STATE_RX = 3 // TX on going
} SFX_state_t;

/********************************
 * \enum SFX_std_t
 * \brief Standard constants
 *******************************/
typedef enum {
    SFX_STD_ETSI = 0, 
    SFX_STD_FCC = 1
} SFX_std_t;



/********************************************************
 * external API dependencies to link with this library
 ********************************************************/
extern u8 *id_ptr; 						// pointer to 4 bytes read only unic id
extern u8 *key_ptr; 					// pointer to unic read only 16bytes key

extern u32 *TxCF;
extern SFX_std_t  *Standard; // pointer to standard : ETSI or FCC
extern u32 *RxCF;
extern u8 *TxRepeat;

/********************************************************
 * Function Prototypes for the library
 ********************************************************/
extern u8* sfx_malloc(u16 size); 											// memory allocation of size bytes : can point to static buffer
extern SFX_error_t sfx_free(u8 *p); 										// memory free
extern SFX_error_t sfx_get_voltage_temperature(u16 *voltage_idle,
											   u16 *voltage_tx ,
											   u16 *temperature); 			// get temperature and input voltage level
extern SFX_error_t sfx_init(te_RxChipMode e_ChipMode); 											// use for init radio, RF configuration, memory space allocation
extern SFX_error_t sfx_close(void); 										// use to close library, free memory space allocation
extern SFX_error_t sfx_send(u8 *message, u8 size);							// use to send a user message of size bytes
extern SFX_error_t sfx_delay(te_DelayType e_TypeDelay);
extern SFX_error_t sfx_change_frequency(u32 frequency); 					// change synthesizer carrier frequency
extern SFX_error_t sfx_AES_128_cbc_encrypt(u8 *Encrypted_data,
										   u8 *Data_To_Encrypt,
										   u8 data_len,
										   const u8 *key,
										   const u8  *iv);					// function to compute AES (hardcoded or soft coded
extern SFX_error_t sfx_get_nv_mem(te_DataType e_DataTypeR, u16 *valueR); 	//get a value from an index in non volatile memory
extern SFX_error_t sfx_set_nv_mem(te_DataType e_DataTypeW, u16 valueW); 	// set a value in non volatile memory


extern SFX_ext_status sfx_waitframe(u8 *frame);
extern s8 sfx_getrssivalue(void);
extern SFX_error_t sfx_StartRxTimeout(u16 Temps);
SFX_error_t sfx_StartWaitingTimeout(u16 Temps);
SFX_error_t sfx_WaitForTimeoutRx(void);
void sfx_StopRxTimeout(void);
extern void sfx_PrintRxTestResult(u8* ReplyForm, u8 RssiValue);

/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/



/**************************************************************************//**
 * @addtogroup Library_API
 * @{
 ******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
//                           Sigfox Library API calls                         //
////////////////////////////////////////////////////////////////////////////////

/*! \fn SFX_error_t SfxInit(void)

 	\brief This function initializes external parameters to the sigfox library.

	\return error code ::SFX_error_t
 */
SFX_error_t SfxInit(void);



/*! \fn SFX_error_t SfxClose(void)

 	\brief This function de-initializes sigfox library

	\return error code ::SFX_error_t
 */
SFX_error_t SfxClose(void);



/*!	\fn SFX_error_t SfxSendFrame(u8 *customer_data, u8 customer_data_length, u8 *ReturnPayload, bool ack)

	\brief 	This function sends \a customer_data of length \a customer_data_length.
			The received downlink data is stored in \a ReturnPayload location and
			acknowlegement is transmitted after that if \a ack is true.


	\param customer_data			is the pointer to the 12-byte(max)
   									array to be sent in uplink.
	\param customer_data_length		is the length of the uplink data 12-byte max.
	\param ReturnPayload			is the pointer to the location where
   									the downlink message will be stored.
   									NULL if sending uplink only frame.
	\param ack						is used to ask for acknoledgement
   									for this frame.
   									NULL if sending uplink only frame.

	\return error code ::SFX_error_t
 */
SFX_error_t SfxSendFrame(u8 *customer_data, u8 customer_data_length, u8 *ReturnPayload, bool ack);



/*! \fn SFX_error_t SfxSendBit(bool state,u8 *ReturnPayload, bool ack)

 	\brief This function sends 1-bit status value \a state. The received
 		   downlink data is stored in \a ReturnPayload location and
		   acknowlegement is transmitted if \a ack is true.

	\param state					is boolean bit status 1/0.
	\param ReturnPayload			is thepointer to the location where
									the downlink message will be stored.
									NULL if sending uplink only.
	\param ack						is the boolean parameter to select
									whether to send ack after the downlink
									mesasge is received.
									NULL if sending uplink only.

	\return error code ::SFX_error_t
 */
SFX_error_t SfxSendBit(bool state,u8 *ReturnPayload, bool ack);



/*! \fn SFX_error_t SfxSendOutOfBand(void)

 	\brief This function sends and Out Of Band (OOB) keep alive message

	\return error code ::SFX_error_t
 */
SFX_error_t SfxSendOutOfBand(void);



/*!	\fn SFX_state_t SfxGetTxState(void)

 	\brief This function returns the state of the radio chip

 	\return Chip state ::SFX_state_t
 */
SFX_state_t SfxGetTxState(void);



/*!	\fn u8* SfxGetLibVersion(void)

	\brief This function returns a pointer to the string containing version of
		   the sigfox library

	\return pointer to the version of the library.
 */
u8* SfxGetLibVersion(void);



/*!	\fn void SfxTxTestMode(s16 frame_count, s16 channel)

	\brief This function transmits total of \a frame_count test frames on the
	 	   specified \a channel.

	\param frame_count		[value: 1 to 32700] means number of test frames to send
   							[value: -1] means infinite test frames
	\param channel			is the uplink channel to transmit test frames on
							[value: 0 to 480] for specific channels
							[value: -1] for hopping frequency after each frame
 */
void SfxTxTestMode(s16 frame_count, s16 channel);



/*!	\fn void SfxRxTestMode(s16 channel, u16 Sequence_nb, u16 Temps)

	\brief This function configures the module in the RX mode for the specified
		   time \a Temps. The module will receive all the messages on specified
		   downlink \a channel with the specified \a Sequence_nb.

	\param channel			is the receive channel for downlink test.
  							[value: 0 to 480] downlink channel number.
	\param Sequence_nb		is the sequence number of downlink message to receive.
	\param Temps			is number of seconds before the RX timeout.
 */
void SfxRxTestMode(s16 channel, u16 Sequence_nb, u16 Temps);


/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/

#endif /* SIGFOX1_H_ */
