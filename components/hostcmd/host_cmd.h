/*
 * host_cmd.h
 *
 *  Created on: Jan 18, 2015
 *      Author: Texas Instruments, Inc
 */

#ifndef HOST_CMD_H_
#define HOST_CMD_H_

/* return states */
/*#define HOST_CMD_NOT_FOUND      0x00
#define HOST_CMD_FOUND          0x01
#define HOST_CMD_SUCCESS		0x02
#define HOST_CMD_ERROR          0xFF*/

/**************************************************************************//**
 * @addtogroup UART
 * @{
 ******************************************************************************/

/*****************************
 * \enum host_cmd_status_t
 * \brief  error constants
 *****************************/
typedef enum {
	HOST_CMD_NOT_FOUND 	= 0x00,		/*!< no command found. */
	HOST_CMD_FOUND  	= 0x01,		/*!< host command found. */
	HOST_CMD_SUCCESS 	= 0x02,		/*!< no errors. */
	HOST_CMD_ERROR 		= 0xFF		/*!< errors in parsing the host command*/
}host_cmd_status_t;


/******************************************************************************
 * FUNCTION PROTOTYPES
 */
host_cmd_status_t parseHostCmd(unsigned char *host_cmd, unsigned char length);
unsigned char dataToString(unsigned char *data, char *str, unsigned char length);


/**************************************************************************//**
 * Close the Doxygen group.
 * @}
 ******************************************************************************/

#endif /* HOST_CMD_H_ */
