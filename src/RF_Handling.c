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
SI_SEGMENT_VARIABLE(desired_rf_protocol, uint8_t, SI_SEG_XDATA) = UNDEFINED_INDEX;
SI_SEGMENT_VARIABLE(rf_sniffing_mode, rf_sniffing_mode_t, SI_SEG_XDATA) = MODE_TIMING;
SI_SEGMENT_VARIABLE(protocol_index_in_sync, uint8_t, SI_SEG_XDATA) = 0x80;

SI_SEGMENT_VARIABLE(last_sniffing_command, uint8_t, SI_SEG_XDATA) = NONE;

SI_SEGMENT_VARIABLE(SYNC_LOW, uint16_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(BIT_HIGH, uint16_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(BIT_LOW, uint16_t, SI_SEG_XDATA) = 0x00;

SI_SEGMENT_VARIABLE(actual_bit_of_byte, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(actual_bit, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(actual_sync_bit, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(actual_byte, uint8_t, SI_SEG_XDATA) = 0;

SI_SEGMENT_VARIABLE(old_crc, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(crc, uint8_t, SI_SEG_XDATA) = 0;

// up to 8 timing buckets for MODE_BUCKET
SI_SEGMENT_VARIABLE(bucket_sync, uint16_t, SI_SEG_XDATA);
SI_SEGMENT_VARIABLE(buckets[15], uint16_t, SI_SEG_XDATA);	// -1 because of the bucket_sync
SI_SEGMENT_VARIABLE(bucket_count, uint8_t, SI_SEG_XDATA) = 0;

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

uint8_t CheckRFSync_pos(uint16_t period_pos, uint16_t sync_high, uint16_t sync_high_delta)
{
	uint8_t ret = false;
	if ((period_pos > (sync_high - sync_high_delta)) &&
		(period_pos < (sync_high + sync_high_delta)))
	{
		ret = true;
	}
	return ret;
}

uint8_t CheckRFSync_neg(uint16_t period_neg, uint16_t sync_low, uint16_t sync_low_delta)
{
	uint8_t ret = false;
	if ((period_neg > (sync_low - sync_low_delta)) &&
		(period_neg < (sync_low + sync_low_delta)))
	{
		ret = true;
	}
	return ret;
}

uint8_t CheckRFSync(uint8_t protocol_index, uint8_t inverse, uint16_t period_pos, uint16_t period_neg)
{
	uint8_t ret = false;
	uint16_t pulse_time;
	uint16_t sync_high_delta;
	uint16_t sync_low_delta;

	// ignore protocols what aren't matching with positive/negative edge
	if (PROTOCOL_DATA[protocol_index].INVERSE != inverse)
		return ret;

	// check if sync high and low is defined
	if ((PROTOCOL_DATA[protocol_index].SYNC.HIGH > 0) && (PROTOCOL_DATA[protocol_index].SYNC.LOW > 0))
	{
		// use calculated pulse time for PT2260 devices to decode the data
		if (protocol_index == PT2260_INDEX)
		{
			pulse_time = PROTOCOL_DATA[protocol_index].SYNC.HIGH > PROTOCOL_DATA[protocol_index].SYNC.LOW ?
					period_pos / PROTOCOL_DATA[protocol_index].SYNC.HIGH : period_neg / PROTOCOL_DATA[protocol_index].SYNC.LOW;
		}
		else
		{
			pulse_time = PROTOCOL_DATA[protocol_index].PULSE_TIME;
		}

		sync_high_delta = (period_pos / 100 * SYNC_TOLERANCE) > SYNC_TOLERANCE_MAX ? SYNC_TOLERANCE_MAX : (period_pos / 100 * SYNC_TOLERANCE);
		sync_low_delta = (period_neg / 100 * SYNC_TOLERANCE) > SYNC_TOLERANCE_MAX ? SYNC_TOLERANCE_MAX : (period_neg / 100 * SYNC_TOLERANCE);

		if (CheckRFSync_pos(period_pos, pulse_time * PROTOCOL_DATA[protocol_index].SYNC.HIGH, sync_high_delta) &&
			CheckRFSync_neg(period_neg, pulse_time * PROTOCOL_DATA[protocol_index].SYNC.LOW, sync_low_delta))
		{
			BIT_LOW = pulse_time * PROTOCOL_DATA[protocol_index].BIT0.HIGH;
			BIT_HIGH = pulse_time * PROTOCOL_DATA[protocol_index].BIT1.HIGH;
			ret = true;
		}
	}
	else if (PROTOCOL_DATA[protocol_index].SYNC.LOW > 0)
	{
		pulse_time = PROTOCOL_DATA[protocol_index].PULSE_TIME;

		sync_low_delta = (period_neg / 100 * SYNC_TOLERANCE) > SYNC_TOLERANCE_MAX ? SYNC_TOLERANCE_MAX : (period_neg / 100 * SYNC_TOLERANCE);

		if (CheckRFSync_neg(period_neg, pulse_time * PROTOCOL_DATA[protocol_index].SYNC.LOW, sync_low_delta))
		{
			BIT_LOW = pulse_time * PROTOCOL_DATA[protocol_index].BIT0.HIGH;
			BIT_HIGH = pulse_time * PROTOCOL_DATA[protocol_index].BIT1.HIGH;
			ret = true;
		}
	}
	else if (PROTOCOL_DATA[protocol_index].SYNC.HIGH > 0)
	{
		pulse_time = PROTOCOL_DATA[protocol_index].PULSE_TIME;

		sync_high_delta = (period_pos / 100 * SYNC_TOLERANCE) > SYNC_TOLERANCE_MAX ? SYNC_TOLERANCE_MAX : (period_pos / 100 * SYNC_TOLERANCE);

		if (CheckRFSync_neg(period_pos, pulse_time * PROTOCOL_DATA[protocol_index].SYNC.HIGH, sync_high_delta))
		{
			BIT_LOW = pulse_time * PROTOCOL_DATA[protocol_index].BIT0.HIGH;
			BIT_HIGH = pulse_time * PROTOCOL_DATA[protocol_index].BIT1.HIGH;
			ret = true;
		}
	}

	return ret;
}

//-----------------------------------------------------------------------------
// Check for a RF sync
//-----------------------------------------------------------------------------
uint8_t RFInSync(uint8_t protocol_index, uint8_t inverse, uint16_t period_pos, uint16_t period_neg)
{
	uint8_t ret = 0x80;
	uint8_t index;

	switch(protocol_index)
	{
		// protocol is undefined, do loop through all protocols
		case UNDEFINED_INDEX:

			// check all protocols
			for (index = 0x00 ; index < PROTOCOLCOUNT; index++)
			{
				if(CheckRFSync(index, inverse, period_pos, period_neg))
				{
					ret = index;
					break;
				}
			}
			break;

		// check other protocols
		default:

			if (CheckRFSync(protocol_index, inverse, period_pos, period_neg))
			{
				ret = protocol_index;
			}

			break;
	}

	return ret;
}

void HandleRFData(uint8_t inverse, uint16_t capture_period_pos, uint16_t capture_period_neg)
{
	SI_SEGMENT_VARIABLE(capture_period_pos_s, int32_t, SI_SEG_XDATA);
	SI_SEGMENT_VARIABLE(capture_period_neg_s, int32_t, SI_SEG_XDATA);
	static uint16_t low_pulse_time;

	switch (rf_state)
	{
		// check if we receive a sync
		case RF_IDLE:
			// check first if last decoded RF signal was cleared
			if (RF_DATA_STATUS != 0)
				break;

			// check if maybe a snyc got received
			if ((capture_period_pos < MIN_FOOTER_LENGTH) && (capture_period_neg < MIN_FOOTER_LENGTH))
				break;

			// check all protocols in the list
			protocol_index_in_sync = RFInSync(desired_rf_protocol, inverse, capture_period_pos, capture_period_neg);

			// check if a matching protocol got found
			if (protocol_index_in_sync != 0x80)
			{
				// backup sync time
				SYNC_LOW = capture_period_neg;
				actual_bit_of_byte = 8;
				actual_bit = 0;
				actual_sync_bit = 0;
				low_pulse_time = 0;
				memset(RF_DATA, 0, sizeof(RF_DATA));
				crc = 0x00;
				LED = LED_ON;
				rf_state = RF_IN_SYNC;
				break;
			}
			break;

		// one matching sync got received
		case RF_IN_SYNC:
		{
			uint16_t delayTolerance = PROTOCOL_DATA[protocol_index_in_sync].PULSE_TIME *
					PROTOCOL_DATA[protocol_index_in_sync].TOLERANCE / 100;

			SI_SEGMENT_VARIABLE(bit0_high, uint16_t, SI_SEG_XDATA);
			SI_SEGMENT_VARIABLE(bit0_low, uint16_t, SI_SEG_XDATA);
			SI_SEGMENT_VARIABLE(bit1_high, uint16_t, SI_SEG_XDATA);
			SI_SEGMENT_VARIABLE(bit1_low, uint16_t, SI_SEG_XDATA);

			// at first skip SYNC bits
			if ((PROTOCOL_DATA[protocol_index_in_sync].SYNC_BIT_COUNT > 0) &&
				(actual_sync_bit < PROTOCOL_DATA[protocol_index_in_sync].SYNC_BIT_COUNT))
			{
				actual_sync_bit++;
				break;
			}

			if (protocol_index_in_sync != PT2260_INDEX)
			{
				bit0_high = PROTOCOL_DATA[protocol_index_in_sync].PULSE_TIME * PROTOCOL_DATA[protocol_index_in_sync].BIT0.HIGH;
				bit0_low = PROTOCOL_DATA[protocol_index_in_sync].PULSE_TIME * PROTOCOL_DATA[protocol_index_in_sync].BIT0.LOW;
				bit1_high = PROTOCOL_DATA[protocol_index_in_sync].PULSE_TIME * PROTOCOL_DATA[protocol_index_in_sync].BIT1.HIGH;
				bit1_low = PROTOCOL_DATA[protocol_index_in_sync].PULSE_TIME * PROTOCOL_DATA[protocol_index_in_sync].BIT1.LOW;
			}
			else
			{
				bit0_high = BIT_LOW;
				bit0_low = BIT_HIGH;
				bit1_high = BIT_HIGH;
				bit1_low = BIT_LOW;
				delayTolerance = BIT_LOW * PROTOCOL_DATA[protocol_index_in_sync].TOLERANCE / 100;
			}

			// check the rest of the bits
			actual_bit_of_byte--;
			actual_bit++;

			// use signed variables
			capture_period_pos_s = capture_period_pos;
			capture_period_neg_s = capture_period_neg;

			// check if bit is a logic 0 or 1
			if(((abs(capture_period_pos_s - bit0_high) < delayTolerance) &&
				(abs(capture_period_neg_s - bit0_low) < delayTolerance)) ||
					// the timing can not be used for the last bit because of the missing rising edge on the end
					((capture_period_pos < low_pulse_time) && (actual_bit == PROTOCOL_DATA[protocol_index_in_sync].BIT_COUNT)))
			{
				// backup last bit high time
				BIT_LOW = capture_period_pos;
				LED = LED_OFF;

				// backup low bit pulse time to be able to determine the last bit
				if (capture_period_pos > low_pulse_time)
					low_pulse_time = capture_period_pos;
			}
			else if(((abs(capture_period_pos_s - bit1_high) < delayTolerance) &&
					 (abs(capture_period_neg_s - bit1_low) < delayTolerance)) ||
					// the timing can not be used for the last bit because of the missing rising edge on the end
					((capture_period_pos > low_pulse_time) && (actual_bit == PROTOCOL_DATA[protocol_index_in_sync].BIT_COUNT)))
			{
				// backup last bit high time
				BIT_HIGH = capture_period_pos;
				LED = LED_ON;
				RF_DATA[(actual_bit - 1) / 8] |= (1 << actual_bit_of_byte);
			}
			else
			{
				RF_DATA_STATUS = 0;
				LED = LED_OFF;
				rf_state = RF_IDLE;
				protocol_index_in_sync = 0x80;
				break;
			}

			// 8 bits are done, compute crc of data
			if (actual_bit_of_byte == 0)
			{
				crc = Compute_CRC8_Simple_OneByte(crc ^ RF_DATA[(actual_bit - 1) / 8]);
				actual_bit_of_byte = 8;
			}

			// check if all bits for this protocol got received
			if (actual_bit == PROTOCOL_DATA[protocol_index_in_sync].BIT_COUNT)
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
					RF_DATA_STATUS = protocol_index_in_sync;
					RF_DATA_STATUS |= RF_DATA_RECEIVED_MASK;
					protocol_index_in_sync = 0x80;
				}

				LED = LED_OFF;
				rf_state = RF_IDLE;
			}
			break;
		}
	}
}

void PCA0_channel0EventCb()
{
	SI_SEGMENT_VARIABLE(current_capture_value, uint16_t, SI_SEG_XDATA);
	static uint16_t previous_capture_value_pos, previous_capture_value_neg;
	static uint16_t capture_period_pos, capture_period_neg;

	// Store most recent capture value
	current_capture_value = PCA0CP0 * 10;

	// ignore RF noise
	if (current_capture_value < MIN_PULSE_LENGTH)
		return;

	// positive edge
	if (R_DATA)
	{
		// Update previous capture value with most recent info.
		previous_capture_value_pos = current_capture_value;

		// Calculate capture period from last two values.
		capture_period_neg = current_capture_value - previous_capture_value_neg;

		// do sniffing by mode
		switch (rf_sniffing_mode)
		{
			// do sniffing by timing mode
			case MODE_TIMING:
				// check if a decode is already started
				if (protocol_index_in_sync != 0x80)
					if(PROTOCOL_DATA[protocol_index_in_sync].INVERSE == true)
						return;

				HandleRFData(false, capture_period_pos, capture_period_neg);
				break;

			// do sniffing by bucket mode
			case MODE_BUCKET:
				Bucket_Received(capture_period_neg);
				break;
		}
	}
	// negative edge
	else
	{
		// Update previous capture value with most recent info.
		previous_capture_value_neg = current_capture_value;

		// Calculate capture period from last two values.
		capture_period_pos = current_capture_value - previous_capture_value_pos;

		// do sniffing by mode
		switch (rf_sniffing_mode)
		{
			// do sniffing by timing mode
			case MODE_TIMING:
				// check if a decode is already started
				if (protocol_index_in_sync != 0x80)
					if(PROTOCOL_DATA[protocol_index_in_sync].INVERSE == false)
						return;

				HandleRFData(true, capture_period_neg, capture_period_pos);
				break;

			// do sniffing by bucket mode
			case MODE_BUCKET:
				Bucket_Received(capture_period_pos);
				break;
		}
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

	// restore timer to 100000Hz, 10µs interval
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

void SendSingleBit(uint16_t high_time, uint16_t low_time)
{
	// switch to high
	LED = LED_ON;
	T_DATA = TDATA_ON;
	InitTimer3_us(10, high_time);
	// wait until timer has finished
	WaitTimer3Finished();

	// switch to low
	LED = LED_OFF;
	T_DATA = TDATA_OFF;
	InitTimer3_us(10, low_time);
	// wait until timer has finished
	WaitTimer3Finished();
}
//-----------------------------------------------------------------------------
// Send generic signal based on n time bucket pairs (high/low timing)
//-----------------------------------------------------------------------------
void SendRFBuckets(const uint16_t buckets[], const uint8_t rfdata[], uint8_t n, uint8_t repeats)
{
	do
	{
		uint8_t i;

		for (i = 0; i < n; i++)			// transmit n bucket pairs
			SendSingleBit(buckets[rfdata[i] >> 4], buckets[rfdata[i] & 0x0f]);
	}
	while (repeats-- != 0);				// how many times do I need to repeat?

	LED = LED_OFF;

	rf_state = RF_FINISHED;
}

void SendTimingProtocol(uint16_t sync_high, uint16_t sync_low,
		uint16_t bit_0_high, uint16_t bit_0_low, uint16_t bit_1_high, uint16_t bit_1_low,
		uint8_t sync_bits, uint8_t bitcount, uint8_t position)
{
	uint8_t i;
	uint8_t actual_position = position;
	uint8_t actual_bit_of_byte = 0x80;

	// do send sync
	SendSingleBit(sync_high, sync_low);

	// send sync bits if used
	if (sync_bits > 0)
	{
		for (i = 0; i < sync_bits; i++)
		{
			// send a bit 1
			SendSingleBit(bit_1_high, bit_1_low);
		}
	}

	// send RF data
	for (i = 0; i < bitcount; i++)
	{
		if (RF_DATA[actual_position] & actual_bit_of_byte)
		{
			// send a bit 1
			SendSingleBit(bit_1_high, bit_1_low);
		}
		else
		{
			// send a bit 0
			SendSingleBit(bit_0_high, bit_0_low);
		}

		actual_bit_of_byte >>= 1;

		if (actual_bit_of_byte == 0)
		{
			actual_position++;
			actual_bit_of_byte = 0x80;
		}
	}

	LED = LED_OFF;

	rf_state = RF_FINISHED;
}


bool probablyFooter(uint16_t duration)
{
	return duration >= MIN_FOOTER_LENGTH;
}

bool matchesFooter(uint16_t duration)
{
  uint16_t footer_delta = bucket_sync / 100 * SYNC_TOLERANCE;
  footer_delta = footer_delta > SYNC_TOLERANCE_MAX ? SYNC_TOLERANCE_MAX : footer_delta;
  return (((bucket_sync - footer_delta) < duration) && (duration < (bucket_sync + footer_delta)));
}

bool findBucket(uint16_t duration, uint8_t *index)
{
	bool ret = false;
	uint8_t i;

	for (i = 0; i < bucket_count; i++)
	{
		// calculate delta by the current bucket and check if the new duration fits into
		uint16_t delta = duration / 4 + duration / 8;
		delta = delta > buckets[i] ? buckets[i] : delta;

		if (((buckets[i] - delta) < duration) && (duration < (buckets[i] + delta)))
		{
			if (index != NULL)
				*index = i;

			ret = true;
			break;
		}
	}

	return ret;
}

bool definedBucket(uint16_t duration, uint8_t *index)
{
	bool ret = false;
	uint16_t delta;
	uint8_t i;

	//search new duration first in existing array
	if(findBucket(duration, index))
	{
		ret = true;
	}
	else
	{
		// check it the other way if a existing bucket will match in the new duration
		for (i = 0; i < bucket_count; i++)
		{
			delta = buckets[i] / 4 + buckets[i] / 8 + buckets[i] / 16;
			if (((duration - delta) < buckets[i]) && (buckets[i] < (duration + delta)))
			{
				if (index != NULL)
					*index = i;

				ret = true;
				break;
			}
		}
	}

	return ret;
}

void CheckUsedBuckets(uint8_t data_len)
{
	uint8_t i, x, a, removed_buckets = 0;

	// mark first all used buckets
	for (i = 0; i < (data_len + 1); i++)
	{
		if ((buckets[RF_DATA[i] >> 4] & 0x7FFF) > 0)
			buckets[RF_DATA[i] >> 4] |= 0x8000;

		if ((buckets[RF_DATA[i] & 0x0F] & 0x7FFF) > 0)
			buckets[RF_DATA[i] & 0x0F] |= 0x8000;
	}

	// move buckets forward
	i = 0;
	while (i < bucket_count)
	{
		if ((buckets[i] & 0x8000) == 0)
		{
			x = i;
			while (x < bucket_count)
			{
				buckets[x] = buckets[x + 1];

				// replace all x+1 with x in RF data
				for (a= 0; a < (data_len + 1); a++)
				{
					if ((RF_DATA[a] >> 4) == (x + 1))
						RF_DATA[a] = (x << 4) | (RF_DATA[a] & 0x0F);

					if ((RF_DATA[a] & 0x0F) == (x + 1))
						RF_DATA[a] = (RF_DATA[a] & 0xF0) | x;
				}
				x++;
			}

			removed_buckets++;
		}
		else
		{
			// clear used mark
			buckets[i] &= 0x7FFF;
		}

		i++;
	}

	bucket_count -= removed_buckets;
}

void Bucket_Received(uint16_t duration)
{
	uint8_t bucket_index;

	switch (rf_state)
	{
		// check if we receive a sync
		case RF_IDLE:
			LED = LED_OFF;
			// check first if last decoded RF signal was cleared
			if (RF_DATA_STATUS != 0)
				break;

			if (probablyFooter(duration))
			{
				// clean buckets
				for (bucket_index = 0; bucket_index < (sizeof(buckets)/sizeof(buckets[0])); bucket_index++)
					buckets[bucket_index] = 0;

				bucket_count = 0;
				bucket_index = 0;
				bucket_sync = duration;
				rf_state = RF_IN_SYNC;
				LED = LED_ON;
			}
			break;
		// one matching sync got received
		case RF_IN_SYNC:
			// check if duration is longer than sync bucket +25% and restart
			if (duration > (bucket_sync + bucket_sync / 4))
			{
				// this bucket looks like the sync bucket
				bucket_sync = duration;
				bucket_count = 0;
			}
			// check if this bucket is a sync bucket, receive is complete
			else if (matchesFooter(duration) && (bucket_count > 0))
			{
				// all buckets got received, start decoding on the next repeat
				actual_byte = 0;
				actual_bit_of_byte = 4;
				crc = 0x00;
				rf_state = RF_DECODE_BUCKET;
			}
			// check if useful bucket got received
			else if (duration >= MIN_PULSE_LENGTH)
			{
				// check if bucket was already received
				if (definedBucket(duration, &bucket_index))
				{
					// make new average of this bucket
					//buckets[bucket_index] = (buckets[bucket_index] + duration) / 2;
				}
				else
				{
					// new bucket received, add to array
					buckets[bucket_count] = duration;
					bucket_count++;

					// check if maximum of array got reached
					if (bucket_count > (sizeof(buckets)/sizeof(buckets[0])))
					{
						bucket_count = 0;
						// restart sync
						rf_state = RF_IDLE;
					}
				}
			}
			else
			{
				// restart sync
				rf_state = RF_IDLE;
			}
			break;

		// do decoding of the received buckets
		case RF_DECODE_BUCKET:
			// toggle led
			LED = !LED;
			// check if bucket can be decoded
			if (findBucket(duration, &bucket_index))
			{
				if (actual_bit_of_byte == 4)
				{
					RF_DATA[actual_byte] = bucket_index << 4;
					actual_bit_of_byte = 0;
				}
				else
				{
					RF_DATA[actual_byte] |= (bucket_index & 0x0F);

					// 8 bits are done, compute crc of data
					crc = Compute_CRC8_Simple_OneByte(crc ^ RF_DATA[actual_byte]);

					actual_byte++;
					actual_bit_of_byte = 4;

					// check if maximum of array got reached
					if (actual_byte > RF_DATA_BUFFERSIZE)
					{
						StopTimer2();
						bucket_count = 0;
						// restart sync
						rf_state = RF_IDLE;
					}
				}
			}
			// check if sync bucket got received
			else if (matchesFooter(duration))
			{
				// check if timeout timer for crc is finished
				if (IsTimer2Finished())
					old_crc = 0;

				// check new crc on last received data for debounce
				if (crc != old_crc)
				{
					// new data, restart crc timeout
					StopTimer2();
					InitTimer2_ms(1, 500);
					old_crc = crc;

					// add sync bucket number to data
					if (actual_bit_of_byte == 0)
					{
						RF_DATA[actual_byte] |= (bucket_count & 0x0F);
					}
					else
					{
						RF_DATA[actual_byte] = (bucket_count << 4) | 0x0F;
					}

					// remove unused buckets
					CheckUsedBuckets(actual_byte);

					RF_DATA_STATUS |= RF_DATA_RECEIVED_MASK;
				}

				LED = LED_OFF;
				rf_state = RF_IDLE;
			}
			else
			{
				StopTimer2();
				// bucket not found in list, restart
				bucket_count = 0;
				// restart sync
				rf_state = RF_IDLE;
			}
			break;
	}
}
