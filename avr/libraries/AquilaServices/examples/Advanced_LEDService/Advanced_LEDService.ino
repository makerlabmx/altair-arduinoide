#include <Wire.h>
#include <Mesh.h>
#include <AquilaProtocol.h>
#include <AquilaServices.h>
#include <JsonGenerator.h>
#include <JsonParser.h>

using namespace ArduinoJson;

/*
  LED Service Example:

  This example exposes a service called "led" for consulting and modifying the 
  state of an LED.

  Requesting "GET" in this service will return a JSON with a boolean key called 
  "isOn".

  Requesting "PUT" in this service with a JSON with a boolean key called "isOn"
  will change the state of the LED accordingly and return the new state.

  It also defines Actions for changing the LED state.

  We are using the Arduino Json library for encoding and decoding the data.
  You can get its documentation here: https://github.com/bblanchon/ArduinoJson
*/

#define LED 13
#define BUTTON 33

// Event definitions:
Event buttonPressed;

bool edoLED = false;

double previousPress = 0;
int pressTOut = 1000;

void setLED(bool state)
{
  if(state)
  {
    digitalWrite(LED,LOW);
    edoLED = true;
  }
  else
  {
    digitalWrite(LED,HIGH);
    edoLED = false;
  }
}

// Service definitions:

bool LEDService(uint16_t reqAddr, uint8_t method, char *data, uint8_t dataSize)
{
  if(method == GET)
  {
    Generator::JsonObject<1> json;
    json["isOn"] = edoLED;
    
    // creating a buffer with enough space for the generated json:
    char buffer[16];
    json.printTo(buffer, sizeof(buffer));
    Services.response(reqAddr, R200, buffer, strlen(buffer));
  }
  else if(method == PUT)
  {
    if(dataSize > 0)
    {
      // Parsing request data:
      Parser::JsonParser<32> parser;
      Parser::JsonObject parsedJson = parser.parse(data);
      if(!parsedJson.success()) { Services.response(reqAddr, R500); return false; }
      
      // Updating state:
      if(parsedJson.containsKey("isOn"))
      {
        setLED( (bool)parsedJson["isOn"] ); 
      }
      
      // Returning new state:
      Generator::JsonObject<1> json;
      json["isOn"] = edoLED;
      
      // creating a buffer with enough space for the generated json:
      char buffer[16];
      json.printTo(buffer, sizeof(buffer));
      Services.response(reqAddr, R200, buffer, strlen(buffer));
    }
    else
    {
      Services.response(reqAddr, R500);
    }
  }
  else
  {
    Services.response(reqAddr, R405);
  }
  return true;
}

// Action definitions:
bool LEDOn(uint8_t param, bool gotParam)
{
  setLED(true);
}

bool LEDOff(uint8_t param, bool gotParam)
{
  setLED(false);
}

bool LEDToggle(uint8_t param, bool gotParam)
{
  if (edoLED)
  {
    setLED(false);
  }
  else
  {
    setLED(true);
  }
}

void setup()
{
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);

  pinMode(BUTTON, INPUT);  
  
  // Starting comminication hardware and libraries:
  Mesh.begin();
  Aquila.begin();
  Services.begin();
  
  // Initialize services:
  Services.add("led", LEDService);
  
  // Seting device identification:
  Aquila.setClass("mx.makerlab.ledservice");
  Aquila.setName("Altair Service");
  // Adding Actions:
  Aquila.addAction("Apagar LED", LEDOff);
  Aquila.addAction("Encender LED", LEDOn);
  Aquila.addAction("Cambiar estado LED",LEDToggle);
  // Adding events:
  // Weird symbols are UTF8 values for o with accent
  buttonPressed = Aquila.addEvent("Se presion\xC3\xB3 el Bot\xC3\xB3n");
  // Anouncing to the hub (for auto discovery in the aquila-server ui).
  Mesh.announce(HUB);
}

void loop()
{
  // Constantly listening for requests and tasks:
  Mesh.loop();
  Aquila.loop();
  Services.loop();
  
  double now = millis();
  if(digitalRead(BUTTON) == LOW && now > (previousPress + pressTOut) )
  {
      previousPress = now;
      Aquila.emit(buttonPressed);
  }
  
}
