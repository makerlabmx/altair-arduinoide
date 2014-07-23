#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "config.h"

#include <stdint.h>
#include <stdbool.h>
#include "ProtocolDB.h"
#include "phy.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define PROTOCOL_VERSION 1
#define PROTOCOL_DEFAULT_PAN 0xca5a
#define PROTOCOL_DEFAULT_ID "mx.makerlab.example"
#define PROTOCOL_DEFAULT_NAME "Example"
#define PROTOCOL_ADDRESSLEN DEST_ADDRESS_SIZE

#ifndef PROTOCOL_MAXACTIONS
#define PROTOCOL_MAXACTIONS 100
#endif

#ifndef PROTOCOL_MAXEVENTS
#define PROTOCOL_MAXEVENTS 100
#endif

// Currently not used (not implemented)
#define PROTOCOL_MAXCUSTCOMMANDS 10

//Control byte indexes:
#define PROTOCOL_COMMAND_TYPE 0
#define PROTOCOL_HAS_PARAM 3
#define PROTOCOL_HAS_DATA 4

//Command type definitions:
#define PROTOCOL_NACK 	0x00
#define PROTOCOL_ACK  	0x01
#define PROTOCOL_ACTION 0x02
#define PROTOCOL_GET 	0x03
#define PROTOCOL_POST 	0x04
#define PROTOCOL_CUSTOM 0x05
#define PROTOCOL_EVENT 	0x06

//Command definitions:
#define PROTOCOL_COM_NACTIONS 0
#define PROTOCOL_COM_NEVENTS 1
#define PROTOCOL_COM_ID	2
#define PROTOCOL_COM_SIZE 3
#define PROTOCOL_COM_NENTRIES 4
#define PROTOCOL_COM_ABSENTRY 5
#define PROTOCOL_COM_ENTRY 6
#define PROTOCOL_COM_CLEAR 7
#define PROTOCOL_COM_ADDENTRY 8
#define PROTOCOL_COM_DELABSENTRY 9
#define PROTOCOL_COM_DELENTRY 10
#define PROTOCOL_COM_ACTION 11
#define PROTOCOL_COM_EVENT 12
#define PROTOCOL_COM_NAME 13

#define BROADCAST (uint8_t*)Phy_broadcast

typedef struct
{
	char *description;
	bool (*function)(uint8_t param, bool gotParam);
} Action;

#define Event uint8_t

/*
typedef struct
{
	char type;
	char *command;
	uint8_t paramlen;
	uint8_t datalen;
	void (*function)(uint8_t param, char data[], uint8_t *address);
} CustomCommand;
*/
typedef struct
{
	Phy phy;
	char *id;
	char *name;

	uint8_t nActions;
	uint8_t nEvents;
 	Action *actions[PROTOCOL_MAXACTIONS];
 	char *events[PROTOCOL_MAXEVENTS];
 	//CustomCommand *customCommands[PROTOCOL_MAXCUSTCOMMANDS];
 	//uint8_t nCustomCommands;

} Protocol;

void Protocol_init(Protocol *self);

void Protocol_setId(Protocol *self, char *nid);

void Protocol_setName(Protocol *self, char *nName);

void Protocol_setAction(Protocol *self, uint8_t n, char description[], bool (*function)(uint8_t param, bool gotParam));

void Protocol_addAction(Protocol *Self, char description[], bool (*function)(uint8_t param, bool gotParam));

void Protocol_setEvent(Protocol *self, uint8_t n, char description[]);

Event Protocol_addEvent(Protocol *self, char description[]);

void Protocol_triggerEvent(Protocol *self, uint8_t event, uint8_t param, bool hasParam);


void Protocol_loop(Protocol *self);

void Protocol_send(Protocol *self, uint8_t *address, char *command, uint8_t size);

void Protocol_requestAction(Protocol *self, uint8_t *address, uint8_t action, uint8_t param, bool hasParam);

bool Protocol_doAction(Protocol *self, uint8_t action, uint8_t param, bool gotParam);


void Protocol__rxHandler(void);

void Protocol__parsePacket(Protocol *self);


void Protocol_sendAck(Protocol *self, uint8_t *address);

void Protocol_sendNack(Protocol *self, uint8_t *address);

void Protocol_sendNActions(Protocol *self, uint8_t *address);

void Protocol_sendNEvents(Protocol *self, uint8_t *address);

void Protocol_sendId(Protocol *self, uint8_t *address);

void Protocol_sendName(Protocol *self, uint8_t *address);


void Protocol_sendSize(Protocol *self, uint8_t *address);

void Protocol_sendNEntries(Protocol *self, uint8_t *address);

void Protocol_sendEntry(Protocol *self, uint8_t nEntry, uint8_t *address);

void Protocol_clearEntries(Protocol *self, uint8_t *address);

void Protocol_addEntry(Protocol *self, Entry *entry, uint8_t *address);

void Protocol_delEntry(Protocol *self, uint8_t nEntry, uint8_t *address);

void Protocol_setEntry(Protocol *self, uint8_t nEntry, Entry *entry, uint8_t *address);

//void Protocol_setCustomCommand(Protocol *self, char t, char cmd[], uint8_t prmlen, uint8_t dtlen, void (*func)(uint8_t param, char data[], uint8_t *address));

void Protocol_sendAction(Protocol *self, uint8_t n, uint8_t *address);

void Protocol_sendEvent(Protocol *self, uint8_t n, uint8_t *address);

void Protocol_checkEvent(Protocol *self, uint8_t *address, uint8_t event, uint8_t param, uint8_t hasParam);

bool Protocol_addressEquals(uint8_t *addr1, uint8_t *addr2);

#ifdef  __cplusplus
}
#endif

#endif /* PROTOCOL_H */