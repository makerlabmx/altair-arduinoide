/**
* \file AquilaProtocol.cpp
*
* \brief Aquila Events, Actions and Interactions based protocol implementation.
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

#include "Arduino.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "Mesh.h"
#include "AquilaProtocol.h"

int AquilaProtocol::begin()
{
	return Protocol_init(&protocol, Mesh.getSecurityEnabled());
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

AquilaProtocol Aquila;
