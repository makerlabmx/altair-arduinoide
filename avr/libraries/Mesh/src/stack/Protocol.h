/**
* \file Protocol.h
*
* \brief Aquila 802.15.4 Mesh implementation.
*
* Copyright (C) 2014, Rodrigo Méndez. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3. The name of Makerlab may not be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* 4. This software may only be redistributed and used in connection with a
*    Makerlab product.
*
* THIS SOFTWARE IS PROVIDED BY MAKERLAB "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
* EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL MAKERLAB BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* Modification and other use of this code is subject to Makerlab's Limited
* License Agreement (license.txt).
*
*/


#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "config.h"

#include <stdint.h>
#include <stdbool.h>
#include "ProtocolDB.h"
#include "halID.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define PROTOCOL_ENDPOINT 13
#define PROTOCOL_VERSION 3
#define PROTOCOL_DEFAULT_ID "mx.makerlab.example"
#define PROTOCOL_DEFAULT_NAME "Example"
#define PROTOCOL_EUIADDRESSLEN 8

#ifndef PROTOCOL_MAXACTIONS
#define PROTOCOL_MAXACTIONS 100
#endif

#ifndef PROTOCOL_MAXEVENTS
#define PROTOCOL_MAXEVENTS 100
#endif

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
#define PROTOCOL_COM_EUI 14

#define BROADCAST 0xFFFF

typedef struct
{
	char *description;
	bool (*function)(uint8_t param, bool gotParam);
} Action;

#define Event uint8_t

typedef struct
{
	char *id;
	char *name;
	uint8_t EUIAddress[PROTOCOL_EUIADDRESSLEN];

	uint8_t nActions;
	uint8_t nEvents;
 	Action *actions[PROTOCOL_MAXACTIONS];
 	char *events[PROTOCOL_MAXEVENTS];

} Protocol;

int Protocol_init(Protocol *self, bool _secEnabled);

void Protocol_setId(Protocol *self, char *nid);

void Protocol_setName(Protocol *self, char *nName);

void Protocol_setAction(Protocol *self, uint8_t n, char description[], bool (*function)(uint8_t param, bool gotParam));

void Protocol_addAction(Protocol *Self, char description[], bool (*function)(uint8_t param, bool gotParam));

void Protocol_setEvent(Protocol *self, uint8_t n, char description[]);

Event Protocol_addEvent(Protocol *self, char description[]);

void Protocol_emitEvent(Protocol *self, uint8_t event, uint8_t param, bool hasParam);


void Protocol_loop(Protocol *self);

void Protocol_send(Protocol *self, uint16_t address, char *command, uint8_t size);

void Protocol_requestAction(Protocol *self, uint16_t address, uint8_t action, uint8_t param, bool hasParam);

bool Protocol_doAction(Protocol *self, uint8_t action, uint8_t param, bool gotParam);


void Protocol__parsePacket(Protocol *self);


void Protocol_sendAck(Protocol *self, uint16_t address);

void Protocol_sendNack(Protocol *self, uint16_t address);

void Protocol_sendNActions(Protocol *self, uint16_t address);

void Protocol_sendNEvents(Protocol *self, uint16_t address);

void Protocol_sendId(Protocol *self, uint16_t address);

void Protocol_sendName(Protocol *self, uint16_t address);

void Protocol_sendEUI(Protocol *self, uint16_t address);


void Protocol_sendSize(Protocol *self, uint16_t address);

void Protocol_sendNEntries(Protocol *self, uint16_t address);

void Protocol_sendEntry(Protocol *self, uint8_t nEntry, uint16_t address);

void Protocol_clearEntries(Protocol *self, uint16_t address);

void Protocol_addEntry(Protocol *self, Entry *entry, uint16_t address);

void Protocol_delEntry(Protocol *self, uint8_t nEntry, uint16_t address);

void Protocol_setEntry(Protocol *self, uint8_t nEntry, Entry *entry, uint16_t address);


void Protocol_sendAction(Protocol *self, uint8_t n, uint16_t address);

void Protocol_sendEvent(Protocol *self, uint8_t n, uint16_t address);

void Protocol_checkEvent(Protocol *self, uint8_t *EUIAddress, uint8_t event, uint8_t param, uint8_t hasParam);

bool Protocol_EUIAddressEquals(uint8_t *addr1, uint8_t *addr2);

#ifdef  __cplusplus
}
#endif

#endif /* PROTOCOL_H */
