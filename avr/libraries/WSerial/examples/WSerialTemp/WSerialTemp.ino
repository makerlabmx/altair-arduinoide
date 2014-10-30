#include <Arduino.h>
#include "lwm.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
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
	WSerial.begin(DEST);
	NWK_SetAddr(ADDR);
	NWK_SetPanId(0xCA5A);
	PHY_SetChannel(0x1A);

	timer.setInterval(500, sendTemp);
}

void loop()
{
	SYS_TaskHandler();
	WSerial.loop();
	timer.run();
}