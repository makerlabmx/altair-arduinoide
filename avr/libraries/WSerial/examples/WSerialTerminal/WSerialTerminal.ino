#include <Wire.h>
#include "Mesh.h"
#include "WSerial.h"

#define ADDR 2
#define DEST 1

void setup()
{
	Serial.begin(9600);
	Serial.println("Wireless Serial Terminal");
	Mesh.begin(ADDR);
	WSerial.begin(DEST);
}

void loop()
{
	Mesh.loop();
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