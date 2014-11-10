#include <Wire.h>
#include <Mesh.h>
#include "AquilaBridge.h"

/*
	New Data Format 10/11/14: Now we are using LWM, thus all MAC processing is done in the uC, we only send relevant data to PC.
		lqi rssi srcAddr(16) dstAddr(16) srcEndpoint dstEndpoint frameSize [LWM Data]
*/

/*
 *	Update 26/08/14: added command TX buffer and packet RX ring buffer
 *	Update 26/08/14: started implementing 16 bit CRC based on Xmodem: http://www.gaw.ru/pdf/Atmel/app/avr/AVR350.pdf (Not yet active)
 *	Serial Bridge Protocol: [PREAMBLE] COMMAND [Command specific] ([16 bit CRC] not yet activated)
 *		[PREAMBLE]: 0xAA 0x55 0xAA 0x55
 *		
 *	Comands:
 *		CMD_DATA: 			[Command specific] = lqi rssi frameSize [MAC Frame]
 *		CMD_SET_PROM:		[Command specific] = PROM*				*0 or 1
 *		CMD_SET_PAN:		[Command specific] = [PAN*]				*len = 2 (16 bit), lsb first
 *		CMD_SET_CHAN:		[Command specific] = CHAN*				*11 - 24
 *		CMD_START													*Sent on bridge start or reset and in response to CMD_PING
 *		CMD_SET_SHORT_ADDR:	[Command specific] = [ADDR*]			*len = 2 (16 bit), lsb first
 *		//CMD_SET_LONG_ADDR:	[Command specific] = [ADDR*]			*len = 8 (64 bit), lsb first
 *		CMD_PING:													*Sent by PC, response is CMD_START
 *		CMD_SUCESS:													*Sent on data transmit success
 *		CMD_ERROR:													*Sent on data tranmit error
 *		CMD_GET_LONG_ADDR:											*Get bridge MAC address
 */

/*
 *	Cases:
 *	PC 	| CMD_PING	------> | Bridge
 *		| <------ CMD_START	|
 *		|					|
 *	
 *	PC 	| CMD_DATA	------------------> | Bridge
 *		| <-- CMD_SUCCESS or CMD_ERROR	|
 *		|								|
 *	
 *	PC 	| CMD_SET_*	------------------> | Bridge
 *		| <--- CMD_SET_* (Confirmation)	|
 *		|								|
 *
 *	On Data reception:
 * 	PC 	| <------ CMD_DATA 	| Bridge
 *		|					|
 *
 *	On Bridge Startup:
 *	PC 	| <------ CMD_START	| Bridge
 *		|					|
 *
 */

#define ADDRESS 0x2
#define CHANNEL 0X1A
#define PAN 0xCA5A
#define PROMISCUOUS false

#define ERROR_LED 13

void fatalError()
{
	pinMode(ERROR_LED, OUTPUT);
	for(;;)
	{
		digitalWrite(ERROR_LED, HIGH);
		delay(200);
		digitalWrite(ERROR_LED, LOW);
		delay(200);
	}
}

void setup()
{
	if(!Bridge_init(ADDRESS, CHANNEL, PAN, PROMISCUOUS)) fatalError();
}

void loop()
{
	Bridge_loop();
}