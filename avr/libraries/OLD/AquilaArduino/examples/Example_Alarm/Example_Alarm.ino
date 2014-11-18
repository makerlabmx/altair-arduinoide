#include <Wire.h>
#include <Aquila.h>

/*
  Example_Alarm
  Parts needed: buzzer
  In this example, the Altair will work as an
  alarm, that can be activated with the hub
  or via interactions with other devices.
*/

// We have a buzzer connected to pin 9
#define BUZZER 9
// Flag for knowing if the alarm should be sonding
bool alarmSounding = false;

// Action for stopping the alarm 
bool stopAlarm(uint8_t param, bool gotParam)
{
  alarmSounding = false;
}
// Action for making the alarm sound
bool soundAlarm(uint8_t param, bool gotParam)
{
  alarmSounding = true;
}

void setup()
{
  // Setting up the buzzer
  pinMode(BUZZER, OUTPUT);
  // Usual Aquila protocol startup
  Aquila.begin();
  Aquila.setClass("mx.makerlab.alarm");
  Aquila.setName("Alarm");
  // Registering actions
  Aquila.addAction("Stop Alarm", stopAlarm);
  Aquila.addAction("Sound Alarm", soundAlarm);
  // Announcing device
  Aquila.sendClass(BROADCAST);
}

void loop()
{
  // Attending requests
  Aquila.loop();
  
  // make noise
  if(alarmSounding)
  {
    for(int i = 0; i < 100; i++)
    {
      tone(BUZZER, i*10);
      Aquila.loop();
      // We should be careful when using delay with Aquila
      // Dont delay too much or we could lose requests.
      delay(10);
    }
    
  }
  else
  {
    noTone(BUZZER);
  }
}


