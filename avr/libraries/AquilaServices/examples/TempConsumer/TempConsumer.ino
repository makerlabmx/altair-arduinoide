#include <Wire.h>
#include <Mesh.h>
#include <AquilaServices.h>
#include <SimpleTimer.h>

#define ADDR 8
#define DEST 7

SimpleTimer timer;

void tempReqCb(uint16_t srcAddr, uint8_t status, uint8_t *data, uint8_t dataSize)
{
	if(status != R200) { Serial.print("Request Error Status: "); Serial.println(status, HEX); return;}
	if(dataSize > 0) Serial.println(*data);
	else Serial.println("Got null data...");
}

void getTemp()
{
	Serial.println("Making Request...");
	Services.request(DEST, GET, "temperature", tempReqCb);
}

void setup()
{
	Serial.begin(9600);
	Mesh.begin(ADDR);
	Services.begin();

	timer.setInterval(1000, getTemp);

	Serial.println("Temp Consumer starting...");
}

void loop()
{
	Mesh.loop();
	Services.loop();
	timer.run();
}