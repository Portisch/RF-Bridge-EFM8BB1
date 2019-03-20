/*
 * RF_Config.h
 *
 *  Created on: 19.02.2019
 *      Author: Portisch
 *
 *  Enable/Disable the protocols you want to include when compiling
 *  The memory of the used EFM8BB1 is limited to 8KB
 *
 *  Also a option is not including the 0xB1 bucket sniffing as this is only needed to define new protocols
 */

#define INCLUDE_BUCKET_SNIFFING					1

// typical protocols, disable here!             Enable	Remarks
#define EFM8BB1_SUPPORT_PT226X_PROTOCOL			1		// PT2260, EV1527,... original RF bridge protocol
#define EFM8BB1_SUPPORT_HT6P20X_PROTOCOL		1		// HT6P20X chips
#define EFM8BB1_SUPPORT_HT12_PROTOCOL			1		// HT12A/HT12E chips

// more protocols, enable here!                 Enable  Remarks
#define EFM8BB1_SUPPORT_Rohrmotor24_PROTOCOL	0		// Rohrmotor24
#define EFM8BB1_SUPPORT_PAR56_PROTOCOL			0		// UNDERWATER PAR56 LED LAMP, 502266
#define EFM8BB1_SUPPORT_WS_1200_PROTOCOL		0		// Alecto WS-1200 Series Wireless Weather Station
#define EFM8BB1_SUPPORT_ALDI_4x_PROTOCOL		1		// ALDI Remote controlled wall sockets, 4x
#define EFM8BB1_SUPPORT_SP45_PROTOCOL			1		// Meteo SPxx -  Weather station (PHU Metrex)
#define EFM8BB1_SUPPORT_DC90_PROTOCOL			1		// Dooya DC90 remote
#define EFM8BB1_SUPPORT_DG_HOSA_PROTOCOL		1		// Digoo DG-HOSA Smart 433MHz Wireless Household Carbon Monoxide Sensor
#define EFM8BB1_SUPPORT_HT12a_PROTOCOL			1		// HT12A/HT12E chips - Generic Doorbell
#define EFM8BB1_SUPPORT_HT12_Atag_PROTOCOL		1		// HT12A/HT12E chips - Atag Extractor - Plus/Minus/Lights/Timer
#define EFM8BB1_SUPPORT_Kaku_PROTOCOL			1		// KaKu wall sockets
#define EFM8BB1_SUPPORT_DIO_PROTOCOL			1		// DIO Chacon RF 433Mhz, Issue #95
#define EFM8BB1_SUPPORT_1BYONE_PROTOCOL			1		// 1ByOne Doorbell, PR #97
#define EFM8BB1_SUPPORT_Prologue_PROTOCOL		1		// Prologue Sensor, Issue #96
#define EFM8BB1_SUPPORT_DOG_COLLAR_PROTOCOL		1		// Generic dog training collar - board label T-187-n (TX)-1, PR #100
#define EFM8BB1_SUPPORT_BY302_PROTOCOL			1		// Byron BY302 Doorbell, Issue #102
