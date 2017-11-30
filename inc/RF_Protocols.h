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
	// time in 탎 for one bit. This is the sum of the high and low time of on bit
	// example bit 1: high time 700탎, low time 300탎: sum is 1000탎 == 1kHz
	uint16_t BIT_TIME;
	// duty cycle for logic bit 1
	uint8_t BIT_HIGH_DUTY;
	// duty cycle for logic bit 0
	uint8_t BIT_LOW_DUTY;
	// bit count for this protocol
	uint8_t BIT_COUNT;
} PROTOCOL_DATA_t;

#define SYNC_TOLERANCE 			200

/*
 * Rohrmotor24
 * https://github.com/bjwelker/Raspi-Rollo/tree/master/Arduino/Rollo_Code_Receiver
 */
#define ROHRMOTOR24_IDENTIFIER	0x01
#define ROHRMOTOR24			{ROHRMOTOR24_IDENTIFIER, 4800, 1500, 1000, 30, 70, 40}

/*
 * UNDERWATER PAR56 LED LAMP, 502266
 * http://www.seamaid-lighting.com/de/produit/lampe-par56/
 */
#define Seamaid_PAR_56_RGB_IDENTIFIER	0x02
#define Seamaid_PAR_56_RGB	{Seamaid_PAR_56_RGB_IDENTIFIER, 3000, 9000, 1500, 25, 75, 24}


/*
 * Protocol array
 */
static const PROTOCOL_DATA_t PROTOCOL_DATA[2] = { ROHRMOTOR24, Seamaid_PAR_56_RGB};
#define PROTOCOLCOUNT sizeof PROTOCOL_DATA / sizeof PROTOCOL_DATA[0]



#endif /* INC_RF_PROTOCOLS_H_ */
