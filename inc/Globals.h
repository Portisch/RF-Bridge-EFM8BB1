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

#define Sniffing_On	1

#define SYSCLK	24500000

#define FIRMWARE_VERSION		0x03

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

#define START_GET(x) (((x).status >> 12) & 0x0F)
#define START_INC(x) ((x).status = ((((x).status >> 12) + 1) << 12) | ((x).status & 0x0FFF))
#define START_CLEAR(x) ((x).status = 0, (x).bit_count = 0)

#define BIT0_GET(x) (((x).status >> 8) & 0x0F)
#define BIT0_INC(x) ((x).status = ((((x).status >> 8) + 1) << 8) | ((x).status & 0xF0FF))
#define BIT0_CLEAR(x) ((x).status &= 0xF0FF)

#define BIT1_GET(x) (((x).status >> 4) & 0x0F)
#define BIT1_INC(x) ((x).status = ((((x).status >> 4) + 1) << 4) | ((x).status & 0xFF0F))
#define BIT1_CLEAR(x) ((x).status &= 0xFF0F)

#define END_GET(x) ((x).status & 0x0F)
#define END_INC(x) ((x).status = ((x).status + 1) | ((x).status & 0xFFF0))
#define END_CLEAR(x) ((x).status &= 0xFFF0)

#define BITS_GET(x) ((x).bit_count)
#define BITS_INC(x) ((x).bit_count += 1)

#define PROTOCOL_PULSES(X) X ## _pulses
#define PROTOCOL_START(X) X ## _start
#define PROTOCOL_BIT0(X) X ## _bit0
#define PROTOCOL_BIT1(X) X ## _bit1

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
