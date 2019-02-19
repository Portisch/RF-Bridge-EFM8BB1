/*
 * RF_Config.h
 *
 *  Created on: 19.02.2019
 *      Author: Portisch
 *
 *  Enable/Disable the protocols you want to include when compiling
 *  The memory of the used EFM8BB1 is limited to 8KB
 */

// typical protocols, disable here!             Enable	Remarks
#define EFM8BB1_SUPPORT_PT226X_PROTOCOL			1		// PT2260, EV1527,... original RF bridge protocol
#define EFM8BB1_SUPPORT_HT6P20X_PROTOCOL		1		// HT6P20X chips
#define EFM8BB1_SUPPORT_HT12_PROTOCOL			1		// HT12A/HT12E chips

// more protocols, enable here!                 Enable  Remarks
#define EFM8BB1_SUPPORT_Rohrmotor24_PROTOCOL	0		// Rohrmotor24
#define EFM8BB1_SUPPORT_PAR56_PROTOCOL			0		// UNDERWATER PAR56 LED LAMP, 502266
#define EFM8BB1_SUPPORT_WS_1200_PROTOCOL		0		// Alecto WS-1200 Series Wireless Weather Station
#define EFM8BB1_SUPPORT_ALDI_4x_PROTOCOL		0		// ALDI Remote controlled wall sockets, 4x
#define EFM8BB1_SUPPORT_SP45_PROTOCOL			0		// Meteo SPxx -  Weather station (PHU Metrex)
#define EFM8BB1_SUPPORT_DC90_PROTOCOL			0		// Dooya DC90 remote
#define EFM8BB1_SUPPORT_DG_HOSA_PROTOCOL		0		// Digoo DG-HOSA Smart 433MHz Wireless Household Carbon Monoxide Sensor
