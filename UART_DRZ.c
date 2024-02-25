/*
 * @file UART_DRZ.c
 * @brief UART Driver for Silicon Labs EFR32ZG23 and related series 2 chips
 *
 *  Created on: Jul 10, 2023
 *      Author: Eric Ryherd - DrZWave.blog
 *
 * Minimal drivers to initialize and setup the xG23 UARTs efficiently and provide simple functions for sending/receiving data.
 * Assumes using the EUSARTs and not the USART which has limited functionality and only a 2 byte buffer vs 16 in the EUSART.
 * Assumes high frequency mode (not operating in low-power modes with a low-frequency clock)
 * This example just implements a set of drivers for EUSART1. The code is tiny so make copies for the others as needed.
 * EUSART1 is the most flexible as the IOs can be assigned to any GPIO. EUSART0/2 have limited routing to GPIOs.
 * Normally the SDK uses EUSART0 for DEBUGPRINT so leave it for that purpose.
 * 
 * See https://drzwave.blog/2023/07/25/installing-uart-drivers-in-a-z-wave-project/ for details on the development of this driver.
 * 
 */

#include <em_cmu.h>
#include <ev_man.h>
#include "UART_DRZ.h"
#include "events.h"

// Rx Buffer and pointers for EUSART1. Make copies for other EUSARTs.
static uint8_t RxFIFO1[RX_FIFO_DEPTH];
static int RxFifoReadIndx1;
static int RxFifoWriteIndx1;
//static uint32_t EUSART1_Status;

/* UART_Init - basic initialization for the most common cases - works for all EUSARTs
 * Write to the appropriate UART registers to enable special modes after calling this function to enable fancy features.
 */
void UART_Init( EUSART_TypeDef *uart,   // EUSART1 - Pointer to one of the EUSARTs
    uint32_t baudrate,                  // 0=enable Autobaud, 1-1,000,000 bits/sec
    EUSART_Databits_TypeDef databits,   // eusartDataBits8=8 bits - must use the typedef!
    EUSART_Stopbits_TypeDef stopbits,   // eusartStopbits1=1 bit - follow the typdef for other settings
    EUSART_Parity_TypeDef parity,       // eusartNoParity
    GPIO_Port_TypeDef TxPort,           // gpioPortA thru D - Note that EUSART0 & 2 have GPIO port limitations
    unsigned int TxPin, 
    GPIO_Port_TypeDef RxPort, 
    unsigned int RxPin) 
{

    // Check for valid uart and assign uartnum
    int uartnum = EUSART0 == uart ? 0 :
        EUSART1 == uart ? 1 :
        EUSART2 == uart ? 2 : -1;
    EFM_ASSERT(uartnum>=0);

    CMU_Clock_TypeDef clock = uartnum == 0 ? cmuClock_EUSART0 :
        uartnum == 1 ? cmuClock_EUSART1 : cmuClock_EUSART2;

    if (uartnum>=0) {
        // Configure the clocks
        if (0==uartnum){
            CMU_ClockSelectSet(clock, cmuSelect_EM01GRPCCLK); // EUSART0 requires special clock configuration
        } // EUSART 1 and 2 use EM01GRPCCLK and changing it will cause VCOM to use the wrong baud rate.
        CMU_ClockEnable(clock, true);

        // Configure Frame Format
        uart->FRAMECFG = ((uart->FRAMECFG & ~(_EUSART_FRAMECFG_DATABITS_MASK | _EUSART_FRAMECFG_STOPBITS_MASK | _EUSART_FRAMECFG_PARITY_MASK))
                | (uint32_t) (databits) // note that EUSART_xxxxxx_TypeDef puts these settings in the proper bit locations
                | (uint32_t) (parity)
                | (uint32_t) (stopbits));

        EUSART_Enable(uart, eusartEnable);

        if (baudrate == 0) {
            uart->CFG0 |= EUSART_CFG0_AUTOBAUDEN;       // autobaud is enabled with baudrate=0 - note that 0x55 has to be received for autobaud to work
        } else {
            EUSART_BaudrateSet(uart, 0, baudrate);    // checks various limits to ensure no overflow and handles oversampling
        }

        CMU_ClockEnable(cmuClock_GPIO, true);    // Typically already enabled but just to be sure enable the GPIO clock anyway

        // Configure TX and RX GPIOs
        GPIO_PinModeSet(TxPort, TxPin, gpioModePushPull, 1);
        GPIO_PinModeSet(RxPort, RxPin, gpioModeInputPull, 1);
        GPIO->EUSARTROUTE[uartnum].ROUTEEN = GPIO_EUSART_ROUTEEN_TXPEN;
        GPIO->EUSARTROUTE[uartnum].TXROUTE = (TxPort << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
            | (TxPin << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
        GPIO->EUSARTROUTE[uartnum].RXROUTE = (RxPort << _GPIO_EUSART_RXROUTE_PORT_SHIFT)
            | (RxPin << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
    }

    RxFifoReadIndx1 = 0; // TODO - expand to other EUSARTs as needed
    RxFifoWriteIndx1 = 0;

    // Enable Rx Interrupts
    EUSART1->IEN_SET = EUSART_IEN_RXFL;
    NVIC_EnableIRQ(EUSART1_RX_IRQn);
}

/* EUSART1_RX_IRQHandler is the receive side interrupt handler for EUSART1.
 * startup_efr32zg23.c defines each of the IRQs as a WEAK function to Default_Handler which is then placed in the interrupt vector table.
 * By defining a function of the same name it overrides the WEAK function and places this one in the vector table.
 * Change this function name to match the EUSART you are using.
 *
 * This ISR pulls each byte out of the EUSART FIFO and places it into the software RxFIFO.
 */
void EUSART1_RX_IRQHandler(void){
  uint8_t dat;
  uint32_t flags = EUSART1->IF;
  EUSART1->IF_CLR = flags;                // clear all interrupt flags
  NVIC_ClearPendingIRQ(EUSART1_RX_IRQn);  // clear the NVIC Interrupt

  for (int i=0; (EUSART_STATUS_RXFL & EUSART1->STATUS) && (i<16); i++) { // Pull all bytes out of EUSART
      dat = EUSART1->RXDATA;                  // read 1 byte out of the hardware FIFO in the EUSART
      if (EUSART1_RxDepth()<RX_FIFO_DEPTH) { // is there room in the RxFifo?
          RxFIFO1[RxFifoWriteIndx1++] = dat;
          if (RxFifoWriteIndx1 >= RX_FIFO_DEPTH) {
              RxFifoWriteIndx1 = 0;
          }
      } else {  // No room in the RxFIFO, drop the data
          // TODO - report underflow
          break;
      }
      // TODO - add testing for error conditions here - like the FIFO is full... Set a bit and call an event
  }
  // TODO - check for error conditions
  ZCB_eventSchedulerEventAdd(EVENT_EUSART1_CHARACTER_RECEIVED); // Tell the application there is data in RxFIFO
}

// Return a byte from the RxFIFO - be sure there is one available by calling RxDepth first
uint8_t EUSART1_GetChar(void) {
  uint8_t rtn;
  rtn = RxFIFO1[RxFifoReadIndx1++]; // read the byte and increment the pointer
  if (RxFifoReadIndx1>=RX_FIFO_DEPTH) { // correct the pointer to the circular buffer when it wraps past end
    RxFifoReadIndx1 = 0;
  }
  return(rtn);
}

// Put 1 character into the EUSART1 hardware Tx FIFO - returns True if FIFO is not full and False if FIFO is full and the byte was not added - nonblocking
// Rely on the 16 byte hardware FIFO for data buffering.
bool EUSART1_PutChar(uint8_t dat) {
  bool rtn = false;
  if (EUSART1->STATUS & EUSART_STATUS_TXFL) {
      EUSART1->TXDATA = dat;
      rtn = true;
  }
  return(rtn);
}

// Number of valid data bytes in the RxFIFO - use this to avoid blocking GetChar
int EUSART1_RxDepth(void) {
  int rtn;
  rtn = RxFifoReadIndx1 - RxFifoWriteIndx1;
  if (rtn<0) {  // unroll the circular buffer 
      rtn +=RX_FIFO_DEPTH;
  }
  return(rtn);
}
