/*
 * RF_Handling.h
 *
 *  Created on: 28.11.2017
 *      Author:
 */

#ifndef INC_RF_HANDLING_H_
#define INC_RF_HANDLING_H_

extern bool buffer_out(uint16_t *bucket);
extern void HandleRFBucket(uint16_t duration, bool high_low);
extern uint8_t PCA0_DoSniffing(uint8_t active_command);
extern void PCA0_StopSniffing(void);
extern void SendRFBuckets(uint16_t buckets[],
		SI_VARIABLE_SEGMENT_POINTER(rfdata, uint8_t, SI_SEG_XDATA), uint8_t data_len);
extern void SendBuckets(
		uint16_t pulses[], uint8_t pulses_size,
		uint8_t start[], uint8_t start_size,
		uint8_t bit0[], uint8_t bit0_size,
		uint8_t bit1[], uint8_t bit1_size,
		uint8_t bit_count,
		bool inverse,
		SI_VARIABLE_SEGMENT_POINTER(rfdata, uint8_t, SI_SEG_XDATA));
extern void SendBucketsByIndex(uint8_t index, SI_VARIABLE_SEGMENT_POINTER(rfdata, uint8_t, SI_SEG_XDATA));
extern void Bucket_Received(uint16_t duration, bool high_low);

// 112 byte == 896 bits, so a RF signal with maximum of 896 bits is possible
// for bucket transmission, this depends on the number of buckets.
// E.g. 4 buckets have a total overhead of 11, allowing 101 bits (high/low pairs)
#define RF_DATA_BUFFERSIZE		112

typedef enum
{
	RF_IDLE,
	RF_IN_SYNC,
	RF_BUCKET_REPEAT,
	RF_BUCKET_IN_SYNC,
	RF_DECODE_BUCKET,
	RF_FINISHED
} rf_state_t;

typedef enum
{
	STANDARD,
	ADVANCED
} rf_sniffing_mode_t;

#define RF_DATA_RECEIVED_MASK	0x80

extern SI_SEGMENT_VARIABLE(RF_DATA[RF_DATA_BUFFERSIZE], uint8_t, SI_SEG_XDATA);
// RF_DATA_STATUS
// Bit 7:	1 Data received, 0 nothing received
// Bit 6-0:	Protocol identifier
extern SI_SEGMENT_VARIABLE(RF_DATA_STATUS, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(rf_state, rf_state_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(sniffing_mode, rf_sniffing_mode_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(last_sniffing_command, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(SYNC_LOW, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(BIT_HIGH, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(BIT_LOW, uint16_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(actual_bit_of_byte, uint8_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(actual_byte, uint8_t, SI_SEG_XDATA);

extern SI_SEGMENT_VARIABLE(bucket_sync, uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(buckets[7], uint16_t, SI_SEG_XDATA);
extern SI_SEGMENT_VARIABLE(bucket_count, uint8_t, SI_SEG_XDATA);

#endif /* INC_RF_HANDLING_H_ */
