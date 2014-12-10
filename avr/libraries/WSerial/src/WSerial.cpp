#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "WSerial.h"

static NWK_DataReq_t packet;
uint8_t packetData[WSERIAL_BUFFER_SIZE];

uint16_t destAddress;
bool allowFromAny = false;
volatile uint8_t _rx_buffer_head;
volatile uint8_t _rx_buffer_tail;
volatile uint8_t _tx_buffer_head;
volatile uint8_t _tx_buffer_tail;

bool appDataReqBusy = false;

uint8_t _rx_buffer[WSERIAL_BUFFER_SIZE];
uint8_t _tx_buffer[WSERIAL_BUFFER_SIZE];

static bool receiveMessage(NWK_DataInd_t *ind)
{
	// Check if its from the address we are expecting,
	// ignore otherwise
	// If destAddress == BROADCAST, Accept all
	// If allowFromAny == true, accept all
	if( !(destAddress == BROADCAST || allowFromAny == true) && ind->srcAddr != destAddress) return false;
	// Put received data into rx buffer
	// if not enough space, drop data :(
	if(ind->size > (WSERIAL_BUFFER_SIZE - (unsigned int)(WSERIAL_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail) % WSERIAL_BUFFER_SIZE) ) return false;

	for(int i = 0; i < ind->size; i++)
	{
		_rx_buffer[ (_rx_buffer_head + i) % WSERIAL_BUFFER_SIZE ] = ind->data[i];
	}

	_rx_buffer_head = (_rx_buffer_head + ind->size) % WSERIAL_BUFFER_SIZE;
}

static void appDataCb(NWK_DataReq_t *req)
{
  appDataReqBusy = false;
  (void)req;
}

void WirelessSerial::begin(uint16_t destAddr)
{
	_rx_buffer_head = _rx_buffer_tail = 0;
	_tx_buffer_head = _tx_buffer_tail = 0;
	setDest(destAddr);
	// SYS_Init(); Should init in Mesh.begin()
	PHY_SetRxState(true);
	NWK_OpenEndpoint(WSERIAL_ENDPOINT, receiveMessage);
}

void WirelessSerial::setDest(uint16_t destAddr)
{
	destAddress = destAddr;
}

void WirelessSerial::setAllowFromAny(bool allow)
{
	allowFromAny = allow;
}


void WirelessSerial::end()
{
	PHY_SetRxState(false);
}

int WirelessSerial::available(void)
{
	return (unsigned int)(WSERIAL_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail) % WSERIAL_BUFFER_SIZE;
}

int WirelessSerial::peek(void)
{
	if (_rx_buffer_head == _rx_buffer_tail) {
		return -1;
	} else {
		return _rx_buffer[_rx_buffer_tail];
	}
}

int WirelessSerial::read(void)
{
	// if the head isn't ahead of the tail, we don't have any characters
	if (_rx_buffer_head == _rx_buffer_tail) {
		return -1;
	} else {
		unsigned char c = _rx_buffer[_rx_buffer_tail];
		_rx_buffer_tail = (uint8_t)(_rx_buffer_tail + 1) % WSERIAL_BUFFER_SIZE;
		return c;
	}
}

void WirelessSerial::flush()
{
	// Should force send whatever is in _tx_buffer
	this->sendPacketNow();
}

size_t WirelessSerial::write(uint8_t c)
{
	// Calc new index
	uint8_t i = (_tx_buffer_head + 1) % WSERIAL_BUFFER_SIZE;
	// Check if buffer is full
	if(i == _tx_buffer_tail)
	{
		// send now whatever is in the buffer
		this->sendPacketNow();
		// and then recalculate index to append to buffer
		i = (_tx_buffer_head + 1) % WSERIAL_BUFFER_SIZE;
	}

	_tx_buffer[_tx_buffer_head] = c;
  	_tx_buffer_head = i;

  	return 1;
}

void WirelessSerial::sendPacketNow()
{
	// check if there is something to send or if busy
	if(_tx_buffer_head == _tx_buffer_tail || appDataReqBusy) return;

	unsigned int size = (WSERIAL_BUFFER_SIZE + _tx_buffer_head - _tx_buffer_tail) % WSERIAL_BUFFER_SIZE;

	for(int i = 0; i < size; i++)
	{
		packetData[i] = _tx_buffer[ (_tx_buffer_tail + i) % WSERIAL_BUFFER_SIZE ];
	}

	// empty buffer
	_tx_buffer_head = _tx_buffer_tail;

	packet.dstAddr = destAddress;
	packet.dstEndpoint = WSERIAL_ENDPOINT;
	packet.srcEndpoint = WSERIAL_ENDPOINT;
	if(Mesh.getSecurityEnabled())
		packet.options = NWK_OPT_ENABLE_SECURITY;
	else
		packet.options = 0;
	packet.data = packetData;
	packet.size = size;
	packet.confirm = appDataCb;
	NWK_DataReq(&packet);

	appDataReqBusy = true;
}

void WirelessSerial::loop()
{
	unsigned long now = millis();
	if(now > lastTimeSent + WSERIAL_SEND_INTERVAL)
	{
		this->sendPacketNow();
		lastTimeSent = millis();
	}
}

WirelessSerial WSerial;
