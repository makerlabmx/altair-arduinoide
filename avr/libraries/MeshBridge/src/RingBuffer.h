#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <util/atomic.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define RINGBUFFER_SIZE 10
#define RINGBUFFER_DATA_SIZE 132

typedef struct
{
	uint8_t data[RINGBUFFER_DATA_SIZE];
	uint8_t size;
} RingBufferData;

typedef struct
{
	RingBufferData buffer[RINGBUFFER_SIZE];
	RingBufferData *head;
	RingBufferData *tail;
	uint8_t count;
} RingBuffer;

void RingBuffer_init(RingBuffer* buffer);

bool RingBuffer_isFull(RingBuffer* buffer);

bool RingBuffer_isEmpty(RingBuffer* buffer);

void RingBuffer_insert(RingBuffer* buffer, RingBufferData* data);

void RingBuffer_remove(RingBuffer* buffer, RingBufferData* data);

#endif // RINGBUFFER_H