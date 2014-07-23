#include <Wire.h>
#include <Aquila.h>

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
  Aquila.begin();
  Aquila.setClass("mx.makerlab.test");
  Aquila.setName("Test");
  
  Aquila.addAction("Off", turnOff);
  Aquila.addAction("On", turnOn);
  
  off = Aquila.addEvent("Turned Off");
  on = Aquila.addEvent("Turned On");
  
  Aquila.sendClass(BROADCAST);
}

void loop()
{
  Aquila.loop();
}


