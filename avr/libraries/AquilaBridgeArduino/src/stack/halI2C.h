#ifndef I2C_H
#define I2C_H

/*
  C port of the Arduino Wire library.
  By Rodrigo Méndez, rmendez@makerlab.mx, 2014.
  Based on work by Nicholas Zambetti.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#define BUFFER_LENGTH 32

#ifdef  __cplusplus
extern "C" {
#endif

void I2C_init();
void I2C_initSlave(uint8_t);
void I2C_beginTransmission(uint8_t);
uint8_t I2C_endTransmission(uint8_t);
uint8_t I2C_requestFrom(uint8_t, uint8_t, uint8_t);

size_t I2C_write(uint8_t);
size_t I2C_writeN(const uint8_t *, size_t);
int I2C_available();
int I2C_read();
int I2C_peek();
void I2C_flush();

void I2C_onReceive( void (*)(int) );
void I2C_onRequest( void (*)(void) );


#ifdef  __cplusplus
}
#endif

#endif /* I2C_H */