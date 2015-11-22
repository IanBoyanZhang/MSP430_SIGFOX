//*****************************************************************************
//! @file       sigfox_demo.h
//! @brief      Definitions for sigfox protocol Uplink/Downlink demo
//!
//****************************************************************************/

#ifndef SIGFOX_DEMO_H
#define SIGFOX_DEMO_H

#include "sigfox.h"

/********************************
 * \enum e_SystemState
 * \brief transmitter status
 *******************************/
typedef enum{
   IdleState =0,  /*!< Idle state */
   TxStart,       /*!< Start the carrier */
   TxWaiting,     /*!< Continue Transmit since all bits have not been sent */
   TxProcessing,  /*!< Send a bit */
   TxEnd          /*!< Transmission is finished */
}e_SystemState;


#define BYTES_SIZE_64 64
#define BYTES_SIZE_32 32
#define BYTES_SIZE_30 30
#define BYTES_SIZE_8   8

#define NUMBER_BLOCKS_64BYTES 1
#define NUMBER_BLOCKS_32BYTES 1
#define NUMBER_BLOCKS_30BYTES 2
#define NUMBER_BLOCKS_8BYTES  1

#define START_DYNAMIC_MEMORY 0

#define START_MEMORY_BLOCK_64BYTES START_DYNAMIC_MEMORY
#define END_MEMORY_BLOCK_64BYTES START_DYNAMIC_MEMORY +  NUMBER_BLOCKS_64BYTES * BYTES_SIZE_64

#define START_MEMORY_BLOCK_32BYTES END_MEMORY_BLOCK_64BYTES
#define END_MEMORY_BLOCK_32BYTES START_MEMORY_BLOCK_32BYTES + NUMBER_BLOCKS_32BYTES * BYTES_SIZE_32

#define START_MEMORY_BLOCK_30BYTES END_MEMORY_BLOCK_32BYTES
#define END_MEMORY_BLOCK_30BYTES START_MEMORY_BLOCK_30BYTES + NUMBER_BLOCKS_30BYTES * BYTES_SIZE_30

#define START_MEMORY_BLOCK_8BYTES END_MEMORY_BLOCK_30BYTES
#define END_MEMORY_BLOCK_8BYTES START_MEMORY_BLOCK_8BYTES + NUMBER_BLOCKS_8BYTES * BYTES_SIZE_8

#define SFX_DYNAMIC_MEMORY END_MEMORY_BLOCK_8BYTES

/********************************
 * \struct MemoryBlock
 * \brief memory block
 *******************************/
typedef struct
{
	u8 * memory_ptr;	/*!< pointer to the memory location */
	u8  allocated;		/*!< allocation status */
}MemoryBlock;

extern u8 DynamicMemoryTable[SFX_DYNAMIC_MEMORY];

extern MemoryBlock Table_64bytes[NUMBER_BLOCKS_64BYTES];
extern MemoryBlock Table_32bytes[NUMBER_BLOCKS_32BYTES];
extern MemoryBlock Table_30bytes[NUMBER_BLOCKS_30BYTES];
extern MemoryBlock Table_8bytes[NUMBER_BLOCKS_8BYTES];


#endif
