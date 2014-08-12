#include "Protocol.h"

volatile uint8_t flagGotPacket;

void Protocol__rxHandler(void)
{
	flagGotPacket++;
}

void Protocol_init(Protocol *self)
{
	flagGotPacket = 0;

	Phy_init(&self->phy);
	PDB_init();
	self->id = PROTOCOL_DEFAULT_ID;
	self->name = PROTOCOL_DEFAULT_NAME;
	self->nActions = 0;
	self->nEvents = 0;
	
	// zeroing actions
	int i;
	for(i = 0; i < PROTOCOL_MAXACTIONS; i++)
	{
		self->actions[i] = NULL;
	}
	// zeroing events
	for(i = 0; i < PROTOCOL_MAXEVENTS; i++)
	{
		self->events[i] = NULL;
	}

	//self->nCustomCommands = 0;

	//Testing, should read from config:
	Phy_setPan(&self->phy, PROTOCOL_DEFAULT_PAN);

	#if PHY_SECURITY_ENABLED == 1
	uint8_t key[] = SEC_KEY;
	Phy_setSecKey(&self->phy, key);
	#endif
	//

	//self->phy.txHandler = &txHandler;
	self->phy.rxHandler = &Protocol__rxHandler;

}

void Protocol_setPAN(Protocol *self, uint16_t pan)
{
	Phy_setPan(&self->phy, pan);
}

void Protocol_loop(Protocol *self)
{
	Phy_loop(&self->phy);
	if(flagGotPacket > 0) Protocol__parsePacket(self);
}

void Protocol__parsePacket(Protocol *self)
{
	//flagGotPacket--;
	flagGotPacket = 0;

	if(self->phy.rxPacket.error) return;	//do nothing on phy layer error.

	uint8_t sAddr[SRC_ADDRESS_SIZE], destAddr[DEST_ADDRESS_SIZE];
	memcpy(sAddr, self->phy.rxPacket.srcAddress, SRC_ADDRESS_SIZE);
	memcpy(destAddr, self->phy.rxPacket.destAddress, DEST_ADDRESS_SIZE);

	uint8_t dataLen = self->phy.rxPacket.dataLen;
	char *data = (char*)self->phy.rxPacket.data;

	if(dataLen < 2) { Protocol_sendNack(self, sAddr); return; }
	
	uint8_t pktVersion = data[0];

	if(pktVersion != PROTOCOL_VERSION) { Protocol_sendNack(self, sAddr); return; }	//if protocol version is diferent, nack and do nothing.

	uint8_t pktControl = data[1];
	uint8_t commandType = pktControl & 0b00000111;
	bool hasParam = (bool) (pktControl>>PROTOCOL_HAS_PARAM & 0x01);
	bool hasData = (bool) (pktControl>>PROTOCOL_HAS_DATA & 0x01);

	uint8_t command, param = 0;
	Entry entry;

	switch(commandType)
	{
		case PROTOCOL_NACK:
			return;
			break;

		case PROTOCOL_ACK:
			return;
			break;

		case PROTOCOL_ACTION:
			if(dataLen < 3 + hasParam) { Protocol_sendNack(self, sAddr); return; }
			command = data[2];
			if(hasParam) param = data[3];
			if(Protocol_doAction(self, command, param, hasParam)) Protocol_sendAck(self, sAddr);
			else Protocol_sendNack(self, sAddr);
			return;
			break;

		case PROTOCOL_GET:
			if(dataLen < 3 + hasParam) { Protocol_sendNack(self, sAddr); return; }
			command = data[2];
			if(hasParam) param = data[3];

			switch(command)
			{

				case PROTOCOL_COM_NACTIONS:
					Protocol_sendNActions(self, sAddr);
					break;
				case PROTOCOL_COM_NEVENTS:
					Protocol_sendNEvents(self, sAddr);
					break;
				case PROTOCOL_COM_ID:
					Protocol_sendId(self, sAddr);
					break;
				case PROTOCOL_COM_SIZE:
					Protocol_sendSize(self, sAddr);
					break;
				case PROTOCOL_COM_NENTRIES:
					Protocol_sendNEntries(self, sAddr);
					break;
				case PROTOCOL_COM_ENTRY:
					if(hasParam) Protocol_sendEntry(self, param, sAddr);
					else Protocol_sendNack(self, sAddr);
					break;
				case PROTOCOL_COM_ACTION:
					if(hasParam) Protocol_sendAction(self, param, sAddr);
					else Protocol_sendNack(self, sAddr);
					break;
				case PROTOCOL_COM_EVENT:
					if(hasParam) Protocol_sendEvent(self, param, sAddr);
					else Protocol_sendNack(self, sAddr);
					break;
				case PROTOCOL_COM_NAME:
					Protocol_sendName(self, sAddr);
					break;
			}
			return;
			break;

		case PROTOCOL_POST:
			if(dataLen < 3 + hasParam) { Protocol_sendNack(self, sAddr); return; }
			command = data[2];
			if(hasParam) param = data[3];

			switch(command)
			{

				case PROTOCOL_COM_CLEAR:
					Protocol_clearEntries(self, sAddr);
					break;
				case PROTOCOL_COM_ENTRY:
					if(hasParam && hasData && dataLen >= (4 + PROTOCOL_ENTRYLEN))
					{
						memcpy(&entry, &data[4], PROTOCOL_ENTRYLEN);
						Protocol_setEntry(self, param, &entry, sAddr);
					}
					else Protocol_sendNack(self, sAddr);
					break;
				case PROTOCOL_COM_ADDENTRY:
					if(hasData && !hasParam && dataLen >= (3 + PROTOCOL_ENTRYLEN))
					{
						memcpy(&entry, &data[3], PROTOCOL_ENTRYLEN);
						Protocol_addEntry(self, &entry, sAddr);
					}
					else Protocol_sendNack(self, sAddr);
					break;
				case PROTOCOL_COM_DELENTRY:
					if(hasParam) Protocol_delEntry(self, param, sAddr);
					else Protocol_sendNack(self, sAddr);
					break;
			}
			return;
			break;

		case PROTOCOL_CUSTOM:
			Protocol_sendNack(self, sAddr);
			return;
			break;

		case PROTOCOL_EVENT:
			if(dataLen < 3 + hasParam) { return; }
			command = data[2];
			if(hasParam) param = data[3];
			Protocol_checkEvent(self, sAddr, command, param, hasParam);
			return;
			break;

		default:
			Protocol_sendNack(self, sAddr);
			return;
	}


}

void Protocol_send(Protocol *self, uint8_t *address, char *command, uint8_t size)
{
	char data[MAX_DATA_SIZE];
	data[0] = PROTOCOL_VERSION;		//appends protocol version as first byte
	memcpy(&data[1], command, size);
	Phy_send(&self->phy, address, (uint8_t *)data, (size + 1));
	//Hal_delay(10); //evita pérdida de datos por lentitud del la lectura de recepcion.
}

void Protocol_triggerEvent(Protocol *self, uint8_t event, uint8_t param, bool hasParam)
{
	char packet[3];
	//control byte:
	packet[0] = PROTOCOL_EVENT<<PROTOCOL_COMMAND_TYPE | hasParam<<PROTOCOL_HAS_PARAM | 0<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[1] = event;
	//param byte:
	if(hasParam) packet[2] = param;

	Protocol_send(self, BROADCAST, packet, 2 + hasParam);
}

void Protocol_requestAction(Protocol *self, uint8_t *address, uint8_t action, uint8_t param, bool hasParam)
{
	int i;
	bool forUs = true;
	for(i = 0; i < PROTOCOL_ADDRESSLEN; i++)
	{
		if(self->phy.localAddress[i] != address[i]) 
		{
			forUs = false;
			break;
		}
	}

	if(forUs)	//checa si el mensaje es para nosotros mismos.
	{
		Protocol_doAction(self, action, param, hasParam);
	}
	else
	{
		char packet[3];
		//control byte:
		packet[0] = PROTOCOL_ACTION<<PROTOCOL_COMMAND_TYPE | hasParam<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
		//command byte:
		packet[1] = action;
		//param byte:
		if(hasParam) packet[2] = param;
    
	    Protocol_send(self, address, packet, 2 + hasParam);
	}
    
}

bool Protocol_doAction(Protocol *self, uint8_t action, uint8_t param, bool gotParam)
{
	if(action < PROTOCOL_MAXACTIONS)
	{
		if(self->actions[action] != NULL)
		{
			return self->actions[action]->function(param, gotParam);
		}
	}

	return false;
}

void Protocol_setId(Protocol *self, char *nid)
{
	self->id = nid;
}

void Protocol_setName(Protocol *self, char *nName)
{
	self->name = nName;
}

void Protocol_sendAck(Protocol *self, uint8_t *address)
{
	char packet[1];
	//control byte:
	packet[0] = PROTOCOL_ACK<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 0<<PROTOCOL_HAS_DATA;
	Protocol_send(self, address, packet, 1);
}

void Protocol_sendNack(Protocol *self, uint8_t *address)
{
	char packet[1];
	//control byte:
	packet[0] = PROTOCOL_NACK<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 0<<PROTOCOL_HAS_DATA;
	Protocol_send(self, address, packet, 1);
}

void Protocol_sendNActions(Protocol *self, uint8_t *address)
{
	char packet[3];
	//control byte:
	packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[1] = PROTOCOL_COM_NACTIONS;
	//data byte:
	packet[2] = self->nActions;

    Protocol_send(self, address, packet, 3);
}

void Protocol_sendNEvents(Protocol *self, uint8_t *address)
{
	char packet[3];
	//control byte:
	packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[1] = PROTOCOL_COM_NEVENTS;
	//data byte:
	packet[2] = self->nEvents;

    Protocol_send(self, address, packet, 3);
}

void Protocol_sendId(Protocol *self, uint8_t *address)
{
	uint8_t idLen = strlen(self->id);

	char packet[MAX_DATA_SIZE];
	//control byte:
	packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[1] = PROTOCOL_COM_ID;
	//data:
	memcpy(&packet[2], self->id, idLen);

    Protocol_send(self, address, packet, 2 + idLen);
}

void Protocol_sendName(Protocol *self, uint8_t *address)
{
	uint8_t nameLen = strlen(self->name);

	char packet[MAX_DATA_SIZE];
	//control byte:
	packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[1] = PROTOCOL_COM_NAME;
	//data:
	memcpy(&packet[2], self->name, nameLen);

    Protocol_send(self, address, packet, 2 + nameLen);
}

void Protocol_sendSize(Protocol *self, uint8_t *address)
{
 	char packet[3];
	//control byte:
	packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[1] = PROTOCOL_COM_SIZE;
	//data byte:
	packet[2] = (char)PROTOCOL_MAXENTRIES;

    Protocol_send(self, address, packet, 3);
}

void Protocol_sendNEntries(Protocol *self, uint8_t *address)
{
	DBHeader header;
	PDB_readHeader(&header);
	int nEntries = header.nEntries;

	if (nEntries > PROTOCOL_MAXENTRIES) nEntries = 0;

	char packet[3];
	//control byte:
	packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[1] = PROTOCOL_COM_NENTRIES;
	//data byte:
	packet[2] = nEntries;

    if(!Phy__isBroadcast(address)) Protocol_send(self, address, packet, 3);

}

void Protocol_sendEntry(Protocol *self, uint8_t nEntry, uint8_t *address)
{
	Entry entry;
	if(!PDB_getEntry(&entry, nEntry)) 
	{
		Protocol_sendNack(self, address);
		return;
	}
	char packet[3 + PROTOCOL_ENTRYLEN];
	//control byte:
	packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 1<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[1] = PROTOCOL_COM_ENTRY;
	//param byte:
	packet[2] = nEntry;
	//data bytes:
	memcpy(&packet[3], &entry, PROTOCOL_ENTRYLEN);
    
    if(!Phy__isBroadcast(address)) Protocol_send(self, address, packet, 3 + PROTOCOL_ENTRYLEN);
}

void Protocol_clearEntries(Protocol *self, uint8_t *address)
{
	// cleans database header.
	PDB_format();
	Protocol_sendAck(self, address);
}

void Protocol_addEntry(Protocol *self, Entry *entry, uint8_t *address)
{
	if(PDB_appendEntry(entry)) Protocol_sendAck(self, address);
	else Protocol_sendNack(self, address);
}

void Protocol_delEntry(Protocol *self, uint8_t nEntry, uint8_t *address)
{
	if(PDB_delEntry(nEntry)) Protocol_sendAck(self, address);
	else Protocol_sendNack(self, address);
}

void Protocol_setEntry(Protocol *self, uint8_t nEntry, Entry *entry, uint8_t *address)
{
	if(PDB_setEntry(entry, nEntry)) Protocol_sendAck(self, address);
	else Protocol_sendNack(self, address);
}

void Protocol_setAction(Protocol *self, uint8_t n, char description[], bool (*function)(uint8_t param, bool gotParam))
{
	if(n < PROTOCOL_MAXACTIONS)
	{
		if(self->actions[n] == NULL) self->nActions++;
		Action *newAction;
		newAction = malloc(sizeof(Action));
		uint8_t size = strlen(description);
		newAction->description = malloc(size + 1);
		strcpy(newAction->description, description);
		newAction->function = function;
		self->actions[n] = newAction;
	}
}

void Protocol_addAction(Protocol *self, char description[], bool (*function)(uint8_t param, bool gotParam))
{
	Protocol_setAction(self, self->nActions, description, function);
}

void Protocol_setEvent(Protocol *self, uint8_t n, char description[])
{
	if(n < PROTOCOL_MAXEVENTS) 
	{
		if(self->events[n] == NULL) self->nEvents++;
		uint8_t size = strlen(description);
		self->events[n] = malloc(size + 1);
		strcpy(self->events[n], description);
	}
}

Event Protocol_addEvent(Protocol *self, char description[])
{
	uint8_t n = self->nEvents;
	Protocol_setEvent(self, n, description);
	return n;
}

/*
void Protocol_setCustomCommand(Protocol *self, char t, char cmd[], uint8_t prmlen, uint8_t dtlen, void (*func)(uint8_t param, char data[], uint8_t *address))
{
	if(self->nCustomCommands < PROTOCOL_MAXCUSTCOMMANDS)
	{
		CustomCommand newCommand;
		newCommand.type = t;
		newCommand.command = cmd;
		newCommand.paramlen = prmlen;
		newCommand.datalen = dtlen;
		newCommand.function = func;
		self->customCommands[self->nCustomCommands] = &newCommand;
		self->nCustomCommands++;
	}
}
*/

void Protocol_sendAction(Protocol *self, uint8_t n, uint8_t *address)
{
	if(n < PROTOCOL_MAXACTIONS)
	{
		uint8_t cont = 0;
		int i;
		for(i = 0; i < PROTOCOL_MAXACTIONS; i++)
		{
			if(self->actions[i] != NULL)
			{
				if(cont == n)
				{

					char packet[MAX_DATA_SIZE];
					//control byte:
					packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 1<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
					//command byte:
					packet[1] = PROTOCOL_COM_ACTION;
					//param byte:
					packet[2] = i;
					//data:
					uint8_t descLen = strlen(self->actions[i]->description);
					memcpy(&packet[3], self->actions[i]->description, descLen);

				    Protocol_send(self, address, packet, 3 + descLen);
					return;
				}
				cont++;
			}
		}
	}
	Protocol_sendNack(self, address);
}

void Protocol_sendEvent(Protocol *self, uint8_t n, uint8_t *address)
{
	if(n < PROTOCOL_MAXEVENTS)
	{
		uint8_t cont = 0;
		int i;
		for(i = 0; i < PROTOCOL_MAXEVENTS; i++)
		{
			if(self->events[i] != NULL)
			{
				if(cont == n)
				{
					char packet[MAX_DATA_SIZE];
					//control byte:
					packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 1<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
					//command byte:
					packet[1] = PROTOCOL_COM_EVENT;
					//param byte:
					packet[2] = i;
					//data:
					uint8_t descLen = strlen(self->events[i]);
					memcpy(&packet[3], self->events[i], descLen);

				    Protocol_send(self, address, packet, 3 + descLen);
					return;
				}
				cont++;
			}
		}
	}
	Protocol_sendNack(self, address);
}

void Protocol_checkEvent(Protocol *self, uint8_t *address, uint8_t event, uint8_t param, uint8_t hasParam)
{
	DBHeader header;
	Entry entry;
	PDB_readHeader(&header);
	uint8_t nEntries = header.nEntries;

	if (nEntries > PROTOCOL_MAXENTRIES) nEntries = 0;  //checa en caso de numero invalido (si no se ha programado nunca)

	PDB_iteratorBegin();
	int i = 0;
	for(i = 0; i < nEntries; i++)
	{
		if(!PDB_iteratorGetNextEntry(&entry)) continue;
		if(entry.event == event)
		{
			if(Protocol_addressEquals(address, entry.address))
			{
				if(hasParam)
				{
					Protocol_doAction(self, entry.action, param, hasParam);
				}
				else
				{
					Protocol_doAction(self, entry.action, entry.param, entry.config.hasParam);
				}
			}
		}
	}
}

bool Protocol_addressEquals(uint8_t *addr1, uint8_t *addr2)
{
	int i = 0;
	for(i = 0; i < 8; i++)
	{
		if(addr1[i] != addr2[i]) return false;
	}
	return true;
}
