/**************************************************************************//**
    @file       hal_board.h

    @brief      Board HAL header file for MSP on SmartRF TrxEB

******************************************************************************/
#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * INCLUDES
 */
#include <msp430.h>
#include "hal_types.h"
//#include "hal_msp430.h"

/******************************************************************************
 * CONSTANTS
 */
// Board Support Package ID
#define BOARD_TRXEB_MSP5438A

// Buttons
#define HAL_BOARD_IO_BUTTON_LEFT_PORT       2
#define HAL_BOARD_IO_BUTTON_LEFT_PIN        1
#define HAL_BOARD_IO_BUTTON_RIGHT_PORT      2
#define HAL_BOARD_IO_BUTTON_RIGHT_PIN       2
#define HAL_BOARD_IO_BUTTON_S_PORT          2
#define HAL_BOARD_IO_BUTTON_S_PIN           3
#define HAL_BOARD_IO_BUTTON_UP_PORT         2
#define HAL_BOARD_IO_BUTTON_UP_PIN          4
#define HAL_BOARD_IO_BUTTON_DOWN_PORT       2
#define HAL_BOARD_IO_BUTTON_DOWN_PIN        5

// LEDs
#define HAL_BOARD_IO_LED_1_PORT             4
#define HAL_BOARD_IO_LED_1_PIN              0
#define HAL_BOARD_IO_LED_2_PORT             4
#define HAL_BOARD_IO_LED_2_PIN              1
#define HAL_BOARD_IO_LED_3_PORT             4
#define HAL_BOARD_IO_LED_3_PIN              2
#define HAL_BOARD_IO_LED_4_PORT             4
#define HAL_BOARD_IO_LED_4_PIN              3

// Ambient light sensor
#define HAL_BOARD_IO_ALS_PWR_PORT           6
#define HAL_BOARD_IO_ALS_PWR_PIN            1
#define HAL_BOARD_IO_ALS_OUT_PORT           6
#define HAL_BOARD_IO_ALS_OUT_PIN            2

// Accelerometer
#define HAL_BOARD_IO_ACC_PWR_PORT           6
#define HAL_BOARD_IO_ACC_PWR_PIN            0
#define HAL_BOARD_IO_ACC_INT_PORT           2
#define HAL_BOARD_IO_ACC_INT_PIN            0
#define HAL_BOARD_IO_ACC_MISO_PORT          9
#define HAL_BOARD_IO_ACC_MISO_PIN           5
#define HAL_BOARD_IO_ACC_MOSI_PORT          9
#define HAL_BOARD_IO_ACC_MOSI_PIN           4
#define HAL_BOARD_IO_ACC_CLK_PORT           9
#define HAL_BOARD_IO_ACC_CLK_PIN            0
#define HAL_BOARD_IO_ACC_CS_PORT            8
#define HAL_BOARD_IO_ACC_CS_PIN             7

// SPI flash
#define HAL_BOARD_IO_FLASH_PWR_PORT         7
#define HAL_BOARD_IO_FLASH_PWR_PIN          6
#define HAL_BOARD_IO_FLASH_RST_PORT         7
#define HAL_BOARD_IO_FLASH_RST_PIN          2
#define HAL_BOARD_IO_FLASH_MISO_PORT        9
#define HAL_BOARD_IO_FLASH_MISO_PIN         2
#define HAL_BOARD_IO_FLASH_MOSI_PORT        9
#define HAL_BOARD_IO_FLASH_MOSI_PIN         1
#define HAL_BOARD_IO_FLASH_CLK_PORT         9
#define HAL_BOARD_IO_FLASH_CLK_PIN          3
#define HAL_BOARD_IO_FLASH_CS_PORT          8
#define HAL_BOARD_IO_FLASH_CS_PIN           6

// LCD
#define HAL_BOARD_IO_LCD_PWR_PORT           7
#define HAL_BOARD_IO_LCD_PWR_PIN            7
#define HAL_BOARD_IO_LCD_MODE_PORT          9
#define HAL_BOARD_IO_LCD_MODE_PIN           7
#define HAL_BOARD_IO_LCD_RST_PORT           7
#define HAL_BOARD_IO_LCD_RST_PIN            3
#define HAL_BOARD_IO_LCD_MISO_PORT          9
#define HAL_BOARD_IO_LCD_MISO_PIN           2
#define HAL_BOARD_IO_LCD_MOSI_PORT          9
#define HAL_BOARD_IO_LCD_MOSI_PIN           1
#define HAL_BOARD_IO_LCD_CLK_PORT           9
#define HAL_BOARD_IO_LCD_CLK_PIN            3
#define HAL_BOARD_IO_LCD_CS_PORT            9
#define HAL_BOARD_IO_LCD_CS_PIN             6

//  RF_UART (USCI_A0)
#define HAL_BOARD_RF_UART_TXD_PORT          3
#define HAL_BOARD_RF_UART_TXD_PIN           4
#define HAL_BOARD_RF_UART_RXD_PORT          3
#define HAL_BOARD_RF_UART_RXD_PIN           5

// USB_UART (USCI_A1)
#define HAL_BOARD_USB_UART_TXD_PORT         5
#define HAL_BOARD_USB_UART_TXD_PIN          6
#define HAL_BOARD_USB_UART_RXD_PORT         5
#define HAL_BOARD_USB_UART_RXD_PIN          7
#define HAL_BOARD_USB_UART_CTS_PORT         2
#define HAL_BOARD_USB_UART_CTS_PIN          7
#define HAL_BOARD_USB_UART_RTS_PORT         4
#define HAL_BOARD_USB_UART_RTS_PIN          4

// RF_SPI0 (UCSI_B0)
#define HAL_BOARD_RF_SPI0_MISO_PORT         3
#define HAL_BOARD_RF_SPI0_MISO_PIN          2
#define HAL_BOARD_RF_SPI0_MOSI_PORT         3
#define HAL_BOARD_RF_SPI0_MOSI_PIN          1
#define HAL_BOARD_RF_SPI0_CLK_PORT          3
#define HAL_BOARD_RF_SPI0_CLK_PIN           3
#define HAL_BOARD_RF_SPI0_CS_PORT           3
#define HAL_BOARD_RF_SPI0_CS_PIN            0

// RF_SPI1 (USCI_B1)
#define HAL_BOARD_RF_SPI1_MISO_PORT         5
#define HAL_BOARD_RF_SPI1_MISO_PIN          4
#define HAL_BOARD_RF_SPI1_MOSI_PORT         3
#define HAL_BOARD_RF_SPI1_MOSI_PIN          7
#define HAL_BOARD_RF_SPI1_CLK_PORT          1
#define HAL_BOARD_RF_SPI1_CLK_PIN           2
#define HAL_BOARD_RF_SPI1_CS_PORT           1
#define HAL_BOARD_RF_SPI1_CS_PIN            4


/******************************************************************************
 * PROTOTYPES
 */
void halBoardInit(void);


#ifdef  __cplusplus
}
#endif

/******************************************************************************
  Copyright 2010 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
*******************************************************************************/

#endif
