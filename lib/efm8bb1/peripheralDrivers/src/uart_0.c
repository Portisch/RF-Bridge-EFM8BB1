/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "uart_0.h"

uint8_t UART0_getIntFlags()
{
  return SCON0 & (UART0_TX_IF | UART0_RX_IF);
}

void UART0_clearIntFlag(uint8_t flag)
{
  SCON0 &= ~(flag);
}

void UART0_initTxPolling()
{
  SCON0_TI = 1;
}

void UART0_write(uint8_t value)
{
	SBUF0 = value;
}

uint8_t UART0_read(void)
{
  return SBUF0;
}
void UART0_writeWithExtraBit(uint16_t value)
{
	SCON0_TB8 = value >> 8;
	SBUF0 = value;
}

uint16_t UART0_readWithExtraBit(void)
{
  return (SBUF0 | ((SCON0 & SCON0_RB8__BMASK) << 6) );
}

void UART0_init(UART0_RxEnable_t rxen, UART0_Width_t width, UART0_Multiproc_t mce)
{
    SCON0 &= ~(SCON0_SMODE__BMASK
               | SCON0_MCE__BMASK
               | SCON0_REN__BMASK);
    SCON0 = mce | rxen | width;
}

void UART0_reset()
{
	SCON0 = SCON0_SMODE__8_BIT
			| SCON0_MCE__MULTI_DISABLED
			| SCON0_REN__RECEIVE_DISABLED
			| SCON0_TB8__CLEARED_TO_0
			| SCON0_RB8__CLEARED_TO_0
			| SCON0_TI__NOT_SET
			| SCON0_RI__NOT_SET;
}

//=========================================================
// Interrupt API
//=========================================================
#if EFM8PDL_UART0_USE_BUFFER == 1

/**
 * Internal variable fort tracking buffer transfers. transferLenth[UART0_TX_TRANSFER] = bytes remaining in transfer.
 */
SI_SEGMENT_VARIABLE(txRemaining, static uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(rxRemaining, static uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE_SEGMENT_POINTER(txBuffer,    static uint8_t, EFM8PDL_UART0_TX_BUFTYPE, SI_SEG_XDATA);
SI_SEGMENT_VARIABLE_SEGMENT_POINTER(rxBuffer,    static uint8_t, EFM8PDL_UART0_RX_BUFTYPE, SI_SEG_XDATA);


SI_INTERRUPT(UART0_ISR, UART0_IRQn)
{
  //Buffer and clear flags immediately so we don't miss an interrupt while processing
  uint8_t flags = SCON0 & (UART0_RX_IF | UART0_TX_IF);
  SCON0 &= ~flags;

  if (rxRemaining && (flags &  SCON0_RI__SET))
  {
    *rxBuffer = SBUF0;
    ++rxBuffer;
    --rxRemaining;
    if (!rxRemaining)
    {
      UART0_receiveCompleteCb();
    }
  }

  if ((flags & SCON0_TI__SET))
  {
    if (txRemaining){
      SBUF0 = *txBuffer;
      ++txBuffer;
      --txRemaining;
    }
    else
    {
      UART0_transmitCompleteCb();
    }
  }
}

void UART0_writeBuffer(SI_VARIABLE_SEGMENT_POINTER(buffer,
                                                uint8_t,
                                                EFM8PDL_UART0_TX_BUFTYPE),
                       uint8_t length)
{
  //Init internal data
  txBuffer = buffer+1;
  txRemaining = length-1;

  //Send initial byte
  SBUF0 = *buffer;
}

void UART0_readBuffer(SI_VARIABLE_SEGMENT_POINTER(buffer,
		                                       uint8_t,
		                                       EFM8PDL_UART0_RX_BUFTYPE),
		              uint8_t length)
{
  //Init internal data
  rxBuffer = buffer;
  rxRemaining = length;
}

void UART0_abortWrite()
{
  txRemaining = 0;
}

void UART0_abortRead()
{
  rxRemaining = 0;
}

uint8_t UART0_txBytesRemaining()
{
  return txRemaining;
}

uint8_t UART0_rxBytesRemaining()
{
  return rxRemaining;
}

#endif //EFM8PDL_UART0_USE_BUFFER

#if EFM8PDL_UART0_USE_STDIO == 1

#if defined __C51__

char putchar(char c){
  while(!SCON0_TI);
  SBUF0 = c;
  SCON0_TI = 0;
  return c;
}

char _getkey(){
  while(!SCON0_RI);
  SCON0_RI = 0;
  return SBUF0;
}

#elif defined __ICC8051__

int putchar(int c){
  while(!SCON0_TI);
  SBUF0 = c;
  SCON0_TI = 0;
  return c;
}

int getchar(void){
  while(!SCON0_RI);
  SCON0_RI = 0;
  return SBUF0;
}

#endif

void UART0_initStdio()
{
  SCON0 |= SCON0_REN__RECEIVE_ENABLED | SCON0_TI__SET;
}
#endif //EFM8PDL_UART0_USE_STDIO


