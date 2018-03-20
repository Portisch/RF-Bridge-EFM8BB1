/*
 * RF_Handling.c
 *
 *  Created on: 27.11.2017
 *      Author:
 */

#include <SI_EFM8BB1_Register_Enums.h>
#include <string.h>
#include "Globals.h"
#include "RF_Handling.h"
#include "RF_Protocols.h"
#include "pca_0.h"
#include "uart.h"

SI_SEGMENT_VARIABLE(RF_DATA[RF_DATA_BUFFERSIZE], uint8_t, SI_SEG_XDATA);
SI_SEGMENT_VARIABLE(RF_DATA_STATUS, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(rf_state, rf_state_t, SI_SEG_XDATA) = RF_IDLE;
SI_SEGMENT_VARIABLE(desired_rf_protocol, uint8_t, SI_SEG_XDATA) = UNKNOWN_IDENTIFIER;
SI_SEGMENT_VARIABLE(rf_sniffing_mode, rf_sniffing_mode_t, SI_SEG_XDATA) = MODE_DUTY_CYCLE;

SI_SEGMENT_VARIABLE(last_sniffing_command, uint8_t, SI_SEG_XDATA) = NONE;

SI_SEGMENT_VARIABLE(DUTY_CYCLE_HIGH, uint8_t, SI_SEG_XDATA) = 0x56;
SI_SEGMENT_VARIABLE(DUTY_CYLCE_LOW, uint8_t, SI_SEG_XDATA) = 0xAB;
SI_SEGMENT_VARIABLE(T0_HIGH, uint8_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(T0_LOW, uint8_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(SYNC_HIGH, uint16_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(SYNC_LOW, uint16_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(BIT_HIGH, uint16_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(BIT_LOW, uint16_t, SI_SEG_XDATA) = 0x00;
SI_SEGMENT_VARIABLE(BIT_COUNT, uint8_t, SI_SEG_XDATA) = 0x00;

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
void PCA0_overflowCb()
{
}

void PCA0_intermediateOverflowCb()
{
	if (actual_bit > BIT_COUNT)
	{
		PCA0_StopTransmit();
	}
	else
	{
		if(((RF_DATA[actual_byte] >> actual_bit_of_byte) & 0x01) == 0x01)
		{
			// bit 1
			SetTimer0Overflow(T0_HIGH);
			LED = LED_ON;
		}
		else
		{
			// bit 0
			SetTimer0Overflow(T0_LOW);
			LED = LED_OFF;
		}
	}
}

void PCA0_channel0EventCb()
{
	// stop transfer if all bits are transmitted
	if (actual_bit_of_byte == 0)
	{
		actual_byte++;
		actual_bit_of_byte = 8;
	}

	actual_bit++;
	actual_bit_of_byte--;

	// set duty cycle for the next bit...
	if (actual_bit <= BIT_COUNT)
	{
		SetPCA0DutyCylce();
	}
	else
	{
		// remove spike on end of transmission
		// enable P0.0 for I/O control
		XBR1 &= ~XBR1_PCA0ME__CEX0_CEX1;
		// switch to low
		T_DATA = 0;
	}
}

void PCA0_channel1EventCb()
{
	static uint16_t current_capture_value;
	static uint16_t previous_capture_value_pos, previous_capture_value_neg;
	static uint16_t capture_period_pos, capture_period_neg;

	static uint8_t used_protocol;
	static uint16_t low_pulse_time;
	uint8_t current_duty_cycle;

	// Store most recent capture value
	current_capture_value = PCA0CP1 * 10;

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
			// do sniffing by duty cycle mode
			case MODE_DUTY_CYCLE:
				switch (rf_state)
				{
					// check if we receive a sync
					case RF_IDLE:
						// check first if last decoded RF signal was cleared
						if (RF_DATA_STATUS != 0)
							break;

						// check all protocols in the list
						used_protocol = RFInSync(desired_rf_protocol, capture_period_pos, capture_period_neg);

						// check if a matching protocol got found
						if (used_protocol != 0x80)
						{
							// backup sync time
							SYNC_HIGH = capture_period_pos;
							SYNC_LOW = capture_period_neg;
							actual_bit_of_byte = 8;
							actual_byte = 0;
							actual_bit = 0;
							actual_sync_bit = 0;
							low_pulse_time = 0;
							memset(RF_DATA, 0, sizeof(RF_DATA));
							crc = 0x00;
							rf_state = RF_IN_SYNC;
							break;
						}
						break;

					// one matching sync got received
					case RF_IN_SYNC:
						// at first skip SYNC bits
						if ((PROTOCOL_DATA[used_protocol].SYNC_BIT_COUNT > 0) &&
							(actual_sync_bit < PROTOCOL_DATA[used_protocol].SYNC_BIT_COUNT))
						{
							actual_sync_bit++;
							break;
						}

						// check the rest of the bits
						actual_bit_of_byte--;
						actual_bit++;

						// calculate current duty cycle
						current_duty_cycle = (100 * (uint32_t)capture_period_pos) / ((uint32_t)capture_period_pos + (uint32_t)capture_period_neg);

						// check if high or low duty cycle fits
						// ignore last bit because of wrong duty cycle
						if (!CheckDutyCycle(current_duty_cycle, PROTOCOL_DATA[used_protocol].BIT_HIGH_DUTY) &&
								!CheckDutyCycle(current_duty_cycle, PROTOCOL_DATA[used_protocol].BIT_LOW_DUTY) &&
								actual_bit != PROTOCOL_DATA[used_protocol].BIT_COUNT)
						{
							RF_DATA_STATUS = 0;
							LED = LED_OFF;
							rf_state = RF_IDLE;
							break;
						}

						if ((CheckDutyCycle(current_duty_cycle, PROTOCOL_DATA[used_protocol].BIT_HIGH_DUTY) &&
								(actual_bit < PROTOCOL_DATA[used_protocol].BIT_COUNT)) ||
								// the duty cycle can not be used for the last bit because of the missing rising edge on the end
								((capture_period_pos > low_pulse_time) && (actual_bit == PROTOCOL_DATA[used_protocol].BIT_COUNT)))
						{
							// backup last bit high time
							BIT_HIGH = capture_period_pos;
							LED = LED_ON;
							RF_DATA[(actual_bit - 1) / 8] |= (1 << actual_bit_of_byte);
						}
						else
						{
							// backup last bit high time
							BIT_LOW = capture_period_pos;
							LED = LED_OFF;
							// backup low bit pulse time to be able to determine the last bit
							if (capture_period_pos > low_pulse_time)
								low_pulse_time = capture_period_pos;
						}

						// 8 bits are done, compute crc of data
						if (actual_bit_of_byte == 0)
						{
							crc = Compute_CRC8_Simple_OneByte(crc ^ RF_DATA[(actual_bit - 1) / 8]);
							actual_bit_of_byte = 8;
						}

						// check if all bits for this protocol got received
						if (actual_bit == PROTOCOL_DATA[used_protocol].BIT_COUNT)
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
								RF_DATA_STATUS = used_protocol;
								RF_DATA_STATUS |= RF_DATA_RECEIVED_MASK;
							}

							LED = LED_OFF;
							rf_state = RF_IDLE;
						}
						break;
				}
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
			// do sniffing by bucket mode
			case MODE_BUCKET:
				Bucket_Received(capture_period_pos);
				break;
		}
	}
}

void PCA0_channel2EventCb()
{

}

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

//-----------------------------------------------------------------------------
// Check for a RF sync
//-----------------------------------------------------------------------------
uint8_t RFInSync(uint8_t identifier, uint16_t period_pos, uint16_t period_neg)
{
	uint8_t ret = 0x80;
	uint8_t used_protocol;

	switch(identifier)
	{
		// protocol is undefined, do loop through all protocols
		case UNKNOWN_IDENTIFIER:

			// check all protocols
			for (used_protocol = 0x00 ; used_protocol < PROTOCOLCOUNT; used_protocol++)
			{
				// check if used_protocol is for PT226x devices
				if (used_protocol == PCA0_GetProtocolIndex(PT2260_IDENTIFIER))
				{
					if ((period_neg > PT226x_SYNC_MIN) && (period_neg < PT226x_SYNC_MAX))
					{
						uint16_t period_pos_snyc = period_neg / 31;
						if ((period_pos > (period_pos_snyc - (period_pos_snyc / 5))) &&
								(period_pos < (period_pos_snyc + (period_pos_snyc / 5))))
						{
							ret = used_protocol;
							break;
						}
					}
				}
				else
				{
					// check if SYNC high and SYNC low should be compared
					if (PROTOCOL_DATA[used_protocol].SYNC_HIGH > 0)
					{
						if (
							(period_pos > (PROTOCOL_DATA[used_protocol].SYNC_HIGH - SYNC_TOLERANCE)) &&
							(period_pos < (PROTOCOL_DATA[used_protocol].SYNC_HIGH + SYNC_TOLERANCE)) &&
							(period_neg > (PROTOCOL_DATA[used_protocol].SYNC_LOW - SYNC_TOLERANCE)) &&
							(period_neg < (PROTOCOL_DATA[used_protocol].SYNC_LOW + SYNC_TOLERANCE))
						)
						{
							ret = used_protocol;
							break;
						}
					}
					// only SYNC low should be checked
					else
					if ((period_neg > (PROTOCOL_DATA[used_protocol].SYNC_LOW - SYNC_TOLERANCE)) &&
						(period_neg < (PROTOCOL_DATA[used_protocol].SYNC_LOW + SYNC_TOLERANCE))
					)
					{
						ret = used_protocol;
						break;
					}
				}
			}
			break;

		// check other protocols
		default:
			used_protocol = PCA0_GetProtocolIndex(identifier);

			// check if identifier got found in list
			if (used_protocol != PCA0_GetProtocolIndex(PT2260_IDENTIFIER))
				break;

			if ((period_neg > PT226x_SYNC_MIN) && (period_neg < PT226x_SYNC_MAX))
			{
				uint16_t period_pos_snyc = period_neg / 31;
				if ((period_pos > (period_pos_snyc - (period_pos_snyc / 5))) &&
						(period_pos < (period_pos_snyc + (period_pos_snyc / 5))))
				{
					ret = used_protocol;
				}
			}
			break;
	}

	return ret;
}

//-----------------------------------------------------------------------------
// check given duty cycle by tolerance
//-----------------------------------------------------------------------------
bool CheckDutyCycle(uint8_t current_duty_cycle, uint8_t desired_duty_cycle)
{
	return ((current_duty_cycle > (desired_duty_cycle - DUTY_CYCLE_TOLERANCE)) &&
			(current_duty_cycle < (desired_duty_cycle + DUTY_CYCLE_TOLERANCE)));
}

//-----------------------------------------------------------------------------
// Send RF SYNC HIGH/LOW Routine
//-----------------------------------------------------------------------------
void SendRF_SYNC(void)
{
	LED = LED_ON;
	// enable P0.0 for I/O control
	XBR1 &= ~XBR1_PCA0ME__CEX0_CEX1;
	// switch to high
	T_DATA = 1;
	// do high time
	// start timer
	InitTimer3_us(10, SYNC_HIGH);
	// wait until timer has finished
	WaitTimer3Finished();
	LED = LED_OFF;
	// switch to low
	T_DATA = 0;

	// do low time
	// start timer
	InitTimer3_us(10, SYNC_LOW);
	// wait until timer has finished
	WaitTimer3Finished();
	// disable P0.0 for I/O control, enter PCA mode
	XBR1 |= XBR1_PCA0ME__CEX0_CEX1;
}

uint8_t PCA0_GetProtocolIndex(uint8_t identifier)
{
	uint8_t i;
	uint8_t protocol_index = 0xFF;

	// check first for valid identifier
	if ((identifier > 0x00) && (identifier < 0x80))
	{
		// find protocol index by identifier
		for(i = 0; i < PROTOCOLCOUNT; i++)
		{
			if (PROTOCOL_DATA[i].IDENTIFIER == identifier)
			{
				protocol_index = i;
				break;
			}
		}
	}

	return protocol_index;
}

void PCA0_InitTransmit(uint16_t sync_high, uint16_t sync_low, uint16_t BIT_HIGH_TIME, uint8_t BIT_HIGH_DUTY,
		uint16_t BIT_LOW_TIME, uint8_t BIT_LOW_DUTY, uint8_t bitcount)
{
	uint16_t bit_time;

	bit_time = (100 * (uint32_t)BIT_HIGH_TIME) / BIT_HIGH_DUTY;
	// calculate T0_Overflow
	T0_HIGH = (uint8_t)(0x100 - ((uint32_t)SYSCLK / (0xFF * (1000000 / (uint32_t)bit_time))));

	bit_time = (100 * (uint32_t)BIT_LOW_TIME) / BIT_LOW_DUTY;
	// calculate T0_Overflow
	T0_LOW = (uint8_t)(0x100 - ((uint32_t)SYSCLK / (0xFF * (1000000 / (uint32_t)bit_time))));

	// calculate high and low duty cycle
	DUTY_CYCLE_HIGH = (uint16_t)((BIT_HIGH_DUTY * 0xFF) / 100);
	DUTY_CYLCE_LOW = (uint16_t)((BIT_LOW_DUTY * 0xFF) / 100);

	// set constants
	SYNC_HIGH = sync_high;
	SYNC_LOW = sync_low;
	BIT_COUNT = bitcount;

	// enable interrupt for RF transmitting
	PCA0CPM0 |= PCA0CPM0_ECCF__ENABLED;
	// clear Cycle Overflow Flag
	PCA0PWM &= ~PCA0PWM_COVF__OVERFLOW;
	PCA0PWM |= PCA0PWM_ECOV__COVF_MASK_ENABLED;

	// disable interrupt for RF receiving
	PCA0CPM1 &= ~PCA0CPM1_ECCF__ENABLED;

	/***********************************************************************
	 - PCA Counter/Timer Low Byte = 0xFF
	 ***********************************************************************/
	PCA0L = (0xFF << PCA0L_PCA0L__SHIFT);
}

void SetPCA0DutyCylce(void)
{
	if(((RF_DATA[actual_byte] >> actual_bit_of_byte) & 0x01) == 0x01)
	{
		// bit 1
		PCA0_writeChannel(PCA0_CHAN0, DUTY_CYCLE_HIGH << 8);
	}
	else
	{
		// bit 0
		PCA0_writeChannel(PCA0_CHAN0, DUTY_CYLCE_LOW << 8);
	}
}

void SetTimer0Overflow(uint8_t T0_Overflow)
{
	/***********************************************************************
	 - Timer 0 High Byte = T0_Overflow
	 ***********************************************************************/
	TH0 = (T0_Overflow << TH0_TH0__SHIFT);
}

void PCA0_StartTransmit(void)
{
	actual_bit_of_byte = 0x08;
	actual_bit_of_byte--;
	actual_bit = 1;

	rf_state = RF_TRANSMITTING;

	// set first bit to be in sync when PCA0 is starting
	SetPCA0DutyCylce();

	// make RF sync pulse
	SendRF_SYNC();

	PCA0_run();
}

void PCA0_StopTransmit(void)
{
	// set duty cycle to zero
	PCA0_writeChannel(PCA0_CHAN0, 0x0000);
	// disable interrupt for RF transmitting
	PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;
	// clear Cycle Overflow Flag
	PCA0PWM &= ~PCA0PWM_COVF__OVERFLOW;
	PCA0PWM &= ~PCA0PWM_ECOV__COVF_MASK_ENABLED;
	// stop PCA
	PCA0_halt();

	// clear all interrupt flags of PCA0
	PCA0CN0 &= ~(PCA0CN0_CF__BMASK
	  		                       | PCA0CN0_CCF0__BMASK
	  		                       | PCA0CN0_CCF1__BMASK
	  		                       | PCA0CN0_CCF2__BMASK);

	// enable P0.0 for I/O control
	XBR1 &= ~XBR1_PCA0ME__CEX0_CEX1;
	// switch to low
	T_DATA = 0;
	LED = LED_OFF;
	// disable P0.0 for I/O control, enter PCA mode
	//XBR1 |= XBR1_PCA0ME__CEX0_CEX1;
	// let it disabled to avoid a spike on the end

	rf_state = RF_FINISHED;
}

uint8_t PCA0_DoSniffing(uint8_t active_command)
{
	uint8_t ret = last_sniffing_command;

	// restore timer to 100000Hz, 10µs interval
	SetTimer0Overflow(0x0B);

	// enable interrupt for RF receiving
	PCA0CPM1 |= PCA0CPM1_ECCF__ENABLED;
	// disable interrupt for RF transmitting
	PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;
	// clear Cycle Overflow Flag
	PCA0PWM &= ~PCA0PWM_COVF__OVERFLOW;
	PCA0PWM &= ~PCA0PWM_ECOV__COVF_MASK_ENABLED;

	// start PCA
	PCA0_run();

	InitTimer3_ms(1, 10);
	// wait until timer has finished
	WaitTimer3Finished();

	// disable P0.0 for I/O control, enter PCA mode
	XBR1 |= XBR1_PCA0ME__CEX0_CEX1;

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
	PCA0CPM1 &= ~PCA0CPM1_ECCF__ENABLED;

	// be sure the timeout timer is stopped
	StopTimer2();

	rf_state = RF_IDLE;
}

//-----------------------------------------------------------------------------
// Send generic signal based on n time bucket pairs (high/low timing)
//-----------------------------------------------------------------------------
void SendRFBuckets(const uint16_t buckets[], const uint8_t rfdata[], uint8_t n, uint8_t repeats)
{
	// disable interrupts for RF receiving and transmitting
	PCA0CPM1 &= ~PCA0CPM1_ECCF__ENABLED;
	PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;
	// clear Cycle Overflow Flag
	PCA0PWM &= ~PCA0PWM_COVF__OVERFLOW;
	PCA0PWM &= ~PCA0PWM_ECOV__COVF_MASK_ENABLED;

	XBR1 &= ~XBR1_PCA0ME__CEX0_CEX1;	// enable P0.0 for I/O control

	do
	{
		uint8_t i;

		for (i = 0; i < n; i++)			// transmit n bucket pairs
		{
			uint16_t j = buckets[rfdata[i] >> 4];	// high bucket
			T_DATA = 1;					// switch to high
			InitTimer3_us(10, j);
			// wait until timer has finished
			WaitTimer3Finished();

			j = buckets[rfdata[i] & 0x0f];			// low bucket
			T_DATA = 0;					// switch to low
			InitTimer3_us(10, j);
			// wait until timer has finished
			WaitTimer3Finished();
		}
		LED = !LED;
	}
	while (repeats-- != 0);				// how many times do I need to repeat?

	// disable P0.0 for I/O control, enter PCA mode
	XBR1 |= XBR1_PCA0ME__CEX0_CEX1;
	LED = LED_OFF;
}


bool probablyFooter(uint16_t duration)
{
	return duration >= MIN_FOOTER_LENGTH;
}

bool matchesFooter(uint16_t duration)
{
  uint16_t footer_delta = bucket_sync / 4;
  return (((bucket_sync - footer_delta) < duration) && (duration < (bucket_sync + footer_delta)));
}

bool findBucket(uint16_t duration, uint8_t *index)
{
	bool ret = false;
	uint8_t i;
	for (i = 0; i < bucket_count; i++)
	{
		uint16_t delta = duration / 4 + duration / 8;
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
				bucket_count = 0;
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
				rf_state = RF_DECODE_BUCKET;
			}
			// check if useful bucket got received
			else if (duration >= MIN_PULSE_LENGTH)
			{
				// check if bucket was already received
				if (findBucket(duration, &bucket_index))
				{
					// make new average of this bucket
					buckets[bucket_index] = (buckets[bucket_index] + duration) / 2;
				}
				else
				{
					// new bucket received, add to array
					buckets[bucket_count] = duration;
					bucket_count++;

					// check if maximum of array got reached
					if (bucket_count > sizeof(buckets))
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
			// check if sync bucket got received
			if (matchesFooter(duration))
			{
				RF_DATA_STATUS |= RF_DATA_RECEIVED_MASK;
				LED = LED_OFF;
				rf_state = RF_IDLE;
			}
			// check if bucket can be decoded
			else if (findBucket(duration, &bucket_index))
			{
				if (actual_bit_of_byte == 4)
				{
					RF_DATA[actual_byte] = bucket_index << 4;
					actual_bit_of_byte = 0;
				}
				else
				{
					RF_DATA[actual_byte] |= (bucket_index & 0x0F);
					actual_byte++;
					actual_bit_of_byte = 4;

					// check if maximum of array got reached
					if (actual_byte > sizeof(RF_DATA))
					{
						bucket_count = 0;
						// restart sync
						rf_state = RF_IDLE;
					}
				}
			}
			else
			{
				// bucket not found in list, restart
				bucket_count = 0;
				// restart sync
				rf_state = RF_IDLE;
			}
			break;
	}
}
