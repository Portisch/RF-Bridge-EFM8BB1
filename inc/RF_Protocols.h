/*
 * RF_Protocols.h
 *
 *  Created on: 28.11.2017
 *      Author:
 */

#ifndef INC_RF_PROTOCOLS_H_
#define INC_RF_PROTOCOLS_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdint.h>

/*
 * bucket sniffing constants
 */
#define MIN_FOOTER_LENGTH	1300
#define MIN_PULSE_LENGTH	100

/*
 * sync constants
 */
#define SYNC_TOLERANCE 			10
#define SYNC_TOLERANCE_MAX		500

/*
 * number of repeating by default
 */
#define RF_TRANSMIT_REPEATS		8

/*
 * undefined protocol index
 */
#define UNDEFINED_INDEX			0x80

/*
 * Default Sonoff RF Bridge protocol
 */
#define PT2260_INDEX			0x00

/*
 * sniffing type
 */
typedef enum
{
	TIMING,
	BUCKET
} protocol_type_t;

/*
 * high low factor
 */
typedef struct
{
	uint8_t HIGH;
	uint8_t LOW;
} HIGH_LOW;

/*
 * typedef for timing protocols
 */
typedef struct TIMING_PROTOCOL_DATA
{
	// high and low factor for the sync pulse
	HIGH_LOW SYNC;
	// sync bit count
	uint8_t SYNC_BIT_COUNT;
	// pulse time
	uint16_t PULSE_TIME;
	// high and low factor for bit 0
	HIGH_LOW BIT0;
	// high and low factor for bit 1
	HIGH_LOW BIT1;
	// bit count for this protocol
	uint8_t BIT_COUNT;
	// decode tolerance in % of the PULSE_TIME
	uint8_t TOLERANCE;
	// delay in microseconds between the repeats
	uint8_t REPEAT_DELAY;
	// protocol with inverse pulses
	uint8_t INVERSE;
} TIMING_PROTOCOL_DATA;

/*
 * Protocol array
 * use a value of "0" for SYNC.high, SYNC.low or PULSE_TIME to deactivate some checking while decoding
 */
#define PROTOCOLCOUNT	8
#if PROTOCOLCOUNT > 0x7F
#error Too much protocols are defined, stop!
#endif

SI_SEGMENT_VARIABLE(PROTOCOL_DATA[PROTOCOLCOUNT], static const struct TIMING_PROTOCOL_DATA, SI_SEG_CODE) =
{
		/*
		 * PT2260, EV1527,... original RF bridge protocol
		 * http://www.princeton.com.tw/Portals/0/Product/PT2260_4.pdf
		 */
		{
			//0,		// SYNC_HIGH
			//0,		// SYNC_LOW
			{  1, 31 },	// SYNC HIGH_LOW
			0,			// SYNC_BIT_COUNT
			350,		// PULSE_TIME
			{  1,  3 },	// BIT0 HIGH_LOW
			{  3,  1 },	// BIT1 HIGH_LOW
			24,			// BIT_COUNT
			60,			// TOLERANCE
			0,			// REPEAT_DELAY
			false		// INVERSE
		},

		/*
		 * Rohrmotor24
		 * https://github.com/bjwelker/Raspi-Rollo/tree/master/Arduino/Rollo_Code_Receiver
		 */
		{
			//4800,		// SYNC_HIGH
			//1500,		// SYNC_LOW
			{ 13,  4 },	// SYNC HIGH_LOW
			0,			// SYNC_BIT_COUNT
			370,		// PULSE_TIME
			{  1,  2 },	// BIT0 HIGH_LOW
			{  2,  1 },	// BIT1 HIGH_LOW
			40,			// BIT_COUNT
			60,			// TOLERANCE
			8,			// REPEAT_DELAY
			false		// INVERSE
		},

		/*
		 * UNDERWATER PAR56 LED LAMP, 502266
		 * http://www.seamaid-lighting.com/de/produit/lampe-par56/
		 */
		{
			//3000,		// SYNC_HIGH
			//9000,		// SYNC_LOW
			{  8, 23 },	// SYNC HIGH_LOW
			0,			// SYNC_BIT_COUNT
			390,		// PULSE_TIME
			{  1,  3 },	// BIT0 HIGH_LOW
			{  3,  1 },	// BIT1 HIGH_LOW
			24,			// BIT_COUNT
			60,			// TOLERANCE
			0,			// REPEAT_DELAY
			false		// INVERSE
		},

		/*
		 * Wall plug Noru
		  */
		{
			//9500,		// SYNC_HIGH
			//3000,		// SYNC_LOW
			{ 31, 10 },	// SYNC HIGH_LOW
			0,			// SYNC_BIT_COUNT
			300,		// PULSE_TIME
			{  1,  3 },	// BIT0 HIGH_LOW
			{  3,  1 },	// BIT1 HIGH_LOW
			24,			// BIT_COUNT
			60,			// TOLERANCE
			0,			// REPEAT_DELAY
			false		// INVERSE
		},

		/*
		 * Alecto WS-1200 Series Wireless Weather Station
		  */
		{
			//0,		// SYNC_HIGH
			//29400		// SYNC_LOW
			{  0, 59 },	// SYNC HIGH_LOW
			7,			// SYNC_BIT_COUNT
			500,		// PULSE_TIME
			{  3,   2 },// BIT0 HIGH_LOW
			{  1,   2 },// BIT1 HIGH_LOW
			64,			// BIT_COUNT
			60,			// TOLERANCE
			0,			// REPEAT_DELAY
			false		// INVERSE
		},

		/*
		 * ALDI Remote controlled wall sockets, 4x
		  */
		{
			//3000,		// SYNC_HIGH
			//7250,		// SYNC_LOW
			{  7, 18 },	// SYNC HIGH_LOW
			0,			// SYNC_BIT_COUNT
			400,		// PULSE_TIME
			{  1,  3 },	// BIT0 HIGH_LOW
			{  3,  1 },	// BIT1 HIGH_LOW
			24,			// BIT_COUNT
			60,			// TOLERANCE
			0,			// REPEAT_DELAY
			false		// INVERSE
		},

		/*
		 * HT6P20X chips
		 * http://www.holtek.com.tw/documents/10179/11842/6p20v170.pdf
		 */
		{
			//10350,	// SYNC_LOW
			//450,		// SYNC_HIGH
			{ 23,  1 },	// SYNC HIGH_LOW
			0,			// SYNC_BIT_COUNT
			450,		// PULSE_TIME
			{  1,  2 },	// BIT0 HIGH_LOW
			{  2,  1 },	// BIT1 HIGH_LOW
			24,			// BIT_COUNT
			60,			// TOLERANCE
			0,			// REPEAT_DELAY
			true		// INVERSE
		},

		/*
		 * HT12A/HT12E chips
		 * http://www.holtek.com/documents/10179/116711/2_12ev120.pdf
		 */
		{
			//7560,		// SYNC_LOW
			//210,		// SYNC_HIGH
			{ 36,  1 },	// SYNC HIGH_LOW
			0,			// SYNC_BIT_COUNT
			210,		// PULSE_TIME
			{  1,  2 },	// BIT0 HIGH_LOW
			{  2,  1 },	// BIT1 HIGH_LOW
			12,			// BIT_COUNT
			60,			// TOLERANCE
			0,			// REPEAT_DELAY
			true		// INVERSE
		}
};

#endif /* INC_RF_PROTOCOLS_H_ */
