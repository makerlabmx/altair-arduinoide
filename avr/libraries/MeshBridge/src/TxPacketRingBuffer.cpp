#include "TxPacketRingBuffer.h"

void TxRingBuffer_init(TxRingBuffer* buffer)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->head = buffer->buffer;
		buffer->tail = buffer->buffer;
		buffer->count = 0;
	}
}

bool TxRingBuffer_isFull(TxRingBuffer* buffer)
{
	uint8_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = buffer->count;
	}
	return count == TXPACKETRINGBUFFER_SIZE;
}

bool TxRingBuffer_isEmpty(TxRingBuffer* buffer)
{
	uint8_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = buffer->count;
	}
	return count == 0;
}

void TxRingBuffer_insert(TxRingBuffer* buffer, TxPacket* data)
{

	memcpy(buffer->head, data, sizeof(TxPacket));

	if (++buffer->head == &buffer->buffer[TXPACKETRINGBUFFER_SIZE])
	{
		buffer->head = buffer->buffer;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->count++;
	}
}

void TxRingBuffer_remove(TxRingBuffer* buffer, TxPacket* data)
{
	memcpy(data, buffer->tail, sizeof(TxPacket));
			
	if (++buffer->tail == &buffer->buffer[TXPACKETRINGBUFFER_SIZE])
	{
		buffer->tail = buffer->buffer;
	}
	

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->count--;
	}
}