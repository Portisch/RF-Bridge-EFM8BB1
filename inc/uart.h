/*
 * uart.h
 *
 *  Created on: 28.11.2017
 *      Author:
 */

#ifndef INC_UART_H_
#define INC_UART_H_

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------
//#define UART_BUFFER_LENGTH   64 + 4
#define UART_SYNC_INIT		0xAA
#define UART_SYNC_END		0x55

#define UART_BUFFER_SIZE	64 + 4

/*
** high byte error return code of uart_getc()
*/
#define UART_FRAME_ERROR      0x1000              /* Framing Error by UART       */
#define UART_OVERRUN_ERROR    0x0800              /* Overrun condition by UART   */
#define UART_PARITY_ERROR     0x0400              /* Parity Error by UART        */
#define UART_BUFFER_OVERFLOW  0x0200              /* receive ringbuffer overflow */
#define UART_NO_DATA          0x0100              /* no receive data available   */
//-----------------------------------------------------------------------------
// Global Enums
//-----------------------------------------------------------------------------
typedef enum
{
	IDLE,
	SYNC_INIT,
	SYNC_FINISH,
	RECEIVE_LEN,
	RECEIVING,
	TRANSMIT,
	COMMAND
} uart_state_t;

typedef enum
{
	NONE = 0x00,
	COMMAND_AK = 0xA0,
	LEARNING = 0xA1,
	TIMEOUT_EXITS = 0xA2,
	LEARNING_SUCCESS = 0xA3,
	FORWARD_RF_KEY = 0xA4,
	TRANSMIT_KEY = 0xA5,
	SNIFFING_ON = 0xA6,
	SNIFFING_OFF = 0xA7,
	TRANSMIT_DATA = 0xA8
} uart_command_t;


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
extern SI_SEGMENT_VARIABLE(UART_RX_Buffer[UART_BUFFER_SIZE], uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(UART_TX_Buffer[UART_BUFFER_SIZE], uint8_t, SI_SEG_XDATA);
extern uart_state_t uart_state;

extern void uart_buffer_reset(void);
extern uint8_t uart_getlen(void);
extern bool uart_transfer_finished(void);
extern unsigned int uart_getc(void);
extern void uart_putc(uint8_t txdata);
extern void uart_put_command(uint8_t command);
extern void uart_put_uint16_t(uint8_t command, uint16_t value);
extern void uart_put_RF_Data(uint8_t Command, uint8_t used_protocol);


#endif /* INC_UART_H_ */
