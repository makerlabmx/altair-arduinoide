#include "RingBuffer.h"

void RingBuffer_init(RingBuffer* buffer)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->head = buffer->buffer;
		buffer->tail = buffer->buffer;
		buffer->count = 0;
	}
}

bool RingBuffer_isFull(RingBuffer* buffer)
{
	uint8_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = buffer->count;
	}
	return count == RINGBUFFER_SIZE;
}

bool RingBuffer_isEmpty(RingBuffer* buffer)
{
	uint8_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = buffer->count;
	}
	return count == 0;
}

void RingBuffer_insert(RingBuffer* buffer, RingBufferData* data)
{

	buffer->head->size = data->size;
	memcpy(buffer->head->data, data->data, data->size);

	if (++buffer->head == &buffer->buffer[RINGBUFFER_SIZE])
	{
		buffer->head = buffer->buffer;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->count++;
	}
}

void RingBuffer_remove(RingBuffer* buffer, RingBufferData* data)
{
	data->size = buffer->tail->size;
	memcpy(data->data, buffer->tail->data, buffer->tail->size);
			
	if (++buffer->tail == &buffer->buffer[RINGBUFFER_SIZE])
	{
		buffer->tail = buffer->buffer;
	}
	

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->count--;
	}
}