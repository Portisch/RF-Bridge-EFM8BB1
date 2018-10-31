/*
 * RF_Handling.h
 *
 *  Created on: 28.11.2017
 *      Author:
 */

#ifndef INC_RF_HANDLING_H_
#define INC_RF_HANDLING_H_

extern uint8_t Compute_CRC8_Simple_OneByte(uint8_t byteVal);
extern uint8_t RFInSync(uint8_t identifier, uint8_t inverse, uint16_t period_pos, uint16_t period_neg);
extern uint8_t PCA0_GetProtocolIndex(uint8_t identifier);
extern void SetTimer0Overflow(uint8_t T0_Overflow);
extern uint8_t PCA0_DoSniffing(uint8_t active_command);
extern void PCA0_StopSniffing(void);
extern void SendRFBuckets(uint16_t *buckets, uint8_t *rfdata, uint8_t n, uint8_t repeats);
extern void SendTimingProtocol(uint16_t sync_high, uint16_t sync_low,
		uint16_t bit_0_high, uint16_t bit_0_low, uint16_t bit_1_high, uint16_t bit_1_low,
		uint8_t sync_bits, uint8_t bitcount, uint8_t position);
extern void Bucket_Received(uint16_t duration);

// 112 byte == 896 bits, so a RF signal with maximum of 896 bits is possible
// for bucket transmission, this depends on the number of buckets.
// E.g. 4 buckets have a total overhead of 11, allowing 101 bits (high/low pairs)
#define RF_DATA_BUFFERSIZE		112

typedef enum
{
	RF_IDLE,
	RF_IN_SYNC,
	RF_DECODE_BUCKET,
	RF_TRANSMITTING,
	RF_FINISHED
} rf_state_t;

typedef enum
{
	// do sniffing by timing
	MODE_TIMING,
	// do sniffing by bucket
	// https://github.com/pimatic/RFControl
	MODE_BUCKET
} rf_sniffing_mode_t;

#define RF_DATA_RECEIVED_MASK	0x80

extern SI_SEGMENT_VARIABLE(RF_DATA[RF_DATA_BUFFERSIZE], uint8_t, SI_SEG_XDATA);
// RF_DATA_STATUS
// Bit 7:	1 Data received, 0 nothing received
// Bit 6-0:	Protocol identifier
extern SI_SEGMENT_VARIABLE(RF_DATA_STATUS, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(rf_state, rf_state_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(desired_rf_protocol, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(rf_sniffing_mode, rf_sniffing_mode_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(protocol_index_in_sync, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(last_sniffing_command, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(SYNC_LOW, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(BIT_HIGH, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(BIT_LOW, uint16_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(actual_bit_of_byte, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_bit, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_sync_bit, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_byte, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(bucket_sync, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(buckets[15], uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(bucket_count, uint8_t, SI_SEG_XDATA);

#endif /* INC_RF_HANDLING_H_ */
