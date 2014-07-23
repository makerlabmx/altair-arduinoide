#ifndef PERSIST_H
#define PERSIST_H

#include "config.h"

#include <stdint.h>
#include <stdbool.h>
#include <avr/eeprom.h>
#include "hal.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*
 *	Hal Persist, filesystem-like EEPROM abstraction.
 *	Current implementation uses uint16_t indexes, for a theoretical max of 64 KB.
 *	Allocates part of the EEPROM for application, currently cannot deallocate previously allocated space.
 *	Warning: keep in mind that allocations should be done in the same order every time the device is started,
 *	If this order is altered or something changes in development, you should erase the whole EEPROM after programming,
 *	else you can get wrong data readings.
 */

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 8192
#endif

#define PERSIST_OK 1
#define	PERSIST_ERROR 0

typedef struct
{
	int lastStatus;	//status result of last operation

	uint16_t _start;	//Start address in EEPROM
	uint16_t _size;	//number of bytes allocated
	bool _initialized;
} Persist;

int Persist_init(Persist *self, uint16_t size);

uint8_t Persist_read(Persist *self, uint16_t address);

int Persist_read_block(Persist *self, void * dest, uint16_t address, uint16_t size);

// Persist_write: writes a byte in Persist address, returns success or error.
int Persist_write(Persist *self, uint16_t address, uint8_t value);

int Persist_write_block(Persist *self, void * src, uint16_t address, uint16_t size);


// Persist_remaining: returns remaining unallocated bytes in EEPROM
uint16_t Persist_remaining();

#ifdef  __cplusplus
}
#endif

#endif /* PERSIST_H */