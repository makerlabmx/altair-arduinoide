#include <Wire.h>
#include <Mesh.h>
#include <AquilaServices.h>
#include <AltairTemperature.h>

#define ADDR 7
#define DEST 8

bool tempService(uint16_t reqAddr, uint8_t method, uint8_t *data, uint8_t dataSize)
{
	Serial.println("temperature service running");
	uint8_t temp = getTempC();
	Services.response(reqAddr, R200, &temp, 1);
}

void setup()
{
	Serial.begin(9600);

	Mesh.begin(ADDR);
	Services.begin();

	Services.add("temperature", tempService);

	Serial.println("Temperature Service starting...");
}

void loop()
{
	Mesh.loop();
	Services.loop();
}