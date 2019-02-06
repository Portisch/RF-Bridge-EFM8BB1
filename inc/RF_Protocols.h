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
#include "RF_Config.h"

/*
 * bucket sniffing constants
 */
#define MIN_FOOTER_LENGTH	300
#define MIN_PULSE_LENGTH	100

#define PT226x_SYNC_MIN		4500

/*
 * sync constants
 */
#define TOLERANCE_MAX		500
#define TOLERANCE_MIN		100

/*
 * number of repeating by default
 */
#define RF_TRANSMIT_REPEATS		8

typedef struct PROTOCOL_STATUS
{
	uint16_t status;
	uint8_t bit_count;
	uint8_t actual_bit_of_byte;
} PROTOCOL_STATUS;

typedef struct PROTOCOL_DATA_UINT8_T
{
	// pointer to array of uint8_t elements
	SI_VARIABLE_SEGMENT_POINTER(dat, uint8_t, SI_SEG_CODE);
	// size of the array
	uint8_t size;
} PROTOCOL_DATA_UINT8_T;

typedef struct PROTOCOL_DATA_UINT16_T
{
	// pointer to array of uint16_t elements
	SI_VARIABLE_SEGMENT_POINTER(dat, uint16_t, SI_SEG_CODE);
	// size of the array
	uint8_t size;
} PROTOCOL_DATA_UINT16_T;

typedef struct BUCKET_PROTOCOL_DATA
{
	// array and array size of buckets
	PROTOCOL_DATA_UINT16_T pulses;
	// array and array size of start buckets
	PROTOCOL_DATA_UINT8_T start;
	// array and array size of bit 0 buckets
	PROTOCOL_DATA_UINT8_T bit0;
	// array and array size of bit 1 buckets
	PROTOCOL_DATA_UINT8_T bit1;
	// bit count for this protocol
	uint8_t bit_count;
	// protocol with inverse pulses
	uint8_t inverse;
} BUCKET_PROTOCOL_DATA;

/*
 * PT2260, EV1527,... original RF bridge protocol
 * http://www.princeton.com.tw/Portals/0/Product/PT2260_4.pdf
 */
#if EFM8BB1_SUPPORT_PT226X_PROTOCOL == 1
#define PT226X
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(PT226X)[], static uint16_t, SI_SEG_CODE) = { 350, 1050, 10850 };
SI_SEGMENT_VARIABLE(PROTOCOL_START(PT226X)[], static uint8_t, SI_SEG_CODE) = { 0, 2 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(PT226X)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(PT226X)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * Rohrmotor24
 * https://github.com/bjwelker/Raspi-Rollo/tree/master/Arduino/Rollo_Code_Receiver
 */
#if EFM8BB1_SUPPORT_Rohrmotor24_PROTOCOL == 1
#define Rohrmotor24
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(Rohrmotor24)[], static uint16_t, SI_SEG_CODE) = { 370, 740, 4800, 1500};
SI_SEGMENT_VARIABLE(PROTOCOL_START(Rohrmotor24)[], static uint8_t, SI_SEG_CODE) = { 2, 3 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(Rohrmotor24)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(Rohrmotor24)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * UNDERWATER PAR56 LED LAMP, 502266
 * http://www.seamaid-lighting.com/de/produit/lampe-par56/
 */
#if EFM8BB1_SUPPORT_PAR56_PROTOCOL == 1
#define PAR56
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(PAR56)[], static uint16_t, SI_SEG_CODE) = { 380, 1100, 3000, 9000};
SI_SEGMENT_VARIABLE(PROTOCOL_START(PAR56)[], static uint8_t, SI_SEG_CODE) = { 2, 3 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(PAR56)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(PAR56)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * Alecto WS-1200 Series Wireless Weather Station
 */
#if EFM8BB1_SUPPORT_WS_1200_PROTOCOL == 1
#define WS_1200
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(WS_1200)[], static uint16_t, SI_SEG_CODE) = { 500, 1000, 1500, 29500 };
SI_SEGMENT_VARIABLE(PROTOCOL_START(WS_1200)[], static uint8_t, SI_SEG_CODE) = { 3 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(WS_1200)[], static uint8_t, SI_SEG_CODE) = { 2, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(WS_1200)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
#endif

/*
 * ALDI Remote controlled wall sockets, 4x
  */
#if EFM8BB1_SUPPORT_ALDI_4x_PROTOCOL == 1
#define ALDI_4x
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(ALDI_4x)[], static uint16_t, SI_SEG_CODE) = { 400, 1200, 3000, 7250};
SI_SEGMENT_VARIABLE(PROTOCOL_START(ALDI_4x)[], static uint8_t, SI_SEG_CODE) = { 2, 3 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(ALDI_4x)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(ALDI_4x)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * HT6P20X chips
 * http://www.holtek.com.tw/documents/10179/11842/6p20v170.pdf
 */
#if EFM8BB1_SUPPORT_HT6P20X_PROTOCOL == 1
#define HT6P20X
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(HT6P20X)[], static uint16_t, SI_SEG_CODE) = { 450, 900, 10350};
SI_SEGMENT_VARIABLE(PROTOCOL_START(HT6P20X)[], static uint8_t, SI_SEG_CODE) = { 2, 0 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(HT6P20X)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(HT6P20X)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * HT12A/HT12E chips
 * http://www.holtek.com/documents/10179/116711/2_12ev120.pdf
 */
#if EFM8BB1_SUPPORT_HT12_PROTOCOL == 1
#define HT12
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(HT12)[], static uint16_t, SI_SEG_CODE) = { 210, 420, 7560};
SI_SEGMENT_VARIABLE(PROTOCOL_START(HT12)[], static uint8_t, SI_SEG_CODE) = { 2, 0 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(HT12)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(HT12)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * HT12A/HT12E chips - Generic Doorbell
 * http://www.holtek.com/documents/10179/116711/2_12ev120.pdf
 */
#if EFM8BB1_SUPPORT_HT12a_PROTOCOL == 1
#define HT12a
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(HT12a)[], static uint16_t, SI_SEG_CODE) = { 200, 380, 6950};
SI_SEGMENT_VARIABLE(PROTOCOL_START(HT12a)[], static uint8_t, SI_SEG_CODE) = { 2, 0 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(HT12a)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(HT12a)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * HT12A/HT12E chips - Atag Extractor - Plus/Minus
 * http://www.holtek.com/documents/10179/116711/2_12ev120.pdf
 */
#if EFM8BB1_SUPPORT_HT12_Atag_PROTOCOL == 1
#define HT12b
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(HT12b)[], static uint16_t, SI_SEG_CODE) = { 350, 650, 13000};
SI_SEGMENT_VARIABLE(PROTOCOL_START(HT12b)[], static uint8_t, SI_SEG_CODE) = { 2, 0 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(HT12b)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(HT12b)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * HT12A/HT12E chips - Atag Extractor - Lights/Timer
 * http://www.holtek.com/documents/10179/116711/2_12ev120.pdf
 */
#if EFM8BB1_SUPPORT_HT12_Atag_PROTOCOL == 1
#define HT12c
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(HT12c)[], static uint16_t, SI_SEG_CODE) = { 350, 700, 15650};
SI_SEGMENT_VARIABLE(PROTOCOL_START(HT12c)[], static uint8_t, SI_SEG_CODE) = { 2, 0 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(HT12c)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(HT12c)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * Meteo SPxx -  Weather station (PHU Metrex)
 * https://gist.github.com/klaper/ce3ba02501516d9a6d294367d2c300a6
 */
#if EFM8BB1_SUPPORT_SP45_PROTOCOL == 1
#define SP45
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(SP45)[], static uint16_t, SI_SEG_CODE) = { 650, 7810, 1820, 3980 };
SI_SEGMENT_VARIABLE(PROTOCOL_START(SP45)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(SP45)[], static uint8_t, SI_SEG_CODE) = { 0, 2 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(SP45)[], static uint8_t, SI_SEG_CODE) = { 0, 3 };
#endif

/*
 * Dooya DC90 remote
 */
#if EFM8BB1_SUPPORT_DC90_PROTOCOL == 1
#define DC90
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(DC90)[], static uint16_t, SI_SEG_CODE) = { 360, 720, 4800, 1500 };
SI_SEGMENT_VARIABLE(PROTOCOL_START(DC90)[], static uint8_t, SI_SEG_CODE) = { 2, 3 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(DC90)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(DC90)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

/*
 * Digoo DG-HOSA Smart 433MHz Wireless Household Carbon Monoxide Sensor
 */
#if EFM8BB1_SUPPORT_DG_HOSA_PROTOCOL == 1
#define DG_HOSA
SI_SEGMENT_VARIABLE(PROTOCOL_PULSES(DG_HOSA)[], static uint16_t, SI_SEG_CODE) = { 590, 1500, 430, 13450 };
SI_SEGMENT_VARIABLE(PROTOCOL_START(DG_HOSA)[], static uint8_t, SI_SEG_CODE) = { 2, 3 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT0(DG_HOSA)[], static uint8_t, SI_SEG_CODE) = { 0, 1 };
SI_SEGMENT_VARIABLE(PROTOCOL_BIT1(DG_HOSA)[], static uint8_t, SI_SEG_CODE) = { 1, 0 };
#endif

SI_SEGMENT_VARIABLE(PROTOCOL_DATA[], static struct BUCKET_PROTOCOL_DATA, SI_SEG_CODE) =
{
#if EFM8BB1_SUPPORT_PT226X_PROTOCOL == 1
		/*
		 * PT2260, EV1527,... original RF bridge protocol
		 */
		{
			{ &PROTOCOL_PULSES(PT226X), ARRAY_LENGTH(PROTOCOL_PULSES(PT226X)) },
			{ &PROTOCOL_START(PT226X), ARRAY_LENGTH(PROTOCOL_START(PT226X)) },
			{ &PROTOCOL_BIT0(PT226X), ARRAY_LENGTH(PROTOCOL_BIT0(PT226X)) },
			{ &PROTOCOL_BIT1(PT226X), ARRAY_LENGTH(PROTOCOL_BIT1(PT226X)) },
			24,
			false
		},
#endif
#if EFM8BB1_SUPPORT_Rohrmotor24_PROTOCOL == 1
		/*
		 * Rohrmotor24
		 */
		{
			{ &PROTOCOL_PULSES(Rohrmotor24), ARRAY_LENGTH(PROTOCOL_PULSES(Rohrmotor24)) },
			{ &PROTOCOL_START(Rohrmotor24), ARRAY_LENGTH(PROTOCOL_START(Rohrmotor24)) },
			{ &PROTOCOL_BIT0(Rohrmotor24), ARRAY_LENGTH(PROTOCOL_BIT0(Rohrmotor24)) },
			{ &PROTOCOL_BIT1(Rohrmotor24), ARRAY_LENGTH(PROTOCOL_BIT1(Rohrmotor24)) },
			40,
			false
		},
#endif
#if EFM8BB1_SUPPORT_PAR56_PROTOCOL == 1
		/*
		 * UNDERWATER PAR56 LED LAMP, 502266
		 */
		{
			{ &PROTOCOL_PULSES(PAR56), ARRAY_LENGTH(PROTOCOL_PULSES(PAR56)) },
			{ &PROTOCOL_START(PAR56), ARRAY_LENGTH(PROTOCOL_START(PAR56)) },
			{ &PROTOCOL_BIT0(PAR56), ARRAY_LENGTH(PROTOCOL_BIT0(PAR56)) },
			{ &PROTOCOL_BIT1(PAR56), ARRAY_LENGTH(PROTOCOL_BIT1(PAR56)) },
			24,
			false
		},
#endif
#if EFM8BB1_SUPPORT_WS_1200_PROTOCOL == 1
		/*
		 * Alecto WS-1200 Series Wireless Weather Station
		 */
		{
			{ &PROTOCOL_PULSES(WS_1200), ARRAY_LENGTH(PROTOCOL_PULSES(WS_1200)) },
			{ &PROTOCOL_START(WS_1200), ARRAY_LENGTH(PROTOCOL_START(WS_1200)) },
			{ &PROTOCOL_BIT0(WS_1200), ARRAY_LENGTH(PROTOCOL_BIT0(WS_1200)) },
			{ &PROTOCOL_BIT1(WS_1200), ARRAY_LENGTH(PROTOCOL_BIT1(WS_1200)) },
			71,
			false
		},
#endif
#if EFM8BB1_SUPPORT_ALDI_4x_PROTOCOL == 1
		/*
		 * ALDI Remote controlled wall sockets, 4x
		 */
		{
			{ &PROTOCOL_PULSES(ALDI_4x), ARRAY_LENGTH(PROTOCOL_PULSES(ALDI_4x)) },
			{ &PROTOCOL_START(ALDI_4x), ARRAY_LENGTH(PROTOCOL_START(ALDI_4x)) },
			{ &PROTOCOL_BIT0(ALDI_4x), ARRAY_LENGTH(PROTOCOL_BIT0(ALDI_4x)) },
			{ &PROTOCOL_BIT1(ALDI_4x), ARRAY_LENGTH(PROTOCOL_BIT1(ALDI_4x)) },
			24,
			false
		},
#endif
#if EFM8BB1_SUPPORT_HT6P20X_PROTOCOL == 1
		/*
		 * HT6P20X chips
		 */
		{
			{ &PROTOCOL_PULSES(HT6P20X), ARRAY_LENGTH(PROTOCOL_PULSES(HT6P20X)) },
			{ &PROTOCOL_START(HT6P20X), ARRAY_LENGTH(PROTOCOL_START(HT6P20X)) },
			{ &PROTOCOL_BIT0(HT6P20X), ARRAY_LENGTH(PROTOCOL_BIT0(HT6P20X)) },
			{ &PROTOCOL_BIT1(HT6P20X), ARRAY_LENGTH(PROTOCOL_BIT1(HT6P20X)) },
			24,
			true
		},
#endif
#if EFM8BB1_SUPPORT_HT12_PROTOCOL == 1
		/*
		 * HT12A/HT12E chips
		 */
		{
			{ &PROTOCOL_PULSES(HT12), ARRAY_LENGTH(PROTOCOL_PULSES(HT12)) },
			{ &PROTOCOL_START(HT12), ARRAY_LENGTH(PROTOCOL_START(HT12)) },
			{ &PROTOCOL_BIT0(HT12), ARRAY_LENGTH(PROTOCOL_BIT0(HT12)) },
			{ &PROTOCOL_BIT1(HT12), ARRAY_LENGTH(PROTOCOL_BIT1(HT12)) },
			12,
			true
		},
#endif
#if EFM8BB1_SUPPORT_HT12a_PROTOCOL == 1
		/*
		 * HT12A/HT12E chips - A
		 */
		{
			{ &PROTOCOL_PULSES(HT12a), ARRAY_LENGTH(PROTOCOL_PULSES(HT12a)) },
			{ &PROTOCOL_START(HT12a), ARRAY_LENGTH(PROTOCOL_START(HT12a)) },
			{ &PROTOCOL_BIT0(HT12a), ARRAY_LENGTH(PROTOCOL_BIT0(HT12a)) },
			{ &PROTOCOL_BIT1(HT12a), ARRAY_LENGTH(PROTOCOL_BIT1(HT12a)) },
			12,
			true
		},
#endif
#if EFM8BB1_SUPPORT_HT12_Atag_PROTOCOL == 1
		/*
		 * HT12A/HT12E chips - B
		 */
		{
			{ &PROTOCOL_PULSES(HT12b), ARRAY_LENGTH(PROTOCOL_PULSES(HT12b)) },
			{ &PROTOCOL_START(HT12b), ARRAY_LENGTH(PROTOCOL_START(HT12b)) },
			{ &PROTOCOL_BIT0(HT12b), ARRAY_LENGTH(PROTOCOL_BIT0(HT12b)) },
			{ &PROTOCOL_BIT1(HT12b), ARRAY_LENGTH(PROTOCOL_BIT1(HT12b)) },
			12,
			true
		},
#endif
#if EFM8BB1_SUPPORT_HT12_Atag_PROTOCOL == 1
		/*
		 * HT12A/HT12E chips - C
		 */
		{
			{ &PROTOCOL_PULSES(HT12c), ARRAY_LENGTH(PROTOCOL_PULSES(HT12c)) },
			{ &PROTOCOL_START(HT12c), ARRAY_LENGTH(PROTOCOL_START(HT12c)) },
			{ &PROTOCOL_BIT0(HT12c), ARRAY_LENGTH(PROTOCOL_BIT0(HT12c)) },
			{ &PROTOCOL_BIT1(HT12c), ARRAY_LENGTH(PROTOCOL_BIT1(HT12c)) },
			18,
			true
		},
#endif
#if EFM8BB1_SUPPORT_SP45_PROTOCOL == 1
		/*
		 * Meteo SPxx -  Weather station (PHU Metrex)
		 */
		{
			{ &PROTOCOL_PULSES(SP45), ARRAY_LENGTH(PROTOCOL_PULSES(SP45)) },
			{ &PROTOCOL_START(SP45), ARRAY_LENGTH(PROTOCOL_START(SP45)) },
			{ &PROTOCOL_BIT0(SP45), ARRAY_LENGTH(PROTOCOL_BIT0(SP45)) },
			{ &PROTOCOL_BIT1(SP45), ARRAY_LENGTH(PROTOCOL_BIT1(SP45)) },
			40,
			false
		},
#endif
#if EFM8BB1_SUPPORT_DC90_PROTOCOL == 1
		/*
		 * Dooya DC90 remote
		 */
		{
			{ &PROTOCOL_PULSES(DC90), ARRAY_LENGTH(PROTOCOL_PULSES(DC90)) },
			{ &PROTOCOL_START(DC90), ARRAY_LENGTH(PROTOCOL_START(DC90)) },
			{ &PROTOCOL_BIT0(DC90), ARRAY_LENGTH(PROTOCOL_BIT0(DC90)) },
			{ &PROTOCOL_BIT1(DC90), ARRAY_LENGTH(PROTOCOL_BIT1(DC90)) },
			40,
			false
		},
#endif
#if EFM8BB1_SUPPORT_DG_HOSA_PROTOCOL == 1
		/*
		 * Digoo DG-HOSA Smart 433MHz Wireless Household Carbon Monoxide Sensor
		 */
		{
			{ &PROTOCOL_PULSES(DG_HOSA), ARRAY_LENGTH(PROTOCOL_PULSES(DG_HOSA)) },
			{ &PROTOCOL_START(DG_HOSA), ARRAY_LENGTH(PROTOCOL_START(DG_HOSA)) },
			{ &PROTOCOL_BIT0(DG_HOSA), ARRAY_LENGTH(PROTOCOL_BIT0(DG_HOSA)) },
			{ &PROTOCOL_BIT1(DG_HOSA), ARRAY_LENGTH(PROTOCOL_BIT1(DG_HOSA)) },
			24,
			false
		},
#endif
};

#define PROTOCOLCOUNT (sizeof(PROTOCOL_DATA) / sizeof(PROTOCOL_DATA[0]))
#endif /* INC_RF_PROTOCOLS_H_ */
