#ifndef PROTOCOLARDUINO_H
#define PROTOCOLARDUINO_H

#include <stdint.h>
#include <stdbool.h>
#include "stack/Protocol.h"

#define BROADCAST (uint8_t*)Phy_broadcast

/*
	Protocol wrapper class for arduino style programming.
*/

class AquilaProtocol
{

private:
	Protocol protocol;

public:
	/*
		Main functions:
	*/
	void begin();

	void loop();

	void setPAN(uint16_t pan);

	void setClass(char *nid);

	void setName(char *nName);

	void setAction(uint8_t n, char description[], bool (*function)(uint8_t param, bool gotParam));

	void addAction(char description[], bool (*function)(uint8_t param, bool gotParam));

	void setEvent(uint8_t n, char description[]);

	Event addEvent(char description[]);

	void emit(uint8_t event, uint8_t param=0, bool hasParam=false);

	void sendClass(uint8_t *address);

	/*
		Advanced functions:
	*/

	void send(uint8_t *address, char *command, uint8_t size);

	void requestAction(uint8_t *address, uint8_t action, uint8_t param=0, bool hasParam=false);

	bool doAction(uint8_t action, uint8_t param, bool gotParam);

};

extern AquilaProtocol Aquila;

#endif //PROTOCOLARDUINO_H