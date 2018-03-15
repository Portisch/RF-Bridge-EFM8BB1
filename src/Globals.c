/*
 * Globals.c
 *
 *  Created on: 07.12.2017
 *      Author:
 */
#include <SI_EFM8BB1_Register_Enums.h>
#include "Globals.h"

SI_SEGMENT_VARIABLE(Timer_2_Timeout, uint16_t, SI_SEG_XDATA) = 0x0000;
SI_SEGMENT_VARIABLE(Timer_2_Interval, uint16_t, SI_SEG_XDATA) = 0x0000;
SI_SEGMENT_VARIABLE(Timer_3_Timeout, uint16_t, SI_SEG_XDATA) = 0x0000;
SI_SEGMENT_VARIABLE(Timer_3_Interval, uint16_t, SI_SEG_XDATA) = 0x0000;

void SetTimer2Reload(uint16_t reload)
{
	/***********************************************************************
	 - Timer 2 Reload High Byte
	 ***********************************************************************/
	TMR2RLH = (((reload >> 8) & 0xFF) << TMR2RLH_TMR2RLH__SHIFT);
	/***********************************************************************
	 - Timer 2 Reload Low Byte = 0x86
	 ***********************************************************************/
	TMR2RLL = ((reload & 0xFF) << TMR2RLL_TMR2RLL__SHIFT);
}

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
 * Init Timer 2 with microseconds interval, maximum is 65535탎.
 */
void InitTimer2_us(uint16_t interval, uint16_t timeout)
{
	SetTimer2Reload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000000 / (uint32_t)interval))));

	// remove 65탎 because of startup delay
	Timer_2_Timeout = timeout - 65;
	Timer_2_Interval = interval;

	// start timer
	TMR2CN0 |= TMR2CN0_TR2__RUN;
}

/*
 * Init Timer 3 with microseconds interval, maximum is 65535탎.
 */
void InitTimer3_us(uint16_t interval, uint16_t timeout)
{
	SetTimerReload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000000 / (uint32_t)interval))));

	// remove 65탎 because of startup delay
	Timer_3_Timeout = timeout - 65;
	Timer_3_Interval = interval;

	// start timer
	TMR3CN0 |= TMR3CN0_TR3__RUN;
}

/*
 * Init Timer 2 with milliseconds interval, maximum is ~2.5ms.
 */
void InitTimer2_ms(uint16_t interval, uint16_t timeout)
{
	SetTimer2Reload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000 / (uint32_t)interval))));

	Timer_2_Timeout = timeout;
	Timer_2_Interval = interval;

	// start timer
	TMR2CN0 |= TMR2CN0_TR2__RUN;
}

/*
 * Init Timer 3 with milliseconds interval, maximum is ~2.5ms.
 */
void InitTimer3_ms(uint16_t interval, uint16_t timeout)
{
	SetTimerReload((uint16_t)(0x10000 - ((uint32_t)SYSCLK / (1000 / (uint32_t)interval))));

	Timer_3_Timeout = timeout;
	Timer_3_Interval = interval;

	// start timer
	TMR3CN0 |= TMR3CN0_TR3__RUN;
}

void WaitTimer2Finished(void)
{
	// wait until timer has finished
	while((TMR2CN0 & TMR2CN0_TR2__BMASK) == TMR2CN0_TR2__RUN);
}

void WaitTimer3Finished(void)
{
	// wait until timer has finished
	while((TMR3CN0 & TMR3CN0_TR3__BMASK) == TMR3CN0_TR3__RUN);
}

void StopTimer2(void)
{
	// stop timer
	TMR2CN0 &= ~TMR2CN0_TR2__RUN;
	// Clear Timer 2 high overflow flag
	TMR2CN0 &= ~TMR2CN0_TF2H__SET;
}

void StopTimer3(void)
{
	// stop timer
	TMR3CN0 &= ~TMR3CN0_TR3__RUN;
	// Clear Timer 3 high overflow flag
	TMR3CN0 &= ~TMR3CN0_TF3H__SET;
}

bool IsTimer2Finished(void)
{
	return ((TMR2CN0 & TMR2CN0_TR2__BMASK) != TMR2CN0_TR2__RUN);
}

bool IsTimer3Finished(void)
{
	return ((TMR3CN0 & TMR3CN0_TR3__BMASK) != TMR3CN0_TR3__RUN);
}

//-----------------------------------------------------------------------------
// TIMER2_ISR
//-----------------------------------------------------------------------------
//
// TIMER2 ISR Content goes here. Remember to clear flag bits:
// TMR2CN0::TF2H (Timer # High Byte Overflow Flag)
// TMR2CN0::TF2L (Timer # Low Byte Overflow Flag)
//
//-----------------------------------------------------------------------------
SI_INTERRUPT (TIMER2_ISR, TIMER2_IRQn)
{
	// Clear Timer 2 high overflow flag
	TMR2CN0 &= ~TMR2CN0_TF2H__SET;

	// check if pulse time is over
	if(Timer_2_Timeout == 0)
	{
		// stop timer
		TMR2CN0 &= ~TMR2CN0_TR2__RUN;
	}

	if (Timer_2_Timeout < Timer_2_Interval)
		Timer_2_Timeout = 0;
	else
		Timer_2_Timeout -= Timer_2_Interval;
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
	if(Timer_3_Timeout == 0)
	{
		// stop timer
		TMR3CN0 &= ~TMR3CN0_TR3__RUN;
	}

	if (Timer_3_Timeout < Timer_3_Interval)
		Timer_3_Timeout = 0;
	else
		Timer_3_Timeout -= Timer_3_Interval;
}
