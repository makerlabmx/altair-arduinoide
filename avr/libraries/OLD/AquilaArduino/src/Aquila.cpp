#include "Aquila.h"

void AquilaProtocol::begin()
{
	Protocol_init(&protocol);
}

/*
	Main functions:
*/
void AquilaProtocol::loop()
{
	Protocol_loop(&protocol);
}

void AquilaProtocol::setPAN(uint16_t pan)
{
	Protocol_setPAN(&protocol, pan);
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
	Protocol_triggerEvent(&protocol, event, param, hasParam);
}

void AquilaProtocol::sendClass(uint8_t *address)
{
	Protocol_sendId(&protocol, address);
}

/*
	Advanced functions:
*/

void AquilaProtocol::send(uint8_t *address, char *command, uint8_t size)
{
	Protocol_send(&protocol, address, command, size);
}

void AquilaProtocol::requestAction(uint8_t *address, uint8_t action, uint8_t param, bool hasParam)
{
	Protocol_requestAction(&protocol, address, action, param, hasParam);
}

bool AquilaProtocol::doAction(uint8_t action, uint8_t param, bool gotParam)
{
	return Protocol_doAction(&protocol, action, param, gotParam);
}

AquilaProtocol Aquila;