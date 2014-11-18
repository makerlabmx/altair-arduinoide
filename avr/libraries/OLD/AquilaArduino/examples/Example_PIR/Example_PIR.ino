#include <Wire.h>
#include <Aquila.h>

#define PIR 18
#define LED 15

int statePIRnow = LOW;
int statePIRbefore = LOW;

Event movement;
Event noMovement;

void setup()
{
  pinMode(PIR, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  Aquila.begin();
  Aquila.setClass("mx.makerlab.movement");

  Aquila.setName("Movement Sensor");
  movement = Aquila.addEvent("Movement detected");
  noMovement = Aquila.addEvent("No movement detected");

  Aquila.sendClass(BROADCAST);
}

void loop()
{
  Aquila.loop();
  
  statePIRnow = digitalRead(PIR);
  if(statePIRnow == HIGH && statePIRbefore == LOW)
  {
    digitalWrite(LED,LOW);
    Aquila.emit(movement);
    statePIRbefore = HIGH;
  }
  else if(statePIRnow == LOW && statePIRbefore == HIGH)
  {
    digitalWrite(LED,HIGH);
    Aquila.emit(noMovement);
    statePIRbefore = LOW;
  }
}
