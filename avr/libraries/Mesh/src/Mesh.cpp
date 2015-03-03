/**
* \file Mesh.cpp
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


#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "Arduino.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "lwm/nwk/nwkSecurity.h"
#include "stack/halID.h"
#include "Mesh.h"

//#define MESH_DEBUG

#ifdef MESH_DEBUG
#include "stack/halSerial.h"
#endif

#define MESH_ADDRENDPOINT 15

/* --------------------------- Address collision ------------------------*/
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

static void pingCb(TxPacket *req)
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

/* --------------------------- /Address collision ------------------------*/

#define MESH_CMD_GETEUI 0
#define MESH_CMD_RESEUI 1

static bool secEnabled;
static bool euiBussy = false;
static uint8_t euiAddr[8];
static TxPacket packet;

static void euiCb(TxPacket *req)
{
	euiBussy = false;
}

void sendEUI(uint16_t dest)
{
	if(euiBussy) return;

	static uint8_t data[9];
	data[0] = MESH_CMD_RESEUI;

	// copy eui
	memcpy(&data[1], euiAddr, 8);

	packet.dstAddr = dest;
	packet.dstEndpoint = MESH_ADDRENDPOINT;
	packet.srcEndpoint = MESH_ADDRENDPOINT;
	if(secEnabled)
		packet.options = NWK_OPT_ENABLE_SECURITY;
	else
		packet.options = 0;
	packet.data = data;
	packet.size = 9;
	packet.confirm = euiCb;

	euiBussy = true;
	NWK_DataReq(&packet);
}

static bool meshCb(RxPacket *ind)
{
	// check if address collision packet
	if(ind->size == 0)	// address collision packet
	{
		// send ack
		return true;
	}
	else 	// size at least 1
	{
		// if security enabled and the package was not secured, ignore it.
		if( secEnabled && !(ind->options & NWK_IND_OPT_SECURED) ) return false;

		if(ind->data[0] == MESH_CMD_GETEUI)
		{
			sendEUI(ind->srcAddr);
		}
	}
	return true;
}

AquilaMesh::AquilaMesh()
{
	secEnabled = false;
	isAsleep = false;
}

// Begin with automatic address
void AquilaMesh::begin()
{
	#ifdef MESH_DEBUG
	Serial_init();
	#endif
	// Init as address 0:
	begin(0);

	uint8_t id[8];
	// TODO: Should check for error on EUI read and do something different.
	getEUIAddr(id);

	genAddr = calcrc(id, 8);
	// Check if we are not in manual address range, and fix.
	if(genAddr < 256) genAddr += 256;

	//test collision:
	//genAddr = 3;

	while(!addrAssignSuccess)
	{
		// Send a ping packet to our generated address to check if its already taken
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

	ID_init();
	if( !ID_getId(euiAddr) );        //Error

	NWK_OpenEndpoint(MESH_ADDRENDPOINT, meshCb);
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

void AquilaMesh::setSecurityKey(uint8_t *key)
{
	NWK_SetSecurityKey(key);
}

void AquilaMesh::setSecurityEnabled(bool enabled)
{
	secEnabled = enabled;
}

bool AquilaMesh::getSecurityEnabled()
{
	return secEnabled;
}

void AquilaMesh::openEndpoint(uint8_t id, bool (*handler)(RxPacket *ind))
{
	NWK_OpenEndpoint(id, handler);
}

void AquilaMesh::sendPacket(TxPacket *packet)
{
	NWK_DataReq(packet);
}

// Announce device to the network
void AquilaMesh::announce(uint16_t dest)
{
	sendEUI(dest);
}

uint16_t AquilaMesh::getShortAddr()
{
	return shortAddr;
}

void AquilaMesh::getEUIAddr(uint8_t* address)
{
	memcpy(address, euiAddr, 8);
}

bool AquilaMesh::busy()
{
	return NWK_Busy();
}

void AquilaMesh::sleep()
{
	NWK_SleepReq();
	isAsleep = true;
}

void AquilaMesh::wakeup()
{
	NWK_WakeupReq();
	isAsleep = false;
}

bool AquilaMesh::asleep()
{
	return isAsleep;
}

AquilaMesh Mesh;
