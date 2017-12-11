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
#define RF_CODE_START		0xAA
#define RF_CODE_STOP		0x55

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
	RF_CODE_ACK = 0xA0,
	RF_CODE_LEARN = 0xA1,
	RF_CODE_LEARN_KO = 0xA2,
	RF_CODE_LEARN_OK = 0xA3,
	RF_CODE_RFIN = 0xA4,
	RF_CODE_RFOUT = 0xA5,
	RF_CODE_SNIFFING_ON = 0xA6,
	RF_CODE_SNIFFING_OFF = 0xA7,
	RF_CODE_TRANSMIT_DATA_NEW = 0xA8
} uart_command_t;


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
extern SI_SEGMENT_VARIABLE(UART_RX_Buffer[UART_BUFFER_SIZE], uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(UART_TX_Buffer[UART_BUFFER_SIZE], uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(uart_state, uart_state_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(uart_command, uart_command_t, SI_SEG_XDATA);

extern void uart_buffer_reset(void);
extern uint8_t uart_getlen(void);
extern bool uart_transfer_finished(void);
extern unsigned int uart_getc(void);
extern void uart_putc(uint8_t txdata);
extern void uart_put_command(uint8_t command);
extern void uart_put_uint16_t(uint8_t command, uint16_t value);
extern void uart_put_RF_Data(uint8_t Command, uint8_t used_protocol);
extern void uart_put_RF_CODE_Data(uint8_t Command);


#endif /* INC_UART_H_ */
