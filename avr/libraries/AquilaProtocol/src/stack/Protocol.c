#include "Protocol.h"

static NWK_DataInd_t receivedPacket;
static uint8_t receivedData[MAX_DATA_SIZE];
static uint8_t flagGotPacket;

static bool Protocol__rxHandler(NWK_DataInd_t *ind)
{
	receivedPacket.srcAddr = ind->srcAddr;
	receivedPacket.size = ind->size;
	for(int i = 0; i < ind->size; i++)
	{
		receivedData[i] = ind->data[i];
	}
	receivedPacket.data = receivedData;
	flagGotPacket++;
}

int Protocol_init(Protocol *self)
{
	flagGotPacket = 0;

	// Get EUI Long Address from id chip
	ID_init();
    if( !ID_getId(self->EUIAddress) ) return -1;        //Error

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

	NWK_OpenEndpoint(PROTOCOL_ENDPOINT, Protocol__rxHandler);
	PHY_SetRxState(true);

}

void Protocol_loop(Protocol *self)
{
	if(flagGotPacket > 0) Protocol__parsePacket(self);
}

void Protocol__parsePacket(Protocol *self)
{
	//flagGotPacket--;
	flagGotPacket = 0;

	uint16_t sAddr = receivedPacket.srcAddr;

	uint8_t dataLen = receivedPacket.size;
	char *data = (char*)receivedPacket.data;

	if(dataLen < 2) { Protocol_sendNack(self, sAddr); return; }
	
	uint8_t pktVersion = data[0];

	if(pktVersion != PROTOCOL_VERSION) { Protocol_sendNack(self, sAddr); return; }	//if protocol version is diferent, nack and do nothing.

	uint8_t pktControl = data[1];
	uint8_t commandType = pktControl & 0b00000111;
	bool hasParam = (bool) (pktControl>>PROTOCOL_HAS_PARAM & 0x01);
	bool hasData = (bool) (pktControl>>PROTOCOL_HAS_DATA & 0x01);

	uint8_t command, param = 0, index;
	uint8_t EUIAddress[PROTOCOL_EUIADDRESSLEN];
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
				case PROTOCOL_COM_EUI:
					Protocol_sendEUI(self, sAddr);
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
			if(dataLen < 3 + hasParam + PROTOCOL_EUIADDRESSLEN) { return; }
			index = 2;
			command = data[index++];
			if(hasParam) param = data[index++];
			// Get EUI Address
			memcpy(EUIAddress, &data[index], PROTOCOL_EUIADDRESSLEN);	// TODO: test this
			Protocol_checkEvent(self, EUIAddress, command, param, hasParam);
			return;
			break;

		default:
			Protocol_sendNack(self, sAddr);
			return;
	}


}

bool protocolDataReqBusy = false;

static void Protocol__DataCb(NWK_DataReq_t *req)
{
  protocolDataReqBusy = false;
  (void)req;
}

void Protocol_send(Protocol *self, uint16_t address, char *command, uint8_t size)
{
	// lose packet... check alternative TODO
	if(protocolDataReqBusy) return;

	char data[MAX_DATA_SIZE];
	data[0] = PROTOCOL_VERSION;		//appends protocol version as first byte
	memcpy(&data[1], command, size);

	static NWK_DataReq_t packet;	// Maybe make it static?
	packet.dstAddr = address;
	packet.dstEndpoint = PROTOCOL_ENDPOINT;
	packet.srcEndpoint = PROTOCOL_ENDPOINT;
	packet.options = 0;				// TODO: Check options, check if its ok for broadcast, or we need option...
	packet.data = data;
	packet.size = (size + 1);
	packet.confirm = Protocol__DataCb;
	NWK_DataReq(&packet);

	protocolDataReqBusy = true;
}

void Protocol_emitEvent(Protocol *self, uint8_t event, uint8_t param, bool hasParam)
{
	char packet[3 + PROTOCOL_EUIADDRESSLEN];
	uint8_t index = 0;
	//control byte:
	packet[index++] = PROTOCOL_EVENT<<PROTOCOL_COMMAND_TYPE | hasParam<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[index++] = event;
	//param byte:
	if(hasParam) packet[index++] = param;
	//Append EUIAddress, as unique identifier for event configuration
	memcpy(&packet[index], self->EUIAddress, PROTOCOL_EUIADDRESSLEN);

	// Send event to everyone (Almost constant time)
	Protocol_send(self, BROADCAST, packet, index + PROTOCOL_EUIADDRESSLEN);
	// Check if we are subscribed to our own event (slows down with more entries)
	Protocol_checkEvent(self, self->EUIAddress, event, param, hasParam);
}

void Protocol_requestAction(Protocol *self, uint16_t address, uint8_t action, uint8_t param, bool hasParam)
{
	int i;
	bool forUs = (nwkIb.addr == address);

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

void Protocol_sendAck(Protocol *self, uint16_t address)
{
	char packet[1];
	//control byte:
	packet[0] = PROTOCOL_ACK<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 0<<PROTOCOL_HAS_DATA;
	Protocol_send(self, address, packet, 1);
}

void Protocol_sendNack(Protocol *self, uint16_t address)
{
	char packet[1];
	//control byte:
	packet[0] = PROTOCOL_NACK<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 0<<PROTOCOL_HAS_DATA;
	Protocol_send(self, address, packet, 1);
}

void Protocol_sendNActions(Protocol *self, uint16_t address)
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

void Protocol_sendNEvents(Protocol *self, uint16_t address)
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

void Protocol_sendId(Protocol *self, uint16_t address)
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

void Protocol_sendName(Protocol *self, uint16_t address)
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

void Protocol_sendEUI(Protocol *self, uint16_t address)
{
	char packet[2 + PROTOCOL_EUIADDRESSLEN];
	//control byte;
	packet[0] = PROTOCOL_POST<<PROTOCOL_COMMAND_TYPE | 0<<PROTOCOL_HAS_PARAM | 1<<PROTOCOL_HAS_DATA;
	//command byte:
	packet[1] = PROTOCOL_COM_EUI;
	//data:
	memcpy(&packet[2], self->EUIAddress, PROTOCOL_EUIADDRESSLEN);

	Protocol_send(self, address, packet, 2 + PROTOCOL_EUIADDRESSLEN);
}


void Protocol_sendSize(Protocol *self, uint16_t address)
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

void Protocol_sendNEntries(Protocol *self, uint16_t address)
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

    if(address != BROADCAST) Protocol_send(self, address, packet, 3);

}

void Protocol_sendEntry(Protocol *self, uint8_t nEntry, uint16_t address)
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
    
    if(address != BROADCAST) Protocol_send(self, address, packet, 3 + PROTOCOL_ENTRYLEN);
}

void Protocol_clearEntries(Protocol *self, uint16_t address)
{
	// cleans database header.
	PDB_format();
	Protocol_sendAck(self, address);
}

void Protocol_addEntry(Protocol *self, Entry *entry, uint16_t address)
{
	if(PDB_appendEntry(entry)) Protocol_sendAck(self, address);
	else Protocol_sendNack(self, address);
}

void Protocol_delEntry(Protocol *self, uint8_t nEntry, uint16_t address)
{
	if(PDB_delEntry(nEntry)) Protocol_sendAck(self, address);
	else Protocol_sendNack(self, address);
}

void Protocol_setEntry(Protocol *self, uint8_t nEntry, Entry *entry, uint16_t address)
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

void Protocol_sendAction(Protocol *self, uint8_t n, uint16_t address)
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

void Protocol_sendEvent(Protocol *self, uint8_t n, uint16_t address)
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

void Protocol_checkEvent(Protocol *self, uint8_t *EUIAddress, uint8_t event, uint8_t param, uint8_t hasParam)
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
			if(Protocol_EUIAddressEquals(EUIAddress, entry.address))
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

bool Protocol_EUIAddressEquals(uint8_t *addr1, uint8_t *addr2)
{
	int i = 0;
	for(i = 0; i < 8; i++)
	{
		if(addr1[i] != addr2[i]) return false;
	}
	return true;
}
