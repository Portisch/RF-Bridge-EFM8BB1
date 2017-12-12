/*
 * Globals.c
 *
 *  Created on: 07.12.2017
 *      Author:
 */
#include <SI_EFM8BB1_Register_Enums.h>
#include "Globals.h"

SI_SEGMENT_VARIABLE(Timer_3_Timeout, uint16_t, SI_SEG_XDATA) = 0x0000;
SI_SEGMENT_VARIABLE(Timer_3_Interval, uint16_t, SI_SEG_XDATA) = 0x0000;

void SetTimerReload(uint16_t reload)
{
	/***********************************************************************
	 - Timer 3 Reload High Byte
	 ***********************************************************************/
	TMR3RLH = (((reload >> 8) & 0xFF) << TMR3RLH_TMR3RLH__SHIFT);
	/***********************************************************************
	 - Timer 3 Reload Low Byte = 0x86
	 ***********************************************************************/
	TMR3RLL = ((reload & 0xFF) << TMR3RLL_TMR3RLL__SHIFT);
}

/*
 * Init Timer with microseconds interval, maximum is 65535µs.
 */
void InitTimer_us(uint16_t interval, uint16_t timeout)
{
	SetTimerReload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000000 / (uint32_t)interval))));

	Timer_3_Timeout = timeout;
	Timer_3_Interval = interval;

	// start timer
	TMR3CN0 |= TMR3CN0_TR3__RUN;
}
/*
 * Init Timer 3 with milliseconds interval, maximum is ~2.5ms.
 */
void InitTimer_ms(uint16_t interval, uint16_t timeout)
{
	SetTimerReload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000 / (uint32_t)interval))));

	Timer_3_Timeout = timeout;
	Timer_3_Interval = interval;

	// start timer
	TMR3CN0 |= TMR3CN0_TR3__RUN;
}

void WaitTimerFinished(void)
{
	// wait until timer has finished
	while((TMR3CN0 & TMR3CN0_TR3__BMASK) == TMR3CN0_TR3__RUN);
}

bool IsTimerFinished(void)
{
	return ((TMR3CN0 & TMR3CN0_TR3__BMASK) != TMR3CN0_TR3__RUN);
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

	Timer_3_Timeout -= Timer_3_Interval;
}
