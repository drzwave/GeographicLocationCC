/*
 * UART_DRZ.h
 *
 *  Created on: Jul 10, 2023
 *      Author: eric
 * in events.h, add the following line:
 * EVENT_EUSART1_CHARACTER_RECEIVED
 * Then in the event_handler in the command class/app add a switch for this event which will indicate data is in the RX FIFO.
 */

#ifndef UART_DRZ_H_
#define UART_DRZ_H_

#include <em_eusart.h>
#include <em_gpio.h>

void UART_Init( EUSART_TypeDef *uart,        // Pointer to one of the EUSARTs
    uint32_t baudrate,                  // 0=enable Autobaud, 1-1,000,000 bits/sec
    EUSART_Databits_TypeDef databits,
    EUSART_Stopbits_TypeDef stopbits,
    EUSART_Parity_TypeDef parity,
    GPIO_Port_TypeDef TxPort,
    unsigned int TxPin,
    GPIO_Port_TypeDef RxPort,
    unsigned int RxPin);

int EUSART1_RxDepth(void);
uint8_t EUSART1_GetChar(void);

// Rx FIFO depth in bytes - make it long enough to hold the longest expected message
#define RX_FIFO_DEPTH 32

#endif /* UART_DRZ_H_ */
