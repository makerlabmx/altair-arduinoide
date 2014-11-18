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
} TxPacket;

typedef struct
{
	TxPacket buffer[TXPACKETRINGBUFFER_SIZE];
	TxPacket *head;
	TxPacket *tail;
	uint8_t count;
} TxRingBuffer;

void TxRingBuffer_init(TxRingBuffer* buffer);

bool TxRingBuffer_isFull(TxRingBuffer* buffer);

bool TxRingBuffer_isEmpty(TxRingBuffer* buffer);

void TxRingBuffer_insert(TxRingBuffer* buffer, TxPacket* data);

void TxRingBuffer_remove(TxRingBuffer* buffer, TxPacket* data);

#endif // TXPACKETRINGBUFFER_H