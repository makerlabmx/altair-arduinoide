/**
* \file TxPacketRingBuffer.h
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


#ifndef TXPACKETRINGBUFFER_H
#define TXPACKETRINGBUFFER_H

#include <util/atomic.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <Mesh.h>
#include "lwm/nwk/nwk.h"

#define TXPACKETRINGBUFFER_SIZE 10

typedef struct
{
	uint16_t dstAddr;
	uint8_t dstEndpoint;
	uint8_t srcEndpoint;
	uint8_t data[AQUILAMESH_MAXPAYLOAD];
	uint8_t size;
} TxBufPacket;

typedef struct
{
	TxBufPacket buffer[TXPACKETRINGBUFFER_SIZE];
	TxBufPacket *head;
	TxBufPacket *tail;
	uint8_t count;
} TxRingBuffer;

void TxRingBuffer_init(TxRingBuffer* buffer);

bool TxRingBuffer_isFull(TxRingBuffer* buffer);

bool TxRingBuffer_isEmpty(TxRingBuffer* buffer);

void TxRingBuffer_insert(TxRingBuffer* buffer, TxBufPacket* data);

void TxRingBuffer_remove(TxRingBuffer* buffer, TxBufPacket* data);

#endif // TXPACKETRINGBUFFER_H
