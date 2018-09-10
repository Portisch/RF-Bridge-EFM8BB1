/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "wdt_0.h"
#include "assert.h"

void WDT0_start(){
  WDTCN = 0xA5;
}

void WDT0_stop()
{
  bool ea = IE_EA;
  IE_EA = 0;
  WDTCN = 0xDE;
  WDTCN = 0xAD;
  IE_EA = ea;
}

void WDT0_feed()
{
  WDTCN = 0xA5;
}

void WDT0_init(uint8_t interval,
               WDT0_Timebase_t  timebase,
               WDT0_IdleState_t idleState)
{
  timebase=timebase;
  idleState=idleState;
  WDTCN = interval;
}
