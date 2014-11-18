#include <Wire.h>
#include <Mesh.h>
#include <SimpleTimer.h>


/*
	Mesh Ping Example.
	Mesh library is based on atmel-lwm, with added address auto assignment.
	For more details, check Atmel-42028-Lightweight-Mesh-Developer-Guide_Application-Note_AVR2130

	Instructions:
	For this example, you need two Altairs.
		1. Upload this sketch to the first Altair
		2. Swap LOCAL_ADDRESS and DESTINATION_ADDRESS
		3. Upload to the second Altair
		4. Power both Altairs
		5. Open the Serial monitor, now you should see the ping messages.
*/

#define LOCAL_ADDRESS 1
#define DESTINATION_ADDRESS 2

// Status led, will toggle with every message received
#define LED 14
bool ledState = 0;

// For sending periodical pings
SimpleTimer timer;

byte pingCounter = 0;

static bool receiveMessage(NWK_DataInd_t *ind) 
{
	// Toggle LED
	ledState = !ledState;
	digitalWrite(LED, ledState);

	Serial.print("Received message - From: ");
	Serial.print(ind->srcAddr, HEX);
	Serial.print("  lqi: ");
	Serial.print(ind->lqi, DEC);

	Serial.print("  rssi: ");
	Serial.print(ind->rssi, DEC);

	Serial.print("  ping: ");
	byte pingCounter = (byte)*(ind->data);
	Serial.println(pingCounter);
	return true;
}

static void pingConfirm(NWK_DataReq_t *req)
{
	// Here, you could check req->status
	// For knowing if the packet was succesfully sent.
}

static void sendMessage(void)
{
	pingCounter++;
	Serial.print("pinging address ");
	Serial.println(DESTINATION_ADDRESS);

	// forming packet
	static NWK_DataReq_t packet;
	packet.dstAddr = DESTINATION_ADDRESS;
	packet.dstEndpoint = 1;
	packet.srcEndpoint = 1;
	packet.options = 0;
	packet.data = &pingCounter;
	packet.size = sizeof(pingCounter);
	packet.confirm = pingConfirm;
	NWK_DataReq(&packet);
}

void setup()
{
	pinMode(LED, OUTPUT);
	Serial.begin(9600);

	// If you call begin without address, it will automatically assign one.
	Mesh.begin(LOCAL_ADDRESS);
	// The following 3 instructions are not needed, Just showing all Mesh functions with their default settings:
	Mesh.setAddr(LOCAL_ADDRESS);
	Mesh.setPanId(0xCA5A);
	Mesh.setChannel(0x1A);

	// Get and print local address:
	Serial.print("Local Address: ");
	Serial.println(Mesh.getShortAddr());

	// Open an endpoint, something like the Ports in TCP/IP
	// You can use endpoints 0 to 7
	Mesh.openEndpoint(1, receiveMessage);

	// Ping every second
	timer.setInterval(1000, sendMessage);
}

void loop()
{
	Mesh.loop();
	timer.run();
}