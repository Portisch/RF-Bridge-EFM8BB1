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

SI_SEGMENT_VARIABLE(Timer_3_Timeout, uint16_t, SI_SEG_XDATA) = 0x0000;
SI_SEGMENT_VARIABLE(sniffing_is_on, uint8_t, SI_SEG_XDATA) = false;

SI_SEGMENT_VARIABLE(DUTY_CYCLE_HIGH, uint8_t, SI_SEG_XDATA) = 0x56;
SI_SEGMENT_VARIABLE(DUTY_CYLCE_LOW, uint8_t, SI_SEG_XDATA) = 0xAB;

SI_SEGMENT_VARIABLE(actual_bit_of_byte, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(actual_bit, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(actual_byte, uint8_t, SI_SEG_XDATA) = 0;
SI_SEGMENT_VARIABLE(protocol_index, uint8_t, SI_SEG_XDATA) = 0;

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------
void PCA0_overflowCb()
{

}

void PCA0_intermediateOverflowCb()
{

}

void PCA0_channel0EventCb()
{
	// stop transfer if all bits are transmitted
	if (actual_bit_of_byte == 0)
	{
		actual_byte++;
		actual_bit_of_byte = 8;
	}

	if (actual_bit == PROTOCOL_DATA[protocol_index].BIT_COUNT)
	{
		PCA0_StopTransmit();
		return;
	}

	actual_bit++;
	actual_bit_of_byte--;

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

void PCA0_channel1EventCb()
{
	static uint16_t current_capture_value;
	static uint16_t previous_capture_value_pos, previous_capture_value_neg;
	static uint16_t capture_period_pos, capture_period_neg;

	static uint8_t used_protocol;
	static uint16_t low_pulse_time;
	uint8_t in_sync = false;

	// Store most recent capture value
	current_capture_value = PCA0CP1 * 10;

	// positive edge
	if (R_DATA)
	{
		// Update previous capture value with most recent info.
		previous_capture_value_pos = current_capture_value;

		// Calculate capture period from last two values.
		capture_period_neg = current_capture_value - previous_capture_value_neg;

		switch (rf_state)
		{
			// check if we receive a sync
			case RF_IDLE:
				// check first if last decoded RF signal was cleared
				if (RF_DATA_STATUS != 0)
					break;

				// check all protocols
				for ( used_protocol = 0; used_protocol < PROTOCOLCOUNT; used_protocol++)
				{
					// check if SYNC high and SYNC low should be compared
					if (PROTOCOL_DATA[used_protocol].SYNC_HIGH > 0)
					{
						if (
							(capture_period_pos > (PROTOCOL_DATA[used_protocol].SYNC_HIGH - SYNC_TOLERANCE)) &&
							(capture_period_pos < (PROTOCOL_DATA[used_protocol].SYNC_HIGH + SYNC_TOLERANCE)) &&
							(capture_period_neg > (PROTOCOL_DATA[used_protocol].SYNC_LOW - SYNC_TOLERANCE)) &&
							(capture_period_neg < (PROTOCOL_DATA[used_protocol].SYNC_LOW + SYNC_TOLERANCE))
						)
						{
							in_sync = true;
						}
					}
					// only SYNC low should be checked
					else
					{
						if (
							(capture_period_neg > (PROTOCOL_DATA[used_protocol].SYNC_LOW - SYNC_TOLERANCE)) &&
							(capture_period_neg < (PROTOCOL_DATA[used_protocol].SYNC_LOW + SYNC_TOLERANCE))
						)
						{
							in_sync = true;
						}
					}

					// check if a matching protocol got found
					if (in_sync)
					{
						actual_bit_of_byte = 8;
						actual_byte = 0;
						actual_bit = 0;
						low_pulse_time = 0;
						memset(RF_DATA, 0, sizeof(RF_DATA));
						rf_state = RF_IN_SYNC;
						break;
					}
				}
				break;

			// one matching sync got received
			case RF_IN_SYNC:
				actual_bit_of_byte--;
				actual_bit++;

				// if high time is longer than low time: logic 1
				// if high time is shorter than low time: logic 0
				// the high time of bit 0 is getting measured to be able to determine the last bit
				if (
					((capture_period_pos > capture_period_neg) && (actual_bit < PROTOCOL_DATA[used_protocol].BIT_COUNT)) ||
					((capture_period_pos > low_pulse_time) && (actual_bit == PROTOCOL_DATA[used_protocol].BIT_COUNT))
					)
				{
					LED = LED_ON;
					RF_DATA[(actual_bit - 1) / 8] |= (1 << actual_bit_of_byte);
				}
				else
				{
					LED = LED_OFF;
					// backup low bit pulse time to be able to determine the last bit
					if (capture_period_pos > low_pulse_time)
						low_pulse_time = capture_period_pos;
				}

				if (actual_bit_of_byte == 0)
					actual_bit_of_byte = 8;

				// check if all bits for this protocol got received
				if (actual_bit == PROTOCOL_DATA[used_protocol].BIT_COUNT)
				{
					RF_DATA_STATUS = used_protocol;
					RF_DATA_STATUS |= RF_DATA_RECEIVED_MASK;
					LED = LED_OFF;
					rf_state = RF_IDLE;
				}
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
	}
}

void PCA0_channel2EventCb()
{

}

//-----------------------------------------------------------------------------
// Send RF SYNC HIGH/LOW Routine
//-----------------------------------------------------------------------------
void SendRF_SYNC(uint8_t used_protocol)
{
	// enable P0.0 for I/O control
	XBR1 &= ~XBR1_PCA0ME__CEX0_CEX1;
	// do activate the SYN115 chip
	Timer_3_Timeout = 3000;
	// switch to high
	T_DATA = 1;
	// start 5탎 timer
	TMR3CN0 |= TMR3CN0_TR3__RUN;
	// wait until timer has finished
	while((TMR3CN0 & TMR3CN0_TR3__BMASK) == TMR3CN0_TR3__RUN);
	// switch to low
	T_DATA = 0;

	Timer_3_Timeout = 100;
	// start 5탎 timer
	TMR3CN0 |= TMR3CN0_TR3__RUN;
	// wait until timer has finished
	while((TMR3CN0 & TMR3CN0_TR3__BMASK) == TMR3CN0_TR3__RUN);
	// switch to high
	T_DATA = 1;
	// do high time
	Timer_3_Timeout = PROTOCOL_DATA[used_protocol].SYNC_HIGH;
	// start 5탎 timer
	TMR3CN0 |= TMR3CN0_TR3__RUN;
	// wait until timer has finished
	while((TMR3CN0 & TMR3CN0_TR3__BMASK) == TMR3CN0_TR3__RUN);
	// switch to low
	T_DATA = 0;

	// do low time
	Timer_3_Timeout = PROTOCOL_DATA[used_protocol].SYNC_LOW;
	// start 5탎 timer
	TMR3CN0 |= TMR3CN0_TR3__RUN;
	// wait until timer has finished
	while((TMR3CN0 & TMR3CN0_TR3__BMASK) == TMR3CN0_TR3__RUN);
	// disable P0.0 for I/O control, enter PCA mode
	XBR1 |= XBR1_PCA0ME__CEX0_CEX1;
}

uint8_t PCA0_DoTransmit(uint8_t identifier)
{
	uint8_t i;
	uint8_t protocol_index = 0xFF;
	uint8_t TCON_save;

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

		// check if protocol got found
		if (protocol_index != 0xFF)
		{
			// calculate T0_Overflow
			i = (uint8_t)(0x100 - ((uint32_t)SYSCLK / (0xFF * (1000000 / (uint32_t)PROTOCOL_DATA[protocol_index].BIT_TIME))));

			//Save Timer Configuration
			TCON_save = TCON;
			//Stop Timer 0
			TCON &= ~TCON_TR0__BMASK;

			/***********************************************************************
			 - Timer 0 High Byte = i (T0_Overflow)
			 ***********************************************************************/
			TH0 = (i << TH0_TH0__SHIFT);

			//Restore Timer Configuration
			TCON |= (TCON_save & TCON_TR0__BMASK);

			// calculate high and low duty cycle
			DUTY_CYCLE_HIGH = (uint16_t)(0xFF - ((PROTOCOL_DATA[protocol_index].BIT_HIGH_DUTY * 0xFF) / 100));
			DUTY_CYLCE_LOW = (uint16_t)(0xFF - ((PROTOCOL_DATA[protocol_index].BIT_LOW_DUTY * 0xFF) / 100));

			// enable interrupt for RF transmitting
			PCA0CPM0 |= PCA0CPM0_ECCF__ENABLED;

			// disable interrupt for RF receiving
			PCA0CPM1 &= ~PCA0CPM1_ECCF__ENABLED;

			/***********************************************************************
			 - PCA Counter/Timer Low Byte = 0xFF
			 ***********************************************************************/
			PCA0L = (0xFF << PCA0L_PCA0L__SHIFT);
		}
	}

	return protocol_index;
}

void PCA0_StopTransmit(void)
{
	// set duty cycle to zero
	PCA0_writeChannel(PCA0_CHAN0, 0x0000);
	// disable interrupt for RF transmitting
	PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;
	PCA0_halt();

	// enable P0.0 for I/O control
	XBR1 &= ~XBR1_PCA0ME__CEX0_CEX1;
	// switch to low
	T_DATA = 0;
	// disable P0.0 for I/O control, enter PCA mode
	XBR1 |= XBR1_PCA0ME__CEX0_CEX1;

	// restart sniffing it was active
	if(sniffing_is_on)
		PCA0_DoSniffing();
}

void PCA0_DoSniffing(void)
{
	// restore timer to 100000Hz, 10탎 interval
	//Save Timer Configuration
	uint8_t TCON_save;
	TCON_save = TCON;
	//Stop Timer 0
	TCON &= ~TCON_TR0__BMASK;

	/***********************************************************************
	 - Timer 0 High Byte = 0x0B
	 ***********************************************************************/
	TH0 = (0x0B << TH0_TH0__SHIFT);

	//Restore Timer Configuration
	TCON |= (TCON_save & TCON_TR0__BMASK);

	// stop PCA
	PCA0CN0_CR = PCA0CN0_CR__STOP;

	// enable interrupt for RF receiving
	PCA0CPM1 |= PCA0CPM1_ECCF__ENABLED;

	// disable interrupt for RF transmitting
	PCA0CPM0 &= ~PCA0CPM0_ECCF__ENABLED;

	// start PCA
	PCA0CN0_CR = PCA0CN0_CR__RUN;

	rf_state = RF_IDLE;
	RF_DATA_STATUS = 0;
	sniffing_is_on = true;
}

void PCA0_StopSniffing(void)
{
	// stop PCA
	PCA0CN0_CR = PCA0CN0_CR__STOP;

	// disable interrupt for RF receiving
	PCA0CPM1 &= ~PCA0CPM1_ECCF__ENABLED;
}

//-----------------------------------------------------------------------------
// TIMER3_ISR
//-----------------------------------------------------------------------------
//
// TIMER3 ISR Content goes here. Remember to clear flag bits:
// TMR3CN0::TF3H (Timer # High Byte Overflow Flag)
// TMR3CN0::TF3L (Timer # Low Byte Overflow Flag)
//
//-----------------------------------------------------------------------------
SI_INTERRUPT (TIMER3_ISR, TIMER3_IRQn)
{
	// Clear Timer 3 high overflow flag
	TMR3CN0 &= ~TMR3CN0_TF3H__SET;

	// check if pulse time is over
	if(Timer_3_Timeout <= 0)
	{
		// stop timer
		TMR3CN0 &= ~TMR3CN0_TR3__RUN;
	}

	Timer_3_Timeout -= 5;
}
