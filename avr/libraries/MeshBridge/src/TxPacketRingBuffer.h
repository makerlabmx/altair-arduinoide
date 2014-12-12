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
