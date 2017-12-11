/*
 * uart.c
 *
 *  Created on: 27.11.2017
 *      Author:
 */

#include <SI_EFM8BB1_Register_Enums.h>
#include <string.h>
#include "Globals.h"
#include "uart_0.h"
#include "uart.h"
#include "RF_Handling.h"
#include "RF_Protocols.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
SI_SEGMENT_VARIABLE(UART_RX_Buffer[UART_BUFFER_SIZE], uint8_t, SI_SEG_XDATA);
SI_SEGMENT_VARIABLE(UART_TX_Buffer[UART_BUFFER_SIZE], uint8_t, SI_SEG_XDATA);
SI_SEGMENT_VARIABLE(UART_RX_Buffer_Position, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(UART_TX_Buffer_Position, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(UART_Buffer_Read_Position, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(UART_Buffer_Write_Position, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(lastRxError, static volatile uint8_t,  SI_SEG_XDATA)=0;

//-----------------------------------------------------------------------------
// UART ISR Callbacks
//-----------------------------------------------------------------------------
void UART0_receiveCompleteCb()
{
}

void UART0_transmitCompleteCb()
{
}

//=========================================================
// Interrupt API
//=========================================================
SI_INTERRUPT(UART0_ISR, UART0_IRQn)
{
	//Buffer and clear flags immediately so we don't miss an interrupt while processing
	uint8_t flags = SCON0 & (UART0_RX_IF | UART0_TX_IF);
	SCON0 &= ~flags;

	// receiving byte
	if ((flags &  SCON0_RI__SET))
	{
	    if ( UART_RX_Buffer_Position == UART_BUFFER_SIZE )
	    {
	        /* error: receive buffer overflow */
	        lastRxError = UART_BUFFER_OVERFLOW >> 8;
	    }
	    else
	    {
	        /* store received data in buffer */
	    	UART_RX_Buffer[UART_RX_Buffer_Position] = SBUF0;
	        UART_RX_Buffer_Position++;
	    }
	}

	// transmit byte
	if ((flags & SCON0_TI__SET))
	{
		if ( UART_TX_Buffer_Position == UART_BUFFER_SIZE )
		{
	        /* error: receive buffer overflow */
	        lastRxError = UART_BUFFER_OVERFLOW >> 8;
		}
		else
		{
			if (UART_Buffer_Write_Position < UART_TX_Buffer_Position)
			{
				SBUF0 = UART_TX_Buffer[UART_Buffer_Write_Position];
				UART_Buffer_Write_Position++;
			}

			if (UART_Buffer_Write_Position == UART_TX_Buffer_Position)
			{
				UART_TX_Buffer_Position = 0;
				UART_Buffer_Write_Position = 0;
			}
		}
	}
}

void uart_buffer_reset(void)
{
	UART_RX_Buffer_Position = 0;
	UART_Buffer_Read_Position = 0;
	UART_TX_Buffer_Position = 0;
	UART_Buffer_Write_Position = 0;
}

uint8_t uart_getlen(void)
{
	return UART_RX_Buffer_Position - UART_Buffer_Read_Position;
}

bool uart_transfer_finished(void)
{
	return UART_Buffer_Write_Position == UART_TX_Buffer_Position;
}

/*************************************************************************
Function: uart_getc()
Purpose:  return byte from ringbuffer
Returns:  lower byte:  received byte from ringbuffer
          higher byte: last receive error
**************************************************************************/
unsigned int uart_getc(void)
{
	unsigned int rxdata;

    if ( UART_Buffer_Read_Position == UART_RX_Buffer_Position ) {
        return UART_NO_DATA;   /* no data available */
    }

    /* get data from receive buffer */
    rxdata = UART_RX_Buffer[UART_Buffer_Read_Position];
    UART_Buffer_Read_Position++;

    /* all got read of the received data, reset to 0 */
    if (UART_Buffer_Read_Position == UART_RX_Buffer_Position)
    {
    	UART_Buffer_Read_Position = 0;
    	UART_RX_Buffer_Position = 0;
    }

    rxdata |= (lastRxError << 8);
    lastRxError = 0;
    return rxdata;
}

/*************************************************************************
Function: uart_putc()
Purpose:  write byte to ringbuffer for transmitting via UART
Input:    byte to be transmitted
Returns:  none
**************************************************************************/
void uart_putc(uint8_t txdata)
{
	if (UART_TX_Buffer_Position == UART_BUFFER_SIZE)
		lastRxError = UART_BUFFER_OVERFLOW >> 8;

	UART_TX_Buffer[UART_TX_Buffer_Position] = txdata;
	UART_TX_Buffer_Position++;
}

void uart_put_command(uint8_t command)
{
	uart_putc(RF_CODE_START);
	uart_putc(command);
	uart_putc(RF_CODE_STOP);
	UART0_initTxPolling();
}

void uart_put_uint16_t(uint8_t command, uint16_t value)
{
	uart_putc(RF_CODE_START);
	uart_putc(command);
	uart_putc((value >> 8) & 0xFF);
	uart_putc(value & 0xFF);
	uart_putc(RF_CODE_STOP);
	UART0_initTxPolling();
}

void uart_put_RF_Data(uint8_t Command, uint8_t used_protocol)
{
	uint8_t i = 0;
	uint8_t b = 0;

	uart_putc(RF_CODE_START);
	uart_putc(Command);

	while(i < PROTOCOL_DATA[used_protocol].BIT_COUNT)
	{
		i += 8;
		b++;
	}
	uart_putc(b+1);

	// set identifier for this protocol
	uart_putc(PROTOCOL_DATA[used_protocol].IDENTIFIER);

	// copy data to UART buffer
	i = 0;
	while(i < b)
	{
		uart_putc(RF_DATA[i]);
		i++;
	}
	uart_putc(RF_CODE_STOP);

	UART0_initTxPolling();
}

void uart_put_RF_CODE_Data(uint8_t Command)
{
	uint8_t i = 0;
	uint8_t b = 0;

	uart_putc(RF_CODE_START);
	uart_putc(Command);

	// sync low time
	uart_putc((SYNC_LOW >> 8) & 0xFF);
	uart_putc(SYNC_LOW & 0xFF);
	// bit 0 high time
	uart_putc((BIT_LOW >> 8) & 0xFF);
	uart_putc(BIT_LOW & 0xFF);
	// bit 1 high time
	uart_putc((BIT_HIGH >> 8) & 0xFF);
	uart_putc(BIT_HIGH & 0xFF);

	// copy data to UART buffer
	i = 0;
	while(i < (24 / 8))
	{
		uart_putc(RF_DATA[i]);
		i++;
	}
	uart_putc(RF_CODE_STOP);

	UART0_initTxPolling();
}
