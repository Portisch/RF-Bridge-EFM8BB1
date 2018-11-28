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
SI_SEGMENT_VARIABLE(UART_RX_Buffer[UART_RX_BUFFER_SIZE], uint8_t, SI_SEG_XDATA);
SI_SEGMENT_VARIABLE(UART_TX_Buffer[UART_TX_BUFFER_SIZE], uint8_t, SI_SEG_DATA);
SI_SEGMENT_VARIABLE(UART_RX_Buffer_Position, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(UART_TX_Buffer_Position, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(UART_Buffer_Read_Position, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(UART_Buffer_Write_Position, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(UART_Buffer_Write_Len, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(lastRxError, static volatile uint8_t,  SI_SEG_XDATA)=0;
SI_SEGMENT_VARIABLE(TX_Finished, bool, SI_SEG_DATA) = false;
SI_SEGMENT_VARIABLE(uart_state, uart_state_t, SI_SEG_XDATA) = IDLE;
SI_SEGMENT_VARIABLE(uart_command, uart_command_t, SI_SEG_XDATA) = NONE;

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
        /* store received data in buffer */
    	UART_RX_Buffer[UART_RX_Buffer_Position] = UART0_read();
        UART_RX_Buffer_Position++;

        // set to beginning of buffer if end is reached
        if ( UART_RX_Buffer_Position == UART_RX_BUFFER_SIZE )
	    	UART_RX_Buffer_Position = 0;
	}

	// transmit byte
	if ((flags & SCON0_TI__SET))
	{
		if (UART_Buffer_Write_Len > 0)
		{
			UART0_write(UART_TX_Buffer[UART_Buffer_Write_Position]);
			UART_Buffer_Write_Position++;
			UART_Buffer_Write_Len--;
		}
		else
			TX_Finished = true;

		if (UART_Buffer_Write_Position == UART_TX_BUFFER_SIZE)
			UART_Buffer_Write_Position = 0;
	}
}

void uart_wait_until_TX_finished(void)
{
	while(!TX_Finished);
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

    if (UART_Buffer_Read_Position == UART_RX_BUFFER_SIZE)
    	UART_Buffer_Read_Position = 0;

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
	TX_Finished = false;

	if (UART_TX_Buffer_Position == UART_TX_BUFFER_SIZE)
		UART_TX_Buffer_Position = 0;

	UART_TX_Buffer[UART_TX_Buffer_Position] = txdata;
	UART_TX_Buffer_Position++;
	UART_Buffer_Write_Len++;
}

void uart_put_command(uint8_t command)
{
	uart_putc(RF_CODE_START);
	uart_putc(command);
	uart_putc(RF_CODE_STOP);
	UART0_initTxPolling();
}

void uart_put_RF_Data_Advanced(uint8_t Command, uint8_t protocol_index)
{
	uint8_t i = 0;
	uint8_t b = 0;
	uint8_t bits = 0;

	uart_putc(RF_CODE_START);
	uart_putc(Command);

	bits = PROTOCOL_DATA[protocol_index].bit_count;

	while(i < bits)
	{
		i += 8;
		b++;
	}

	uart_putc(b+1);

	// send index off this protocol
	uart_putc(protocol_index);

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

void uart_put_RF_Data_Standard(uint8_t Command)
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

void uart_put_RF_buckets(uint8_t Command)
{
	uint8_t i = 0;

	uart_putc(RF_CODE_START);
	uart_putc(Command);
	// put bucket count + sync bucket
	uart_putc(bucket_count + 1);

	// start and wait for transmit
	UART0_initTxPolling();
	uart_wait_until_TX_finished();

	// send up to 7 buckets
	while (i < bucket_count)
	{
		uart_putc((buckets[i] >> 8) & 0xFF);
		uart_putc(buckets[i] & 0xFF);
		i++;
	}

	// send sync bucket
	uart_putc((bucket_sync >> 8) & 0xFF);
	uart_putc(bucket_sync & 0xFF);

	// start and wait for transmit
	UART0_initTxPolling();
	uart_wait_until_TX_finished();

	i = 0;
	while(i < (actual_byte + 1))
	{
		uart_putc(RF_DATA[i]);
		i++;

		// be safe to have no buffer overflow
		if ((i % UART_TX_BUFFER_SIZE) == 0)
		{
			// start and wait for transmit
			UART0_initTxPolling();
			uart_wait_until_TX_finished();
		}
	}

	uart_putc(RF_CODE_STOP);

	UART0_initTxPolling();
}
