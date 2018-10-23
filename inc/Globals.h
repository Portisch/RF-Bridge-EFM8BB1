/*
 * Globals.h
 *
 *  Created on: 27.11.2017
 *      Author:
 */

#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

// USER CONSTANTS
#define LED_ON	1
#define LED_OFF	0

#define BUZZER_ON	1
#define BUZZER_OFF	0

#define TDATA_ON	1
#define TDATA_OFF	0

#define Sniffing	true

#define SYSCLK	24500000

#define FIRMWARE_VERSION		0x01

// USER PROTOTYPES
SI_SBIT(LED, SFR_P1, 0);		// LED
SI_SBIT(T_DATA, SFR_P0, 0);		// T_DATA
SI_SBIT(R_DATA, SFR_P1, 3);		// R_DATA
SI_SBIT(BUZZER, SFR_P1, 6);		// BUZZER

extern void InitTimer2_us(uint16_t interval, uint16_t timeout);
extern void InitTimer3_us(uint16_t interval, uint16_t timeout);
extern void InitTimer2_ms(uint16_t interval, uint16_t timeout);
extern void InitTimer3_ms(uint16_t interval, uint16_t timeout);
extern void WaitTimer2Finished(void);
extern void WaitTimer3Finished(void);
extern void StopTimer2(void);
extern void StopTimer3(void);
extern bool IsTimer2Finished(void);
extern bool IsTimer3Finished(void);



#endif /* INC_GLOBALS_H_ */
