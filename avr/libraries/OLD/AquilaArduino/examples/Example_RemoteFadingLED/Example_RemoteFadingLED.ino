#include <Wire.h>
#include <Aquila.h>

/*
  Example_RemoteFadingLED
  In this example you will be able
  to change the red LED intensity from
  the Hub.
*/
// Internal red LED
#define LED 13

// Actions: turn off and turn on with param (intensity)
bool turnOff(uint8_t param, bool gotParam)
{
  digitalWrite(LED, HIGH);
}

bool turnOn(uint8_t param, bool gotParam)
{
  if(gotParam)
  {
    analogWrite(LED, param);
  }
  else
  {
    digitalWrite(LED, LOW);
  }
}

void setup()
{
  pinMode(LED, OUTPUT);
  
  Aquila.begin();
  Aquila.setClass("mx.makerlab.fade");
  Aquila.setName("Fade");
  
  Aquila.addAction("Off", turnOff);
  Aquila.addAction("Turn On with intensity", turnOn);
  
  Aquila.sendClass(BROADCAST);
}

void loop()
{
  Aquila.loop();
}


