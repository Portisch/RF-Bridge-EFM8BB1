# RF-Bridge-EFM8BB1
RF-Bridge-EFM8BB1

The Sonoff RF Bridge is only supporting one protocol with 24 bits.<br/>
The Idea is to write a alternative firmware for the onboard EFM8BB1 chip.

All original commands 0xA0 to 0xA5 are supported!

# Hardware
There are the pins C2 & C2CK on the board. With a Arduino you can build a programmer to read/erase and program the flash.
Software for the Arduino: https://github.com/conorpp/efm8-arduino-programmer
<br/>
![RF Bridge v1.0](https://raw.github.com/Portisch/RF-Bridge-EFM8BB1/master/doc/Bridge_v1_0.jpg)
![RF Bridge R2 v1.0](https://raw.github.com/Portisch/RF-Bridge-EFM8BB1/master/doc/Bridge_R2_v1_0.jpg)
<br/>
**C2 Interface**<br/>
Use this header to program the firmware on the EFM8BB1.

**RS232**<br/>
Use this RS232 connection for debug/testing. Put the switch S2 to OFF when connecting.
For normal operation the switch S2 have to be in ON position.

# Software
The project is written with Simplicity Studio 4. The resulting *.hex file can be programmed on the EFM8BB1.

# First results
The reading of RF signals is already working:<br/>
Sending start sniffing: 0xAA 0xA6 0x55<br/>
Receiving AKN: 0xAA 0xA0 0x55<br/>

Sending stop sniffing: 0xAA 0xA7 0x55<br/>
Receiving AKN: 0xAA 0xA0 0x55<br/>

# Defining new RF protocols in RF_Protocols.h
If your remote can't be sniffed with the command 0xA6 please use command 0xB1.</br>
This will do bucket sniffing and give maybe a result like this:</br>
Hex: AA B1 04 0439 01C5 0BAE 1C3E 01 10 10 10 10 01 01 01 10 10 01 01 10 10 01 01 01 10 01 01 10 10 10 10 23 55</br>
</br>
0xAA: uart sync init<br/>
0xB1: bucket sniffing<br/>
0x04: bucket count including sync bucket<br/>
0x0439: Bucket 1 length: 1081µs<br/>
0x01C5: Bucket 2 length: 453µs<br/>
0x0BAE: Bucket 3 length: 2990µs<br/>
0x1C3E: Bucket 4 length: 7230µs<br/>
0x01-0x23: RF data received (high/low nibbles denote buckets)<br/>
0x55: uart sync end</br>
</br>
If the data is only including 01, 10 and 23 at the end than it should be possible to decode the signal by command 0xA6.</br>
But for this the protocol have to be defined in the RF_Protocols.h file.

01 means:</br>
<img src="https://raw.github.com/Portisch/RF-Bridge-EFM8BB1/master/doc/01_Bit_1.png" width="400" ></br>
BIT_HIGH_TIME is 1081µs.</br>
BIT_HIGH_DUTY = 100% / (1081µs + 453µS) * 1081µs =~ 70%. </br>

10 means:</br>
<img src="https://raw.github.com/Portisch/RF-Bridge-EFM8BB1/master/doc/10_Bit_0.png" width="400" ></br>
BIT_LOW_TIME is 453µs.</br>
BIT_LOW_DUTY = 100% / (1081µs + 453µS) * 453µs =~ 30%.</br>

23 means:</br>
<img src="https://raw.github.com/Portisch/RF-Bridge-EFM8BB1/master/doc/23_Sync_Bit.png" width="400" ></br>
SYNC_HIGH is 2990µs.</br>
SYNC_LOW is 7230µs.</br>
SYNC_BIT_COUNT is default 0.</br>

The bitcount can be counted like this:</br>

| data 1 | data 2 | data 3 | data 4 | data 5 | data 6 | data 7 | data 8 | data 9 | data 10 | data 11 | data 12 | data 13 | data 14 | data 15 | data 16 | data 17 | data 18 | data 19 | data 20 | data 21 | data 22 | data 23 | data 24 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 01 | 10 | 10 | 10 | 10 | 01 | 01 | 01 | 10 | 10 | 01 | 01 | 10 | 10 | 01 | 01 | 01 | 10 | 01 | 01 | 10 | 10 | 10 | 10 |
| bit 1 | bit 2 | bit 3 | bit 4 | bit 5 | bit 6 | bit 7 | bit 8 | bit 9 | bit 10 | bit 11 | bit 12 | bit 13 | bit 14 | bit 15 | bit 16 | bit 17 | bit 18 | bit 19 | bit 20 | bit 21 | bit 22 | bit 23 | bit 24 |

BIT_COUNT = 24.</br>
REPEAT_DELAY is default 0.</br>

## Firmware identify by command 0xFF
If you get a response to this command the EFM8 chip is running this alternative firmware
<br/>
Hex: AA FF 55<br/>
<br/>
Receiving AKN: AA xx 55<br/>

The xx is the firmware version (0x00-0xFF)

## RF decode from Rohrmotor24.de remote (40 bit of data):
0xAA: uart sync init<br/>
0xA6: sniffing active<br/>
0x06: data len<br/>
0x02: protocol identifier<br/>
0xD0-0x55: data<br/>
0x55: uart sync end

STOP:<br/>
Binary: 10101010 10100110 00000110 00000001 11010000 11111001 00110010 00010001 01010101 01010101<br/>
Hex: AA A6 06 02 D0 F9 32 11 55 55<br/>
DOWN:<br/>
Binary: 10101010 10100110 00000110 00000001 11010000 11111001 00110010 00010001 00110011 01010101<br/>
Hex: AA A6 06 02 D0 F9 32 11 33 55<br/>

## RF decode from Seamaid_PAR_56_RGB remote (24 bit of data):
Light ON:<br/>
Binary: 10101010 10100110 00000100 00000011 00110010 11111010 10001111 01010101<br/>
Hex: AA A6 04 03 32 FA 8F 55<br/>

## Transmiting by command 0xA5
This is the original implemented RF transmit command<br/>
Hex: AA A5 24 E0 01 40 03 84 D0 03 58 55<br/>

0xAA: uart sync init<br/>
0x24-0xE0: Tsyn<br/>
0x01-0x40: Tlow<br/>
0x03-0x84: Thigh<br/>
0xD0-0x58: 24bit Data<br/>

The high time of the SYNC get calculated by the Tsyn (SYNC low time),<br/>
duty cycle of the high bit is 75% and 25% of the low bit.<br/>

## Sniffing by command 0xA6
With the command the standard PT226x sniffing gets disabled and the sniffing of the defined protocols in RF_Protocols.h is starting.<br/>
<br/>
Hex: AA A6 55

Example of a received decoded protocol:<br/>
Hex: AA A6 04 03 32 FA 80 55<br/>

0xAA: uart sync init<br/>
0xA6: sniffed RF data<br/>
0x04: data len<br/>
0x03: protocol identifier (Seamaid_PAR_56_RGB)<br/>
0x32-0x80: data<br/>
0x55: uart sync end

## Stop sniffing by command 0xA7
Stop the 0xA6 sniffing and restart the PT226x sniffing.

## Transmiting by command 0xA8
There is a new command in the firmware to be able to send RF data.<br/>
Predefined protocols in the RF_Protocols.h file can be used directly by<br/>
using the protocol identifier.<br/>

Hex: AA A8 06 02 D0 F9 32 11 33 55<br/>

0xAA: uart sync init<br/>
0xA8: transmit RF data<br/>
0x06: data len<br/>
0x02: protocol identifier (ROHRMOTOR24)<br/>
0xD0-0x33: data<br/>
0x55: uart sync end

#### Universal transmit of a duty cycle based protocol by command 0xA8
When 0x7F get used as protocol identifier the timing can be user defined<br/>
and do not have to be defined in RF_Protocols.h.<br/>
This methode can be used to find correct parameter to define the timing<br/>
in RF_Protocols.h for future.

Hex: AA A8 0D 7F 12 C0 05 DC 02 BC 46 01 2C 1E 08 1E 55<br/>

0xAA: uart sync init<br/>
0xA8: transmit RF data<br/>
0x0D: data len<br/>
0x7F: protocol identifier 0x7F<br/>
0x12-0xC0: SYNC_HIGH<br/>
0x05-0xDC: SYNC_LOW<br/>
0x02-0xBC: BIT_HIGH_TIME<br/>
0x46: BIT_HIGH_DUTY<br/>
0x01-0x2C: BIT_LOW_TIME<br/>
0x1E: BIT_LOW_DUTY<br/>
0x08: BIT_COUNT + SYNC_BIT_COUNT in front of RF data<br/>
0x1E: RF data to send<br/>
0x55: uart sync end<br/>

#### Universal transmit of a time based protocol by command 0xA8
When 0x80 get used as protocol identifier the timing can be user defined<br/>
and do not have to be defined in RF_Protocols.h.<br/>
This methode can be used to find correct parameter to define the timing<br/>
in RF_Protocols.h for future.

Hex: AA A8 0E 80 01 90 1B 58 00 C8 01 02 02 01 0C 1F B0 55<br/>

0xAA: uart sync init<br/>
0xA8: transmit RF data<br/>
0x0E: data len<br/>
0x80: protocol identifier 0x80<br/>
0x01-0x90: SYNC_HIGH<br/>
0x1B-0x58: SYNC_LOW<br/>
0x00-0xC8: PULSE_TIME<br/>
0x01: BIT_0_HIGH FACTOR<br/>
0x02: BIT_0_LOW FACTOR<br/>
0x02: BIT_1_HIGH FACTOR<br/>
0x01: BIT_1_LOW FACTOR<br/>
0x0C: BIT_COUNT<br/>
0x1F-0xB0: RF data to send<br/>
0x55: uart sync end<br/>

## Learning by command 0xA9
Hex: AA A9 55<br/>

With the new learning the RF Bridge will scan for all predefined protocols.
The first received RF code will be sent by OK 0xAB.
If a timeout happens 0xAA will be sent.

## Bucket Transmitting using command 0xB0
This command accommodates RF protocols that can have variable bit times.
With this command, up to 16 time buckets can be defined, that denote the length of a high (mark) or low (space) transmission phase, e.g. for tri-state mode RF protocols.
This command also accommodates code repetition often used for higher reliability.

Hex: AA B0 20 04 1A 0120 01C0 0300 2710 01212122012201212121212121220121212201212203 55

0xAA: uart sync init<br/>
0xB0: transmit bucketed RF data<br/>
0x20: data len: 32 bytes<br/>
0x04: number of buckets: 4<br/>
0x19: number of repetitions: (transmit 1+25 = 26 times)<br/>
0x01-0x20: Bucket 1 length: 288µs<br/>
0x01-0xC0: Bucket 2 length: 448µs<br/>
0x03-0x00: Bucket 3 length: 768µs<br/>
0x27-0x10: Bucket 4 length: 10ms (sync)<br/>
0x01-0x03: RF data to send (high/low nibbles denote buckets to use for RF high (on) and low (off))<br/>
0x55: uart sync end

Please note that currently, there is no learning mode for this!
However, you can use, e.g., an Arduino with the [RFControl](https://github.com/pimatic/RFControl)
library to learn the bucket times and sequences (the
[compressed](https://github.com/pimatic/RFControl/tree/master/examples/compressed) example
gives you everything you need if you convert the decimal numbers to hex).

## Bucket sniffing using command 0xB1
This command will do bucket sniffing.<br/>
<br/>
Hex: AA B1 04 0120 01C0 0300 2710 01212122012201212121212121220121212201212203 55
<br/>
0xAA: uart sync init<br/>
0xB1: bucket sniffing<br/>
0x04: bucket count including sync bucket<br/>
0x01-0x20: Bucket 1 length: 288µs<br/>
0x01-0xC0: Bucket 2 length: 448µs<br/>
0x03-0x00: Bucket 3 length: 768µs<br/>
0x27-0x10: Bucket 4 length: 10ms (sync)<br/>
0x01-0x03: RF data received (high/low nibbles denote buckets)<br/>
0x55: uart sync end

## 0xB1 to 0xB0 helping tool
After learning how bit bucket works from here https://github.com/Portisch/RF-Bridge-EFM8BB1/issues/23 this is a python script to help calculate the right 'B0' message to send using 'RfRaw' command in Tasmota from the received 'B1' sniffing message.

[BitBucketConverter.py by gerardovf](https://gist.github.com/gerardovf/15109405e8a53dd075bbe650d70fafc6)

In the command line give the 'B1' message string and the retries value (in decimal):<br/>
i.e. BitBucketConverter.py "AA B1 04 07EB 0157 00FD 3EBC 010101010101101001010101101010100103 55" 20

Command Line : "AA B1 04 07EB 0157 00FD 3EBC 010101010101101001010101101010100103 55" 20<br/>
Result: 'RfRaw AAB01C041407EB015700FD3EBC01010101010110100101010110101010010355'

For support/help take a look [here](https://github.com/Portisch/RF-Bridge-EFM8BB1/issues/31) or contact gerardovf.

## Beep by command 0xC0
Hex: AA C0 xx xx 55<br/>

Do beep xxxx miliseconds (uint16_t). Like AA C0 03 E8 55 will beep for ~1000ms.

# Next Steps
Add ESPurna support:<br/>
A new protocol have to be implemented to support more RF signals -> have to be defined!
