#include "halPersist.h"

static uint16_t lastAllocatedAddress = 0;	//Updated each time an allocation is made (on init);

int Persist_init(Persist *self, uint16_t size)
{
	self->_initialized = false;
	//Check if enough memory left.
	if( (lastAllocatedAddress + size) <= EEPROM_SIZE )
	{
		self->_start = lastAllocatedAddress;
		self->_size = size;
		self->lastStatus = PERSIST_OK;
		self->_initialized = true;
		lastAllocatedAddress += size;
		return PERSIST_OK;
	}
	//Not enough space:
	self->lastStatus = PERSIST_ERROR;
	return PERSIST_ERROR;
}

uint8_t Persist_read(Persist *self, uint16_t address)
{
	if( !self->_initialized || address >= self->_size )
	{
		self->lastStatus = PERSIST_ERROR;
		return 0xFF;	//thats what some microcontrollers do...
	}

	self->lastStatus = PERSIST_OK;
	return eeprom_read_byte( (unsigned char *)(self->_start + address) );
}

int Persist_read_block(Persist *self, void * dest, uint16_t address, uint16_t size)
{
	if( !self->_initialized || address >= self->_size || address + size >= self->_size )
	{
		self->lastStatus = PERSIST_ERROR;
		return PERSIST_ERROR;
	}

	self->lastStatus = PERSIST_OK;
	eeprom_read_block(dest, (void*)(self->_start + address), size);
	return PERSIST_OK;
}

int Persist_write(Persist *self, uint16_t address, uint8_t value)
{
	if( !self->_initialized || address >= self->_size )
	{
		self->lastStatus = PERSIST_ERROR;
		return PERSIST_ERROR;
	}

	eeprom_write_byte( (unsigned char *)(self->_start + address), value );
	self->lastStatus = PERSIST_OK;
	return PERSIST_OK;
}

int Persist_write_block(Persist *self, void * src, uint16_t address, uint16_t size)
{
	if( !self->_initialized || address >= self->_size || address + size >= self->_size )
	{
		self->lastStatus = PERSIST_ERROR;
		return PERSIST_ERROR;
	}

	self->lastStatus = PERSIST_OK;
	eeprom_write_block(src, (void*)(self->_start + address), size);
	return PERSIST_OK;
}

uint16_t Persist_remaining()
{
	return EEPROM_SIZE - lastAllocatedAddress;
}