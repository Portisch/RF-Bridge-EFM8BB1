/*
 * RF_Handling.h
 *
 *  Created on: 28.11.2017
 *      Author:
 */

#ifndef INC_RF_HANDLING_H_
#define INC_RF_HANDLING_H_

extern void SendRF_SYNC(uint8_t used_protocol);
extern uint8_t PCA0_DoTransmit(uint8_t identifier);
extern void SetPCA0DutyCylce(void);
extern void SetTimer0Overflow(uint8_t T0_Overflow);
extern void PCA0_StopTransmit(void);
extern void PCA0_DoSniffing(void);
extern void PCA0_StopSniffing(void);

#define SYSCLK	24500000
// 64 byte == 512 bits, so a RF signal with maximum of 512 bits is possible
#define RF_DATA_BUFFERSIZE		64

typedef enum
{
	RF_IDLE,
	RF_IN_SYNC
} rf_state_t;

#define RF_DATA_RECEIVED_MASK	0x80

extern SI_SEGMENT_VARIABLE(RF_DATA[RF_DATA_BUFFERSIZE], uint8_t, SI_SEG_XDATA);
// RF_DATA_STATUS
// Bit 7:	1 Data received, 0 nothing received
// Bit 6-0:	Protocol identifier
extern SI_SEGMENT_VARIABLE(RF_DATA_STATUS, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(Timer_3_Timeout, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(sniffing_is_on, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(DUTY_CYCLE_HIGH, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(DUTY_CYLCE_LOW, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(T0_HIGH, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(T0_LOW, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(actual_bit_of_byte, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_bit, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_sync_bit, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_byte, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(protocol_index, uint8_t, SI_SEG_XDATA);

extern uint8_t testbyte;


#endif /* INC_RF_HANDLING_H_ */
