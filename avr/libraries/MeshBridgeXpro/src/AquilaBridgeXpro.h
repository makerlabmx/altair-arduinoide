/**
* \file AquilaBridge.h
*
* \brief Aquila Mesh USB Bridge firmware.
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


#ifndef AQUILABRIDGEARDUINO_H
#define AQUILABRIDGEARDUINO_H

#include "stack/halID.h"
#include "stack/halSerial.h"
#include <Mesh.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define TIMEOUT 20
#define MAX_FRAME_SIZE 128

#define CMD_DATA 0
#define CMD_SET_PROM 1
#define CMD_SET_PAN 2
#define CMD_SET_CHAN 3
#define CMD_START 4
#define CMD_SET_SHORT_ADDR 5
#define CMD_SET_LONG_ADDR 6
#define CMD_PING 7
#define CMD_SUCCESS 8
#define CMD_ERROR 9
#define CMD_GET_LONG_ADDR 10
#define CMD_GET_SECURITY 11
#define CMD_SET_SECURITY 12
#define CMD_SET_KEY 13

bool getChTimeOut(uint8_t *c, long timeOut);

void sendPreamble();

void sendStart();

void sendSuccess();

void sendError();

void sendProm(bool isProm);

void sendPan(uint16_t pan);

void sendChan(uint8_t chan);

void sendShortAddr(uint16_t addr);

void sendLongAddr(uint8_t addr[]);

void txCb(uint8_t status);

void txSend(uint16_t dstAddr, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t size, uint8_t *data);

void txSendNow();

// Called on rx success, gets rx data
void rxHandler(PHY_DataInd_t *ind);

bool serialHandler();

bool Bridge_init(uint16_t addr, uint8_t channel, uint16_t pan, bool promiscuous);

void Bridge_loop();

// #define DEBUG

#ifdef  __cplusplus
}
#endif

#endif //AQUILABRIDGEARDUINO_H
