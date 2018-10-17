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

#define MIN_FOOTER_LENGTH	3500
#define MIN_PULSE_LENGTH	100
#define MAX_BUCKETS			8

typedef enum
{
	DUTY_CYCLE,
	TIMING,
	BUCKET
} protocol_type_t;

typedef const struct
{
	// Protocol specific identifier
	uint8_t IDENTIFIER;
	// Protocol type
	protocol_type_t protocol_type;
	// Protocol data
	uint8_t *protocol_data;
} RF_PROTOCOLS;

// typedef for duty cycle protocols
typedef struct
{
	// normal high signal time on sync pulse
	uint16_t SYNC_HIGH;
	// normal low signal time on sync pulse
	uint16_t SYNC_LOW;
	// bit count of SYNC bits
	uint8_t SYNC_BIT_COUNT;
	// high time of a logic bit 1
	uint16_t BIT_HIGH_TIME;
	// high time of a logic bit 0
	uint16_t BIT_LOW_TIME;
	// duty cycle for logic bit 1
	uint8_t BIT_HIGH_DUTY;
	// duty cycle for logic bit 0
	uint8_t BIT_LOW_DUTY;
	// bit count for this protocol
	uint8_t BIT_COUNT;
	// delay in microseconds between the repeats
	uint8_t REPEAT_DELAY;
} DUTY_CYLCE_PROTOCOL_DATA;


typedef struct
{
	uint8_t high;
	uint8_t low;
} HIGH_LOW;

// typedef for timing protocols
typedef struct
{
	// normal high signal time on sync pulse
	uint16_t SYNC_HIGH;
	// normal low signal time on sync pulse
	uint16_t SYNC_LOW;
	// pulse time
	uint16_t PULSE_TIME;
	// high time of a logic bit 0
	HIGH_LOW bit0;
	// time to determine a logic bit 1 or 0
	HIGH_LOW bit1;
	// bit count for this protocol
	uint8_t BIT_COUNT;
	// decode tolerance in %
	uint8_t TOLERANCE;
	// delay in microseconds between the repeats
	uint8_t REPEAT_DELAY;
} TIMING_PROTOCOL_DATA;

#define SYNC_TOLERANCE 			10
#define SYNC_TOLERANCE_MAX		500
#define DUTY_CYCLE_TOLERANCE 	8

#define RF_TRANSMIT_REPEATS		8

#define UNKNOWN_IDENTIFIER				0x00
#define UNDEFINED_DUTY_CYCLE			0x7F
#define UNDEFINED_TIMING				0x80

/*
 * PT2260, EV1527,... original RF bridge protocol
 * http://www.princeton.com.tw/Portals/0/Product/PT2260_4.pdf
 */
#define PT2260_IDENTIFIER				0x01
#define PT226x_SYNC_MIN					4500
#define PT226x_SYNC_MAX					18000

SI_SEGMENT_VARIABLE(PT2260, static DUTY_CYLCE_PROTOCOL_DATA, SI_SEG_CODE) =
	{
		400,		// SYNC_HIGH
		12400,		// SYNC_LOW
		0,			// SYNC_BIT_COUNT
		1600,		// BIT_HIGH_TIME
		400,		// BIT_LOW_TIME
		75,			// BIT_HIGH_DUTY
		25,			// BIT_LOW_DUTY
		24,			// BIT_COUNT
		0			// REPEAT_DELAY
	};


/*
 * Rohrmotor24
 * https://github.com/bjwelker/Raspi-Rollo/tree/master/Arduino/Rollo_Code_Receiver
 */
#define ROHRMOTOR24_IDENTIFIER			0x02

SI_SEGMENT_VARIABLE(ROHRMOTOR24, static DUTY_CYLCE_PROTOCOL_DATA, SI_SEG_CODE) =
	{
		4800,		// SYNC_HIGH
		1500,		// SYNC_LOW
		0,			// SYNC_BIT_COUNT
		700,		// BIT_HIGH_TIME
		300,		// BIT_LOW_TIME
		70,			// BIT_HIGH_DUTY
		30,			// BIT_LOW_DUTY
		40,			// BIT_COUNT
		8			// REPEAT_DELAY
	};

/*
 * UNDERWATER PAR56 LED LAMP, 502266
 * http://www.seamaid-lighting.com/de/produit/lampe-par56/
 */
#define Seamaid_PAR_56_RGB_IDENTIFIER	0x03

SI_SEGMENT_VARIABLE(Seamaid_PAR_56_RGB, static DUTY_CYLCE_PROTOCOL_DATA, SI_SEG_CODE) =
	{
		3000,		// SYNC_HIGH
		9000,		// SYNC_LOW
		0,			// SYNC_BIT_COUNT
		1100,		// BIT_HIGH_TIME
		400,		// BIT_LOW_TIME
		75,			// BIT_HIGH_DUTY
		25,			// BIT_LOW_DUTY
		24,			// BIT_COUNT
		0			// REPEAT_DELAY
	};

/*
 * Wall plug Noru
  */
#define NORU_IDENTIFIER					0x04

SI_SEGMENT_VARIABLE(NORU, static DUTY_CYLCE_PROTOCOL_DATA, SI_SEG_CODE) =
	{
		9500,		// SYNC_HIGH
		3000,		// SYNC_LOW
		0,			// SYNC_BIT_COUNT
		900,		// BIT_HIGH_TIME
		320,		// BIT_LOW_TIME
		70,			// BIT_HIGH_DUTY
		30,			// BIT_LOW_DUTY
		24,			// BIT_COUNT
		0			// REPEAT_DELAY
	};

/*
 * WS-1200 Series Wireless Weather Station
  */
#define WS_1200_IDENTIFIER				0x05

SI_SEGMENT_VARIABLE(WS_1200, static DUTY_CYLCE_PROTOCOL_DATA, SI_SEG_CODE) =
	{
		0,			// SYNC_HIGH
		29400,		// SYNC_LOW
		7,			// SYNC_BIT_COUNT
		700,		// BIT_HIGH_TIME
		300,		// BIT_LOW_TIME
		38,			// BIT_HIGH_DUTY
		64,			// BIT_LOW_DUTY
		64,			// BIT_COUNT
		0			// REPEAT_DELAY
	};

/*
 * ALDI Remote controlled wall sockets, 4x
  */
#define ALDI_RCWS_IDENTIFIER			0x06

SI_SEGMENT_VARIABLE(ALDI_RCWS, static TIMING_PROTOCOL_DATA, SI_SEG_CODE) =
	{
		3000,		// SYNC_HIGH
		7250,		// SYNC_LOW
		400,		// PULSE_TIME
		{ 1, 3 },	// bit0 HIGH_LOW
		{ 3, 1 },	// bit1 HIGH_LOW
		24,			// BIT_COUNT
		70,
		0			// REPEAT_DELAY
	};

/*
 * Protocol array
 */
#define PROTOCOLCOUNT	6
#if PROTOCOLCOUNT > 0x7F
#error Too much protocols are defined, stop!
#endif

SI_SEGMENT_VARIABLE(PROTOCOL_DATA[PROTOCOLCOUNT], static const RF_PROTOCOLS, SI_SEG_CODE) =
{
		{
				PT2260_IDENTIFIER,
				DUTY_CYCLE,
				&PT2260
		},
		{
				ROHRMOTOR24_IDENTIFIER,
				DUTY_CYCLE,
				&ROHRMOTOR24
		},
		{
				Seamaid_PAR_56_RGB_IDENTIFIER,
				DUTY_CYCLE,
				&Seamaid_PAR_56_RGB
		},
		{
				NORU_IDENTIFIER,
				DUTY_CYCLE,
				&NORU
		},
		{
				WS_1200_IDENTIFIER,
				DUTY_CYCLE,
				&WS_1200
		},
		{
				ALDI_RCWS_IDENTIFIER,
				TIMING,
				&ALDI_RCWS
		}
};

#endif /* INC_RF_PROTOCOLS_H_ */
