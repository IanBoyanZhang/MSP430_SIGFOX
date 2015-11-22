/*
 * circ_buf.h
 *
 *  Created on: Jan 19, 2015
 *      Author: a0869488
 */

#ifndef CIRC_BUF_H_
#define CIRC_BUF_H_

/*
 * \struct	circ_buffer_t
 * \brief	circular buffer parameters
 */
typedef struct {
	unsigned char head_ptr;
	unsigned char tail_ptr;
	unsigned char size_of_buffer;
	char *buffer;
} circ_buffer_t;


/***************************************************************************
 * FUNCTION PROTOTYPES
 */
unsigned char circBufRemainder(circ_buffer_t *buffer);
void circBufPutData(circ_buffer_t *buffer, unsigned char data);
unsigned char circBufGetData(circ_buffer_t *buffer);


#endif /* CIRC_BUF_H_ */
