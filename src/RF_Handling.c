/*
 * RF_Handling.c
 *
 *  Created on: 27.11.2017
 *      Author:
 */

#include <SI_EFM8BB1_Register_Enums.h>
#include <string.h>
#include <stdlib.h>
#include "Globals.h"
#include "RF_Handling.h"
#include "RF_Protocols.h"
#include "pca_0.h"
#include "uart.h"

SI_SEGMENT_VARIABLE(RF_DATA[RF_DATA_BUFFERSIZE], uint8_t, SI_SEG_XDATA);
// RF_DATA_STATUS
// Bit 7:	1 Data received, 0 nothing received
// Bit 6-0:	Protocol identifier
SI_SEGMENT_VARIABLE(RF_DATA_STATUS, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(rf_state, rf_state_t, SI_SEG_XDATA) = RF_IDLE;
SI_SEGMENT_VARIABLE(sniffing_mode, rf_sniffing_mode_t, SI_SEG_XDATA) = STANDARD;

SI_SEGMENT_VARIABLE(last_sniffing_command, uint8_t, SI_SEG_XDATA) = NONE;

// PT226x variables
SI_SEGMENT_VARIABLE(SYNC_LOW, uint16_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(BIT_HIGH, uint16_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(BIT_LOW, uint16_t, SI_SEG_XDATA) = 0x00;

SI_SEGMENT_VARIABLE(actual_byte_high_nibble, bool, SI_SEG_DATA) = false;
SI_SEGMENT_VARIABLE(actual_byte, uint8_t, SI_SEG_XDATA) = 0;

// status of each protocol
SI_SEGMENT_VARIABLE(status[PROTOCOLCOUNT], PROTOCOL_STATUS, SI_SEG_IDATA);

SI_SEGMENT_VARIABLE(old_crc, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(crc, uint8_t, SI_SEG_XDATA) = 0;

// up to 8 timing buckets for RF_CODE_SNIFFING_ON_BUCKET
SI_SEGMENT_VARIABLE(buckets[7], uint16_t, SI_SEG_XDATA);	// -1 because of the bucket_sync

#if INCLUDE_BUCKET_SNIFFING == 1
SI_SEGMENT_VARIABLE(bucket_sync, uint16_t, SI_SEG_XDATA);
SI_SEGMENT_VARIABLE(bucket_count, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(bucket_count_sync_1, uint8_t, SI_SEG_XDATA);
SI_SEGMENT_VARIABLE(bucket_count_sync_2, uint8_t, SI_SEG_XDATA);
#endif

#define GET_W_POSITION(x) (((x) >> 4) & 0x0F)
#define INC_W_POSITION(x) ((x) = ((((x) >> 4) + 1) << 4) | ((x) & 0x0F))
#define DEC_W_POSITION(x) ((x) = ((((x) >> 4) - 1) << 4) | ((x) & 0x0F))
#define CLR_W_POSITION(x) ((x) &= 0x0F)

#define GET_R_POSITION(x) ((x) & 0x0F)
#define INC_R_POSITION(x) ((x) = ((x) + 1) | ((x) & 0xF0))
#define DEC_R_POSITION(x) ((x) = ((x) - 1) | ((x) & 0xF0))
#define CLR_R_POSITION(x) ((x) &= 0xF0)

SI_SEGMENT_VARIABLE(buffer_buckets[4], uint16_t, SI_SEG_XDATA) = {0};
SI_SEGMENT_VARIABLE(buffer_buckets_positions, uint8_t, SI_SEG_XDATA) = 0;

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------
void PCA0_overflowCb() { }

void PCA0_intermediateOverflowCb() { }

uint8_t Compute_CRC8_Simple_OneByte(uint8_t byteVal)
{
    const uint8_t generator = 0x1D;
    uint8_t i;
    uint8_t crc = byteVal; /* init crc directly with input byte instead of 0, avoid useless 8 bitshifts until input byte is in crc register */

    for (i = 0; i < 8; i++)
    {
        if ((crc & 0x80) != 0)
        { /* most significant bit set, shift crc register and perform XOR operation, taking not-saved 9th set bit into account */
            crc = (uint8_t)((crc << 1) ^ generator);
        }
        else
        { /* most significant bit not set, go to next bit */
            crc <<= 1;
        }
    }

    return crc;
}

uint16_t compute_delta(uint16_t bucket)
{
	return ((bucket >> 2) + (bucket >> 4));
	//return (bucket >> 2); // 25% delta of bucket for advanced decoding
}

bool CheckRFBucket(uint16_t duration, uint16_t bucket, uint16_t delta)
{
	return (((bucket - delta) < duration) && (duration < (bucket + delta)));
}

bool CheckRFSyncBucket(uint16_t duration, uint16_t bucket)
{
	uint16_t delta = compute_delta(bucket);
	delta = delta > TOLERANCE_MAX ? TOLERANCE_MAX : delta;
	delta = delta < TOLERANCE_MIN ? TOLERANCE_MIN : delta;
	return CheckRFBucket(duration, bucket, delta);
}

bool DecodeBucket(uint8_t i, bool high_low, uint16_t duration,
		uint16_t *pulses,
		SI_VARIABLE_SEGMENT_POINTER(bit0, uint8_t, SI_SEG_CODE), uint8_t bit0_size,
		SI_VARIABLE_SEGMENT_POINTER(bit1, uint8_t, SI_SEG_CODE), uint8_t bit1_size,
		uint8_t bit_count)
{
	uint8_t last_bit = 0;

	// do init before first bit received
	if (BITS_GET(status[i]) == 0)
	{
		ABP_RESET(status[i]);
		memset(RF_DATA, 0, sizeof(RF_DATA));
		crc = 0x00;
	}

	// start decoding of the bits in sync of the buckets

	// bit 0
	if (CheckRFSyncBucket(duration, pulses[BUCKET_NR(bit0[BIT0_GET(status[i])])]))
	{
		// decode only if high/low does match
		if (BUCKET_STATE(bit0[BIT0_GET(status[i])]) == high_low)
		{
			if (BIT0_GET(status[i]) == 0)
				BIT_LOW = duration;

			BIT0_INC(status[i]);
		}
	}
	// bucket does not match bit, reset status
	else
	{
		BIT0_CLEAR(status[i]);
	}

	// bit 1
	if (CheckRFSyncBucket(duration, pulses[BUCKET_NR(bit1[BIT1_GET(status[i])])]))
	{
		// decode only if high/low does match
		if (BUCKET_STATE(bit1[BIT1_GET(status[i])]) == high_low)
		{
			if (BIT1_GET(status[i]) == 0)
				BIT_HIGH = duration;

			BIT1_INC(status[i]);
		}
	}
	// bucket does not match bit, reset status
	else
	{
		BIT1_CLEAR(status[i]);
	}

	// check if any bucket got decoded, if not restart
	if ((BIT0_GET(status[i]) == 0) && (BIT1_GET(status[i]) == 0))
	{
		LED = LED_OFF;
		START_CLEAR(status[i]);
		return false;
	}

	// on the last bit do not check the last bucket
	// because maybe this is not correct because a
	// repeat delay
	if (BITS_GET(status[i]) == bit_count - 1)
		last_bit = 1;

	// check if bit 0 is finished
	if (BIT0_GET(status[i]) == bit0_size - last_bit)
	{
		LED = LED_ON;
		BITS_CLEAR(status[i]);
		BITS_INC(status[i]);
		ABP_DEC(status[i]);
	}
	// check if bit 1 is finished
	else if (BIT1_GET(status[i]) == bit1_size - last_bit)
	{
		LED = LED_ON;
		BITS_CLEAR(status[i]);
		BITS_INC(status[i]);
		ABP_DEC(status[i]);
		RF_DATA[(BITS_GET(status[i]) - 1) >> 3] |= (1 << ABP_GET(status[i]));
	}

	// 8 bits are done, compute crc of data
	if (ABP_GET(status[i]) == 0)
	{
		crc = Compute_CRC8_Simple_OneByte(crc ^ RF_DATA[(BITS_GET(status[i]) - 1) >> 3]);
		ABP_RESET(status[i]);
	}

	// check if all bit got collected
	if (BITS_GET(status[i]) >= bit_count)
	{
		// check if timeout timer for crc is finished
		if (IsTimer2Finished())
			old_crc = 0;

		// check new crc on last received data for debounce
		if (crc != old_crc)
		{
			// new data, restart crc timeout
			StopTimer2();
			InitTimer2_ms(1, 800);
			old_crc = crc;

			// disable interrupt for RF receiving while uart transfer
			PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;

			// set status
			RF_DATA_STATUS = i;
			RF_DATA_STATUS |= RF_DATA_RECEIVED_MASK;
		}

		LED = LED_OFF;
		START_CLEAR(status[i]);
		return true;
	}

	return false;
}

void HandleRFBucket(uint16_t duration, bool high_low)
{
	uint8_t i = 0;

	// if noise got received stop all running decodings
	if (duration < MIN_BUCKET_LENGTH)
	{
		for (i = 0; i < PROTOCOLCOUNT; i++)
			START_CLEAR(status[i]);

		LED = LED_OFF;
		return;
	}

	// handle the buckets by standard or advanced decoding
	switch(sniffing_mode)
	{
		case STANDARD:
			// check if protocol was not started
			if (START_GET(status[0]) == 0)
			{
				// if PT226x standard sniffing calculate the pulse time by the longer sync bucket
				// this will enable receive PT226x in a range of PT226x_SYNC_MIN <-> 32767�s
				if (duration > PT226x_SYNC_MIN && !high_low) // && (duration < PT226x_SYNC_MAX))
				{
					// increment start because of the skipped first high bucket
					START_INC(status[0]);
					START_INC(status[0]);
					SYNC_LOW = duration;

					buckets[0] = duration / 31;
					buckets[1] = buckets[0] * 3;
					buckets[2] = duration;
				}
			}
			// if sync is finished check if bit0 or bit1 is starting
			else if (START_GET(status[0]) == 2)
			{
				DecodeBucket(0, high_low, duration,
						buckets,
						PROTOCOL_DATA[0].bit0.dat, PROTOCOL_DATA[0].bit0.size,
						PROTOCOL_DATA[0].bit1.dat, PROTOCOL_DATA[0].bit1.size,
						PROTOCOL_DATA[0].bit_count
						);
			}
			break;

		case ADVANCED:
			// check each protocol for each bucket
			for (i = 0; i < PROTOCOLCOUNT; i++)
			{
				// protocol started, check if sync is finished
				if (START_GET(status[i]) < PROTOCOL_DATA[i].start.size)
				{
					// check if sync bucket high/low is matching
					if (BUCKET_STATE(PROTOCOL_DATA[i].start.dat[START_GET(status[i])]) != high_low)
						continue;

					if (CheckRFSyncBucket(duration, PROTOCOL_DATA[i].buckets.dat[BUCKET_NR(PROTOCOL_DATA[i].start.dat[START_GET(status[i])])]))
					{
						START_INC(status[i]);
						continue;
					}
					else
					{
						START_CLEAR(status[i]);
						continue;
					}
				}
				// if sync is finished check if bit0 or bit1 is starting
				else if (START_GET(status[i]) == PROTOCOL_DATA[i].start.size)
				{
					if (DecodeBucket(i, high_low, duration,
							PROTOCOL_DATA[i].buckets.dat,
							PROTOCOL_DATA[i].bit0.dat, PROTOCOL_DATA[i].bit0.size,
							PROTOCOL_DATA[i].bit1.dat, PROTOCOL_DATA[i].bit1.size,
							PROTOCOL_DATA[i].bit_count
							))
						return;
				}
			}
			break;
	}	// switch(sniffing_mode)
}

void buffer_in(uint16_t bucket)
{
	if ((GET_W_POSITION(buffer_buckets_positions) + 1 == GET_R_POSITION(buffer_buckets_positions)) ||
			(GET_R_POSITION(buffer_buckets_positions) == 0 && GET_W_POSITION(buffer_buckets_positions) + 1 == ARRAY_LENGTH(buffer_buckets)))
		return;

	buffer_buckets[GET_W_POSITION(buffer_buckets_positions)] = bucket;

	INC_W_POSITION(buffer_buckets_positions);

	if (GET_W_POSITION(buffer_buckets_positions) >= ARRAY_LENGTH(buffer_buckets))
		CLR_W_POSITION(buffer_buckets_positions);
}

bool buffer_out(SI_VARIABLE_SEGMENT_POINTER(bucket, uint16_t, SI_SEG_XDATA))
{
	uint8_t backup_PCA0CPM0 = PCA0CPM0;

	// check if buffer is empty
	if (GET_W_POSITION(buffer_buckets_positions) == GET_R_POSITION(buffer_buckets_positions))
		return false;

	// disable interrupt for RF receiving while reading buffer
	PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;

	*bucket = buffer_buckets[GET_R_POSITION(buffer_buckets_positions)];
	INC_R_POSITION(buffer_buckets_positions);

	if (GET_R_POSITION(buffer_buckets_positions) >= ARRAY_LENGTH(buffer_buckets))
		CLR_R_POSITION(buffer_buckets_positions);

	// reset register
	PCA0CPM0 = backup_PCA0CPM0;

	return true;
}

void PCA0_channel0EventCb()
{
	uint16_t current_capture_value = PCA0CP0 * 10;
	uint8_t flags = PCA0MD;

	// clear counter
	PCA0MD &= 0xBF;
	PCA0H = 0x00;
	PCA0L = 0x00;
	PCA0MD = flags;

	// if bucket is no noise add it to buffer
	if (/*current_capture_value > MIN_PULSE_LENGTH &&*/ current_capture_value < 0x8000)
	{
		buffer_in(current_capture_value | ((uint16_t)(!R_DATA) << 15));
	}
	else
	{
		// received noise, clear all received buckets
		buffer_buckets_positions = 0;
	}
}

void PCA0_channel1EventCb() { }

void PCA0_channel2EventCb() { }

void SetTimer0Overflow(uint8_t T0_Overflow)
{
	/***********************************************************************
	 - Timer 0 High Byte = T0_Overflow
	 ***********************************************************************/
	TH0 = (T0_Overflow << TH0_TH0__SHIFT);
}

uint8_t PCA0_DoSniffing(uint8_t active_command)
{
	uint8_t ret = last_sniffing_command;

	memset(status, 0, sizeof(PROTOCOL_STATUS) * PROTOCOLCOUNT);

	// restore timer to 100000Hz, 10�s interval
	SetTimer0Overflow(0x0B);

	// enable interrupt for RF receiving
	PCA0CPM0 |= PCA0CPM0_ECCF__ENABLED;

	// start PCA
	PCA0_run();

	InitTimer3_ms(1, 10);
	// wait until timer has finished
	WaitTimer3Finished();

	rf_state = RF_IDLE;
	RF_DATA_STATUS = 0;

	// set uart_command back if sniffing was on
	uart_command = active_command;

	// backup uart_command to be able to enable the sniffing again
	last_sniffing_command = active_command;

	return ret;
}

void PCA0_StopSniffing(void)
{
	// stop PCA
	PCA0_halt();

	// clear all interrupt flags of PCA0
	PCA0CN0 &= ~(PCA0CN0_CF__BMASK
	  		                       | PCA0CN0_CCF0__BMASK
	  		                       | PCA0CN0_CCF1__BMASK
	  		                       | PCA0CN0_CCF2__BMASK);

	// disable interrupt for RF receiving
	PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;

	// be sure the timeout timer is stopped
	StopTimer2();

	rf_state = RF_IDLE;
}

bool SendSingleBucket(bool high_low, uint16_t bucket_time)
{
	// switch to high_low
	LED = high_low;
	T_DATA = high_low;
	InitTimer3_us(10, bucket_time);
	// wait until timer has finished
	WaitTimer3Finished();
	return !high_low;
}

//-----------------------------------------------------------------------------
// Send generic signal based on n time bucket pairs (high/low timing)
//-----------------------------------------------------------------------------
#if INCLUDE_BUCKET_SNIFFING == 1
void SendRFBuckets(
		SI_VARIABLE_SEGMENT_POINTER(buckets, uint16_t, SI_SEG_XDATA),
		SI_VARIABLE_SEGMENT_POINTER(rfdata, uint8_t, SI_SEG_XDATA), uint8_t data_len)
{
	// start transmit of the buckets with a high bucket
	bool high_low = true;
	bool high_low_mark = false;
	uint8_t i;

	// check first two buckets if high/low marking is included
	high_low_mark = (rfdata[0] & 0x88) > 0;

	// transmit data
	for (i = 0; i < data_len; i++)
	{
			high_low = SendSingleBucket(high_low_mark ? (bool)(rfdata[i] >> 7) : high_low, buckets[(rfdata[i] >> 4) & 0x07]);
			high_low = SendSingleBucket(high_low_mark ? (bool)((rfdata[i] >> 3) & 0x01) : high_low, buckets[rfdata[i] & 0x07]);
	}

	LED = LED_OFF;

	rf_state = RF_FINISHED;
}
#endif

void SendBuckets(
		uint16_t *pulses,
		SI_VARIABLE_SEGMENT_POINTER(start, uint8_t, SI_SEG_CODE), uint8_t start_size,
		SI_VARIABLE_SEGMENT_POINTER(bit0, uint8_t, SI_SEG_CODE), uint8_t bit0_size,
		SI_VARIABLE_SEGMENT_POINTER(bit1, uint8_t, SI_SEG_CODE), uint8_t bit1_size,
		SI_VARIABLE_SEGMENT_POINTER(end, uint8_t, SI_SEG_CODE), uint8_t end_size,
		uint8_t bit_count,
		SI_VARIABLE_SEGMENT_POINTER(rfdata, uint8_t, SI_SEG_XDATA))
{
	uint8_t i, a;
	uint8_t actual_byte = 0;
	uint8_t actual_bit = 0x80;

	// transmit sync bucket(s)
	for (i = 0; i < start_size; i++)
		SendSingleBucket(BUCKET_STATE(start[i]), pulses[BUCKET_NR(start[i])]);

	// transmit bit bucket(s)
	for (i = 0; i < bit_count; i++)
	{
		// send bit 0
		if ((rfdata[actual_byte] & actual_bit) == 0)
		{
			for (a = 0; a < bit0_size; a++)
				SendSingleBucket(BUCKET_STATE(bit0[a]), pulses[BUCKET_NR(bit0[a])]);
		}
		// send bit 1
		else
		{
			for (a = 0; a < bit1_size; a++)
				SendSingleBucket(BUCKET_STATE(bit1[a]), pulses[BUCKET_NR(bit1[a])]);
		}

		actual_bit >>= 1;

		if (actual_bit == 0)
		{
			actual_byte++;
			actual_bit = 0x80;
		}
	}

	// transmit end bucket(s)
	for (i = 0; i < end_size; i++)
		SendSingleBucket(BUCKET_STATE(end[i]), pulses[BUCKET_NR(end[i])]);

	LED = LED_OFF;

	rf_state = RF_FINISHED;
}

void SendBucketsByIndex(uint8_t index, SI_VARIABLE_SEGMENT_POINTER(rfdata, uint8_t, SI_SEG_XDATA))
{
	SendBuckets(
			PROTOCOL_DATA[index].buckets.dat,
			PROTOCOL_DATA[index].start.dat, PROTOCOL_DATA[index].start.size,
			PROTOCOL_DATA[index].bit0.dat, PROTOCOL_DATA[index].bit0.size,
			PROTOCOL_DATA[index].bit1.dat, PROTOCOL_DATA[index].bit1.size,
			PROTOCOL_DATA[index].end.dat, PROTOCOL_DATA[index].end.size,
			PROTOCOL_DATA[index].bit_count,
			rfdata
			);
}

#if INCLUDE_BUCKET_SNIFFING == 1
bool probablyFooter(uint16_t duration)
{
	return duration >= MIN_FOOTER_LENGTH;
}

bool matchesFooter(uint16_t duration, bool high_low)
{
	if (!((bucket_sync & 0x8000) >> 15) && high_low)
		return false;

	return CheckRFSyncBucket(duration, bucket_sync & 0x7FFF);
}

bool findBucket(uint16_t duration, uint8_t *index)
{
	uint8_t i;
	uint16_t delta;

	for (i = 0; i < bucket_count; i++)
	{
		// calculate delta by the current duration and check if the new duration fits into
		delta = ((duration >> 2) + (duration >> 3));
		delta = delta > buckets[i] ? buckets[i] : delta;

		if (CheckRFBucket(duration, buckets[i], delta))
		{
			*index = i;
			return true;
		}
	}

	return false;
}

void Bucket_Received(uint16_t duration, bool high_low)
{
	uint8_t bucket_index;

	// if pulse is too short reset status
	if (duration < MIN_BUCKET_LENGTH)
	{
		rf_state = RF_IDLE;
		return;
	}

	switch (rf_state)
	{
		// check if we maybe receive a sync
		case RF_IDLE:
			LED = LED_OFF;

			if (probablyFooter(duration))
			{
				bucket_sync = duration | ((uint16_t)high_low << 15);
				bucket_count_sync_1 = 0;
				rf_state = RF_BUCKET_REPEAT;
			}
			break;

		// check if the same bucket gets received
		case RF_BUCKET_REPEAT:
			if (matchesFooter(duration, high_low))
			{
				// check if a minimum of buckets where between two sync pulses
				if (bucket_count_sync_1 > 4)
				{
					LED = LED_ON;
					bucket_count = 0;
					actual_byte = 0;
					actual_byte_high_nibble = false;
					bucket_count_sync_2 = 0;
					crc = 0x00;
					RF_DATA[0] = 0;
					rf_state = RF_BUCKET_IN_SYNC;
				}
				else
				{
					rf_state = RF_IDLE;
				}
			}
			// check if duration is longer than sync bucket restart
			else if (duration > (bucket_sync & 0x7FFF))
			{
				// this bucket looks like the sync bucket
				bucket_sync = duration | ((uint16_t)high_low << 15);
				bucket_count_sync_1 = 0;
			}
			else
			{
				bucket_count_sync_1++;
			}

			// no more buckets are possible, reset
			if (bucket_count_sync_1 >= RF_DATA_BUFFERSIZE << 1)
				rf_state = RF_IDLE;

			break;

		// same sync bucket got received, filter buckets
		case RF_BUCKET_IN_SYNC:
			bucket_count_sync_2++;

			// check if all buckets got received
			if (bucket_count_sync_2 <= bucket_count_sync_1)
			{
				// check if bucket was already received
				if (!findBucket(duration, &bucket_index))
				{
					// new bucket received, add to array
					buckets[bucket_count] = duration;
					bucket_index = bucket_count;
					bucket_count++;

					// check if maximum of array got reached
					if (bucket_count > ARRAY_LENGTH(buckets))
					{
						// restart sync
						rf_state = RF_IDLE;
					}
				}

				// fill rf data with the current bucket number
				if (actual_byte_high_nibble)
				{
					RF_DATA[actual_byte] = (bucket_index << 4) | ((uint8_t)high_low << 7);
				}
				else
				{
					RF_DATA[actual_byte] |= (bucket_index | ((uint8_t)high_low << 3));

					crc = Compute_CRC8_Simple_OneByte(crc ^ RF_DATA[actual_byte]);

					actual_byte++;

					// check if maximum of array got reached
					if (actual_byte > RF_DATA_BUFFERSIZE)
					{
						// restart sync
						rf_state = RF_IDLE;
					}
				}

				actual_byte_high_nibble = !actual_byte_high_nibble;
			}
			// next bucket after data have to be a sync bucket
			else if (matchesFooter(duration, high_low))
			{
				// check if timeout timer for crc is finished
				if (IsTimer2Finished())
					old_crc = 0;

				// check new crc on last received data for debounce
				if (crc != old_crc)
				{
					// new data, restart crc timeout
					StopTimer2();
					InitTimer2_ms(1, 800);
					old_crc = crc;

					// disable interrupt for RF receiving while uart transfer
					PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;

					// add sync bucket number to data
					RF_DATA[0] |= ((bucket_count << 4) | ((bucket_sync & 0x8000) >> 8));

					// clear high/low flag
					bucket_sync &= 0x7FFF;

					RF_DATA_STATUS |= RF_DATA_RECEIVED_MASK;
				}

				LED = LED_OFF;
				rf_state = RF_IDLE;
			}
			// next bucket after receiving all data buckets was not a sync bucket, restart
			else
			{
				// restart sync
				rf_state = RF_IDLE;
			}
			break;
	}
}
#endif
