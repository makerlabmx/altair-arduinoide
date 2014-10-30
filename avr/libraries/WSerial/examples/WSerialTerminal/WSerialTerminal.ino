#include <Arduino.h>
#include "lwm.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "WSerial.h"

#define ADDR 2
#define DEST 1

void setup()
{
	Serial.begin(9600);
	Serial.println("Wireless Serial Terminal");
	WSerial.begin(DEST);
	NWK_SetAddr(ADDR);
	NWK_SetPanId(0xCA5A);
	PHY_SetChannel(0x1A);
}

void loop()
{
	SYS_TaskHandler();
	WSerial.loop();
	while(WSerial.available())
	{
		Serial.write(WSerial.read());
	}
	while(Serial.available())
	{
		WSerial.write(Serial.read());
	}
}