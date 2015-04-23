#include <Wire.h>
#include <Mesh.h>
#include <AquilaProtocol.h>
#include <AquilaServices.h>

int count1;
float count2;

void setup()
{
	Mesh.begin();
	Aquila.begin();
	Services.begin();

	count1 = 0;
	count2 = 0.0;

	Services.variable("count1", &count1);
	Services.variable("count2", &count2);

	Aquila.setClass("mx.makerlab.varexample");
	Aquila.setName("Variables");

	Mesh.announce(HUB);
}

void loop()
{
	Mesh.loop();
	Aquila.loop();
	Services.loop();

	count1++;
	count2 += 10.93;
}