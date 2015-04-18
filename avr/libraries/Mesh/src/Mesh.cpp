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
#include "TxBuffer.h"

#define MESH_ADDRENDPOINT 15
#define MESH_CMD_GETEUI 0
#define MESH_CMD_RESEUI 1
/////

// Transmission management
static TxBuffer txBuffer;
static bool txBusy;
void (*txUserConfirm)(TxPacket *packet);
void txConfirm(TxPacket *packet)
{
	// Tx finished
	txBusy = false;
	// Call user callback
	if(txUserConfirm != NULL) txUserConfirm(packet);
}

/////

static TxPacket packet;

static uint16_t shortAddr;
static uint16_t genAddr;
static bool waitingAddrConfirm;
static bool addrAssignSuccess;
static bool secEnabled;
static uint8_t euiAddr[8];
uint16_t calcrc(uint8_t *ptr, int count);
static void pingCb(TxPacket *req);
void sendEUI(uint16_t dest);
static bool meshCb(RxPacket *ind);
void assignAddr(uint8_t *id);

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

// This needs to be treated separatedly outside of mesh...
bool euiBusy = false;

void euiCb(TxPacket* packet)
{
	euiBusy = false;
}

void sendEUI(uint16_t dest)
{
	if(euiBusy) return;
	static uint8_t data[9];
	data[0] = MESH_CMD_RESEUI;

	// copy eui
	memcpy(&data[1], euiAddr, 8);

	packet.dstAddr = dest;
	packet.dstEndpoint = MESH_ADDRENDPOINT;
	packet.srcEndpoint = MESH_ADDRENDPOINT;

	uint8_t requestAck = 0;
	if(dest == BROADCAST) requestAck = 0;
	else requestAck = NWK_OPT_ACK_REQUEST;

	if(secEnabled)
		packet.options = requestAck | NWK_OPT_ENABLE_SECURITY;
	else
		packet.options = requestAck;

	packet.data = data;
	packet.size = 9;
	packet.confirm = euiCb;
	
	euiBusy = true;
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

void assignAddr(uint8_t *id)
{
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

AquilaMesh::AquilaMesh()
{
	secEnabled = false;
	isAsleep = false;
	shortAddr = 0;
	genAddr = 0;
	waitingAddrConfirm = false;
	addrAssignSuccess = false;
	TxBuffer_init(&txBuffer);
	txUserConfirm = NULL;
}

// Begin with automatic address
bool AquilaMesh::begin()
{
	#ifdef MESH_DEBUG
	Serial_init();
	#endif
	// Init as address 0:
	if(!begin(0)) return false;

	uint8_t id[8];
	getEUIAddr(id);

	assignAddr(id);

	return true;
}

// Begin with manual address addr
bool AquilaMesh::begin(uint16_t addr)
{

	SYS_Init();
	PHY_SetRxState(true);
	shortAddr = addr;
	NWK_SetAddr(addr);
	NWK_SetPanId(AQUILAMESH_DEFPAN);
	PHY_SetChannel(AQUILAMESH_DEFCHAN);

	ID_init();
	if( !ID_getId(euiAddr) ) return false;        //Error

	NWK_OpenEndpoint(MESH_ADDRENDPOINT, meshCb);
	return true;
}

void AquilaMesh::end()
{
	PHY_SetRxState(false);
}

void AquilaMesh::loop()
{
	SYS_TaskHandler();
	// Send any pending packets
	sendNow();
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

bool AquilaMesh::sendPacket(TxPacket *packet)
{
	if(TxBuffer_isFull(&txBuffer)) return false;

	TxBufPacket bufPacket;
	TxBufPacket_init(&bufPacket, packet);

	TxBuffer_insert(&txBuffer, &bufPacket);
	// Try to send now
	sendNow();
}

void AquilaMesh::sendNow()
{
	if(TxBuffer_isEmpty(&txBuffer) || txBusy) return;
	// Get package from buffer
	TxBufPacket bufPacket;
	TxBuffer_remove(&txBuffer, &bufPacket);

	// Enforce security
	if(secEnabled)
		bufPacket.packet.options = bufPacket.packet.options | NWK_OPT_ENABLE_SECURITY;
	else
		bufPacket.packet.options = bufPacket.packet.options & ~NWK_OPT_ENABLE_SECURITY;

	// Setup the callback
	// Save user confirm callback
	txUserConfirm = bufPacket.packet.confirm;
	// Assign our callback for tx control
	bufPacket.packet.confirm = txConfirm;
	// Request send to nwk layer
	NWK_DataReq(&bufPacket.packet);
	txBusy = true;
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
	while(this->busy()) this->loop();
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
