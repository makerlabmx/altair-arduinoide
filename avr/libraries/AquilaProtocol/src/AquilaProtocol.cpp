#include "Arduino.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "Mesh.h"
#include "AquilaProtocol.h"

int AquilaProtocol::begin()
{
	return Protocol_init(&protocol);
}

/*
	Main functions:
*/
void AquilaProtocol::loop()
{
	Protocol_loop(&protocol);
}

void AquilaProtocol::setClass(char *nid)
{
	Protocol_setId(&protocol, nid);
}

void AquilaProtocol::setName(char *nName)
{
	Protocol_setName(&protocol, nName);
}

void AquilaProtocol::setAction(uint8_t n, char description[], bool (*function)(uint8_t param, bool gotParam))
{
	Protocol_setAction(&protocol, n, description, function);
}

void AquilaProtocol::addAction(char description[], bool (*function)(uint8_t param, bool gotParam))
{
	Protocol_addAction(&protocol, description, function);
}

void AquilaProtocol::setEvent(uint8_t n, char description[])
{
	Protocol_setEvent(&protocol, n, description);
}

Event AquilaProtocol::addEvent(char description[])
{
	return Protocol_addEvent(&protocol, description);
}

void AquilaProtocol::emit(uint8_t event, uint8_t param, bool hasParam)
{
	Protocol_emitEvent(&protocol, event, param, hasParam);
}

void AquilaProtocol::announce(uint16_t address)
{
	Protocol_sendEUI(&protocol, address);
}

/*
	Advanced functions:
*/

void AquilaProtocol::requestAction(uint16_t address, uint8_t action, uint8_t param, bool hasParam)
{
	Protocol_requestAction(&protocol, address, action, param, hasParam);
}

bool AquilaProtocol::doAction(uint8_t action, uint8_t param, bool gotParam)
{
	return Protocol_doAction(&protocol, action, param, gotParam);
}

void AquilaProtocol::getEUIAddress(uint8_t* address)
{
	memcpy(address, protocol.EUIAddress, PROTOCOL_EUIADDRESSLEN);
}

AquilaProtocol Aquila;