#include <Wire.h>
#include <Mesh.h>
#include <AquilaProtocol.h>

#define ADDR 1

Event off;
Event on;

bool turnOff(uint8_t param, bool gotParam)
{
  Aquila.emit(off);
}

bool turnOn(uint8_t param, bool gotParam)
{
  Aquila.emit(on);
}

void setup()
{
  Mesh.begin(ADDR);
  Aquila.begin();
  Aquila.setClass("mx.makerlab.test");
  Aquila.setName("Test");
  
  Aquila.addAction("Off", turnOff);
  Aquila.addAction("On", turnOn);
  
  off = Aquila.addEvent("Turned Off");
  on = Aquila.addEvent("Turned On");
  
  Aquila.announce(BROADCAST);
}

void loop()
{
  Mesh.loop();
  Aquila.loop();
}


