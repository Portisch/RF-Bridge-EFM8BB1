/*
 * RF_Handling.h
 *
 *  Created on: 28.11.2017
 *      Author:
 */

#ifndef INC_RF_HANDLING_H_
#define INC_RF_HANDLING_H_

extern uint8_t RFInSync(uint8_t identifier, uint16_t period_pos, uint16_t period_neg);
extern uint8_t PCA0_GetProtocolIndex(uint8_t identifier);
extern void PCA0_InitTransmit(uint16_t sync_high, uint16_t sync_low, uint16_t BIT_HIGH_TIME, uint8_t BIT_HIGH_DUTY,
		uint16_t BIT_LOW_TIME, uint8_t BIT_LOW_DUTY, uint8_t bitcount);
extern void SetPCA0DutyCylce(void);
extern void SetTimer0Overflow(uint8_t T0_Overflow);
extern void PCA0_StartTransmit(void);
extern void PCA0_StopTransmit(void);
extern uint8_t PCA0_DoSniffing(uint8_t active_command);
extern void PCA0_StopSniffing(void);
extern void SendRFBuckets(uint16_t *buckets, uint8_t *rfdata, uint8_t n, uint8_t times);

// 64 byte == 512 bits, so a RF signal with maximum of 512 bits is possible
#define RF_DATA_BUFFERSIZE		64

#define SWB(val) ((((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00))

typedef enum
{
	RF_IDLE,
	RF_IN_SYNC,
	RF_TRANSMITTING,
	RF_FINISHED
} rf_state_t;

#define RF_DATA_RECEIVED_MASK	0x80

extern SI_SEGMENT_VARIABLE(RF_DATA[RF_DATA_BUFFERSIZE], uint8_t, SI_SEG_XDATA);

// RF_DATA_STATUS
// Bit 7:	1 Data received, 0 nothing received
// Bit 6-0:	Protocol identifier
extern SI_SEGMENT_VARIABLE(RF_DATA_STATUS, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(rf_state, rf_state_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(desired_rf_protocol, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(last_sniffing_command, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(DUTY_CYCLE_HIGH, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(DUTY_CYLCE_LOW, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(T0_HIGH, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(T0_LOW, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(SYNC_HIGH, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(SYNC_LOW, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(BIT_HIGH, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(BIT_LOW, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(BIT_COUNT, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(actual_bit_of_byte, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_bit, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_sync_bit, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_byte, uint8_t, SI_SEG_XDATA);

#endif /* INC_RF_HANDLING_H_ */
