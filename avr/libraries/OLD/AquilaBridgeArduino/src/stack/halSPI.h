#ifndef HALSPI_H
#define HALSPI_H

#include "config.h"

#include <stdio.h>
#include <avr/pgmspace.h>
#include "hal.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*
 *	SPI driver, based on arduino SPI implementation.
 */

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06
//#define SPI_CLOCK_DIV64 0x07

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

inline static byte SPI_transfer(byte _data);

// SPI Configuration methods

inline static void SPI_attachInterrupt();
inline static void SPI_detachInterrupt(); // Default

void SPI_init(); // Default
void SPI_deInit();

void SPI_setBitOrder(uint8_t);
void SPI_setDataMode(uint8_t);
void SPI_setClockDivider(uint8_t);

byte SPI_transfer(byte _data) {
  SPDR = _data;
  while (!(SPSR & _BV(SPIF)))
    ;
  return SPDR;
}

void SPI_attachInterrupt() {
  SPCR |= _BV(SPIE);
}

void SPI_detachInterrupt() {
  SPCR &= ~_BV(SPIE);
}

#ifdef  __cplusplus
}
#endif

#endif /* HALSPI_H */