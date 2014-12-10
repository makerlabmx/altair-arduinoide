#include <Wire.h>
#include "Mesh.h"
#include "WSerial.h"
#include <AltairTemperature.h>
#include <SimpleTimer.h>

#define ADDR 1
#define DEST 2

SimpleTimer timer;

void sendTemp()
{
	float temp = getTempC();
	WSerial.println(temp);
}

void setup()
{
	Mesh.begin(ADDR);
	WSerial.begin(DEST);

	timer.setInterval(500, sendTemp);
}

void loop()
{
	Mesh.loop();
	WSerial.loop();
	timer.run();
}
