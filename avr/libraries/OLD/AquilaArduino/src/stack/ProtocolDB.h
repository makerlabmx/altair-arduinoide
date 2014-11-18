#ifndef PROTOCOLDB_H
#define PROTOCOLDB_H

#include <stdint.h>
#include <stdbool.h>
#include "halPersist.h"

#ifndef PROTOCOL_PERSIST_SIZE
#define PROTOCOL_PERSIST_SIZE 1024
#endif

/*
	Memory structure:
		DBHeader
		Entry
		Entry
		...
		Entry

	DBHeader structure:
		signature0	->	'm'
		signature1	->	'a'
		signature2	->	'k'
		entrySize	->	sizeof(Entry)
		nEntries	->	<number of current active entries>
		indexSectors[0]
		indezSectors[1]	->  Each bit of this indexes represents one entry
		...					on memory, 1 active, 0 free.
		indexSectors[11]	Example: bit 0 of indezSectors[0] is the first entry
							(entry 0) after the Header. 
							Bit 2 of indezSectors[2] is the entry number 18.

	How it works:
		At init, checks if signature and entrySize are correct, if not,
		an empty new header is written. (this could happen when the device is new,
		the memory gets corrupted or a change in this code is made.)
*/

#define PROTOCOL_ENTRYLEN sizeof(Entry)
// Lets use a fixed value for a PERSIST_SIZE of 1024 for now...
// Chosen so that PERSIST_SIZE < sizeof(DBHeader) + (PROTOCOL_NINDEXSECTORS * sizeof(IndexSector) * sizeof(Entry))
#define PROTOCOL_NINDEXSECTORS 10
#define PROTOCOL_MAXENTRIES 80	// PROTOCOL_NINDEXSECTORS * sizeof(IndexSector)
//Decimal points are IMPORTANT (for float casting)
//#define PROTOCOL_NINDEXSECTORS (ceil( (PROTOCOL_PERSIST_SIZE - 1.0) / ( (8.0 * PROTOCOL_ENTRYLEN) + 1.0) ))
//#define PROTOCOL_MAXENTRIES (floor((PROTOCOL_PERSIST_SIZE - 1.0 - PROTOCOL_NINDEXSECTORS)/PROTOCOL_ENTRYLEN))

#ifdef  __cplusplus
extern "C" {
#endif

#define PDB_OK 1
#define PDB_ERROR 0
#define PDB_FORMATED -1

/*typedef struct
{
	bool i0: 1;
	bool i1: 1;
	bool i2: 1;
	bool i3: 1;
	bool i4: 1;
	bool i5: 1;
	bool i6: 1;
	bool i7: 1;
} IndexSector;*/

#define IndexSector uint8_t

typedef struct
{
	uint8_t signature[3];
	uint8_t entrySize;
	uint8_t nEntries;
	IndexSector indexSectors[PROTOCOL_NINDEXSECTORS];
} DBHeader;

typedef struct
{
	bool hasParam :1;
	uint8_t reserved : 7;

} EntryConfig;

typedef struct
{
	EntryConfig config;
	uint8_t event;
	uint8_t address[8];	// 64 bit address
	uint8_t action;
	uint8_t param;
} Entry;

int PDB_init();

int PDB_format();

int PDB_readHeader(DBHeader *header);

int PDB_appendEntry(Entry *entry);

int PDB_getEntry(Entry *entry, uint8_t nEntry);

int PDB_setEntry(Entry *entry, uint8_t nEntry);

int PDB_delEntry(uint8_t nEntry);

void PDB_iteratorBegin();

int PDB_iteratorGetNextEntry(Entry *entry);

#ifdef  __cplusplus
}
#endif
#endif	/* PROTOCOLDB_H */