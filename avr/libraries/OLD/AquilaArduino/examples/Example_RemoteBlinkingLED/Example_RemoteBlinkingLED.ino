#include <Wire.h>
#include <Aquila.h>

/*
  Example_RemoteBlinkingLED
  In this example we will be able to
  turn on and off Altair's internal red
  LED.
*/

// Altair's internal red LED
#define LED 13

// Actions: turn on and off the LED
bool turnOff(uint8_t param, bool gotParam)
{
  digitalWrite(LED, HIGH);
}

bool turnOn(uint8_t param, bool gotParam)
{
    digitalWrite(LED, LOW);
}

void setup()
{
  // Setup the LED
  pinMode(LED, OUTPUT);
  // Aquila protocol setup
  Aquila.begin();
  Aquila.setClass("mx.makerlab.blink");
  Aquila.setName("Blink");
  // Registering actions
  Aquila.addAction("Off", turnOff);
  Aquila.addAction("On", turnOn);
  // Anouncing device
  Aquila.sendClass(BROADCAST);
}

void loop()
{
  // Attending requests
  Aquila.loop();
}


