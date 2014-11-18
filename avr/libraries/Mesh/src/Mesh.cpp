#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "Arduino.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "Mesh.h"

//#define MESH_DEBUG

#ifdef MESH_DEBUG
#include "stack/halSerial.h"
#endif

#define MESH_ADDRENDPOINT 15

// Based on version from: http://www.gaw.ru/pdf/Atmel/app/avr/AVR350.pdf
uint16_t calcrc(uint8_t *ptr, int count)
{
	int crc;
	char i;
	crc = 0;
	while (--count >= 0)
	{
		crc = crc ^ (int) *ptr++ << 8;
		i = 8;
		do
		{
			if (crc & 0x8000)
			crc = crc << 1 ^ 0x1021;
			else
			crc = crc << 1;
		} while(--i);
	}
	return (crc);
}

static uint16_t shortAddr = 0;
static uint16_t genAddr = 0;
static bool waitingAddrConfirm = false;
static bool addrAssignSuccess = false;

static bool addrCb(NWK_DataInd_t *ind)
{
	// send ack
	return true;
}

static void pingCb(NWK_DataReq_t *req)
{
	#ifdef MESH_DEBUG
	printf("pingCb, status: %x\n", req->status);
	#endif
	waitingAddrConfirm = false;

	if(req->status == NWK_SUCCESS_STATUS)
	{
		// Got Ack, means someone else has the address
		addrAssignSuccess = false;
	}
	else
	{	// If didnt get ack, means the address is free, use it.
		addrAssignSuccess = true;
		shortAddr = genAddr;
		NWK_SetAddr(shortAddr);
		#ifdef MESH_DEBUG
		printf("%x\n", genAddr);
		#endif
	}

	(void)req;
}

// Begin with automatic address
void AquilaMesh::begin()
{
	#ifdef MESH_DEBUG
	Serial_init();
	#endif
	uint8_t id[8];
	ID_init();
	ID_getId(id);

	genAddr = calcrc(id, 8);
	// Check if we are not in manual address range, and fix.
	if(genAddr < 256) genAddr += 256;

	//test collision:
	//genAddr = 3;

	// Init as address 0:
	begin(0);

	while(!addrAssignSuccess)
	{
		// Send a ping packet to our generated address to check if its already taken
		static NWK_DataReq_t packet;
		packet.dstAddr = genAddr;
		packet.dstEndpoint = MESH_ADDRENDPOINT;
		packet.srcEndpoint = MESH_ADDRENDPOINT;
		packet.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;	// Don't know why, but without NWK_OPT_ENABLE_SECURITY, it doesn't work
		packet.data = NULL;
		packet.size = 0;
		packet.confirm = pingCb;
		NWK_DataReq(&packet);

		waitingAddrConfirm = true;

		while(waitingAddrConfirm)
		{
			SYS_TaskHandler();
		}

		// If address already in use, try with another one
		if(!addrAssignSuccess) genAddr += 1 + id[7];
	}
}

// Begin with manual address addr
void AquilaMesh::begin(uint16_t addr)
{
	SYS_Init();
	PHY_SetRxState(true);
	shortAddr = addr;
	NWK_SetAddr(addr);
	NWK_SetPanId(AQUILAMESH_DEFPAN);
	PHY_SetChannel(AQUILAMESH_DEFCHAN);

	NWK_OpenEndpoint(MESH_ADDRENDPOINT, addrCb);
}

void AquilaMesh::end()
{
	PHY_SetRxState(false);
}

void AquilaMesh::loop()
{
	SYS_TaskHandler();
}

void AquilaMesh::setAddr(uint16_t addr)
{
	NWK_SetAddr(addr);
}

void AquilaMesh::setPanId(uint16_t panId)
{
	NWK_SetPanId(panId);
}

void AquilaMesh::setChannel(uint8_t channel)
{
	PHY_SetChannel(channel);
}

// Leave this as advanced and require direct NWK_... calling??
// otherwhise we should redefine the Reqs etc... 
void AquilaMesh::openEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind))
{
	NWK_OpenEndpoint(id, handler);
}

uint16_t AquilaMesh::getShortAddr()
{
	return shortAddr;
}

/*void AquilaMesh::getEUIAddress(uint8_t* address)
{
	
}*/

AquilaMesh Mesh;