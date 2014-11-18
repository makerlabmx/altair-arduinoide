#ifndef AQUILAPROTOCOL_H
#define AQUILAPROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <Mesh.h>
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "stack/Protocol.h"

class AquilaProtocol
{
private:
	Protocol protocol;

public:
	/*
		Main functions:
	*/
	int begin();

	void loop();

	void setClass(char *nid);

	void setName(char *nName);

	void setAction(uint8_t n, char description[], bool (*function)(uint8_t param, bool gotParam));

	void addAction(char description[], bool (*function)(uint8_t param, bool gotParam));

	void setEvent(uint8_t n, char description[]);

	Event addEvent(char description[]);

	void emit(uint8_t event, uint8_t param=0, bool hasParam=false);

	void announce(uint16_t address);

	/*
		Advanced functions:
	*/

	void requestAction(uint16_t address, uint8_t action, uint8_t param=0, bool hasParam=false);

	bool doAction(uint8_t action, uint8_t param, bool gotParam);

	void getEUIAddress(uint8_t* address);
};

extern AquilaProtocol Aquila;

#endif //AQUILAPROTOCOL_H