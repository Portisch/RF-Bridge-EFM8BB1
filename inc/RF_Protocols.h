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

typedef struct
{
	// Protocol specific identifier
	uint8_t IDENTIFIER;
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
} PROTOCOL_DATA_t;

#define SYNC_TOLERANCE 			200
#define DUTY_CYCLE_TOLERANCE 	8

/*
 * Rohrmotor24
 * https://github.com/bjwelker/Raspi-Rollo/tree/master/Arduino/Rollo_Code_Receiver
 */
#define ROHRMOTOR24_IDENTIFIER			0x01
#define ROHRMOTOR24			{ROHRMOTOR24_IDENTIFIER, 4800, 1500, 0, 700, 300, 70, 30, 40}

/*
 * UNDERWATER PAR56 LED LAMP, 502266
 * http://www.seamaid-lighting.com/de/produit/lampe-par56/
 */
#define Seamaid_PAR_56_RGB_IDENTIFIER	0x02
#define Seamaid_PAR_56_RGB	{Seamaid_PAR_56_RGB_IDENTIFIER, 3000, 9000, 0, 1100, 400, 75, 25, 24}

/*
 * Wall plug Noru
  */
#define NORU_IDENTIFIER					0x03
#define NORU				{NORU_IDENTIFIER, 9500, 3000, 0, 900, 320, 70, 30, 24}

/*
 * WS-1200 Series Wireless Weather Station
  */
#define WS_1200_IDENTIFIER				0x04
#define WS_1200				{WS_1200_IDENTIFIER, 0, 29400, 7, 700, 300, 38, 64, 64}


/*
 * Protocol array
 */
#define PROTOCOLCOUNT	4
#if PROTOCOLCOUNT > 0x7F
#error Too much protocols are defined, stop!
#endif

SI_SEGMENT_VARIABLE(PROTOCOL_DATA[PROTOCOLCOUNT], static const PROTOCOL_DATA_t, SI_SEG_CODE) =
{
		ROHRMOTOR24,
		Seamaid_PAR_56_RGB,
		NORU,
		WS_1200
};

#endif /* INC_RF_PROTOCOLS_H_ */
