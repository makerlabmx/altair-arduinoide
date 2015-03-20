/**
* \file AquilaBridge.cpp
*
* \brief Aquila Mesh USB Bridge firmware.
*
* Copyright (C) 2014, Rodrigo Méndez. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3. The name of Makerlab may not be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* 4. This software may only be redistributed and used in connection with a
*    Makerlab product.
*
* THIS SOFTWARE IS PROVIDED BY MAKERLAB "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
* EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL MAKERLAB BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* Modification and other use of this code is subject to Makerlab's Limited
* License Agreement (license.txt).
*
*/


#include "AquilaBridgeXpro.h"
#include "RingBuffer.h"
#include "TxPacketRingBuffer.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "lwm/phy/phy.h"
#include "stack/hal.h"

//#define BRIDGE_DEBUG

#define TX_BUFFER_SIZE 132

bool txDataReqBusy = false;

uint8_t bridgeAddress[8];

uint8_t comTxBuffer[TX_BUFFER_SIZE];
RingBuffer pktRxBuffer;
TxRingBuffer txPktBuffer;

// Based on version from: http://www.gaw.ru/pdf/Atmel/app/avr/AVR350.pdf
uint16_t Bridge_calcrc(uint8_t *ptr, int count)
{
  int crc;
  char i;
  crc = 0;
  while (--count >= 0)
  {
    crc = crc ^ (int) *ptr++ << 8;
    i = 8;
    do
    {
      if (crc & 0x8000)
      crc = crc << 1 ^ 0x1021;
      else
      crc = crc << 1;
    } while(--i);
  }
  return (crc);
}

bool getChTimeOut(uint8_t *c, long timeOut)
{
  long lastTime = Hal_millis();
  long now;
  while(!Serial1.available())
  {
    now = Hal_millis();
    if( (now - lastTime) > timeOut ) return false;
  }
  *c = Serial1.read();
  return true;
}

void appendCRC(uint8_t buffer[], uint8_t size)
{
  uint16_t crc = Bridge_calcrc(buffer, size);
  buffer[size] = (uint8_t)(crc >> 8);
  buffer[size + 1] = (uint8_t)(crc);
}

void sendBuffer(uint8_t buffer[], uint8_t size)
{
  //appendCRC(buffer, size);
  // CRC is 16bits (2 bytes), now buffer size is 2 bytes more.
  //size += 2;

  uint8_t i;
  // Preamble:
  Serial1.write(0xAA);
  Serial1.write(0x55);
  Serial1.write(0xAA);
  Serial1.write(0x55);
  // Data:
  for(i = 0; i < size; i++)
  {
    Serial1.write(buffer[i]);
  }
}

void sendStart()
{
  comTxBuffer[0] = CMD_START;
  sendBuffer(comTxBuffer, 1);
}

void sendSuccess()
{
  comTxBuffer[0] = CMD_SUCCESS;
  sendBuffer(comTxBuffer, 1);
}

void sendError()
{
  comTxBuffer[0] = CMD_ERROR;
  sendBuffer(comTxBuffer, 1);
}

void sendProm(bool isProm)
{
  comTxBuffer[0] = CMD_SET_PROM;
  comTxBuffer[1] = (uint8_t) isProm;
  sendBuffer(comTxBuffer, 2);
}

void sendPan(uint16_t pan)
{
  comTxBuffer[0] = CMD_SET_PAN;
  comTxBuffer[1] = (uint8_t) (pan & 0xFF);
  comTxBuffer[2] = (uint8_t) (pan >> 8);
  sendBuffer(comTxBuffer, 3);
}

void sendChan(uint8_t chan)
{
  comTxBuffer[0] = CMD_SET_CHAN;
  comTxBuffer[1] = chan;
  sendBuffer(comTxBuffer, 2);
}

void sendShortAddr(uint16_t addr)
{
  comTxBuffer[0] = CMD_SET_SHORT_ADDR;
  comTxBuffer[1] = (uint8_t) (addr & 0xFF);
  comTxBuffer[2] = (uint8_t) (addr >> 8);
  sendBuffer(comTxBuffer, 3);
}

void sendLongAddr(uint8_t addr[])
{
  comTxBuffer[0] = CMD_SET_LONG_ADDR;
  comTxBuffer[1] = addr[0];
  comTxBuffer[2] = addr[1];
  comTxBuffer[3] = addr[2];
  comTxBuffer[4] = addr[3];
  comTxBuffer[5] = addr[4];
  comTxBuffer[6] = addr[5];
  comTxBuffer[7] = addr[6];
  comTxBuffer[8] = addr[7];
  sendBuffer(comTxBuffer, 9);
}

void sendSecurityEnabled()
{
  comTxBuffer[0] = CMD_GET_SECURITY;
  comTxBuffer[1] = Mesh.getSecurityEnabled();
  sendBuffer(comTxBuffer, 2);
}

static void txCb(NWK_DataReq_t *req)
{
    txDataReqBusy = false;

    /*
      Possible status:

      NWK_SUCCESS_STATUS
    NWK_ERROR_STATUS
    NWK_OUT_OF_MEMORY_STATUS

    NWK_NO_ACK_STATUS
    NWK_NO_ROUTE_STATUS

    NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS
    NWK_PHY_NO_ACK_STATUS
    */

    #ifdef BRIDGE_DEBUG
    Serial.print("txSuccess: ");
    Serial.println(req->status);
    #endif

    if(req->status == NWK_SUCCESS_STATUS)
    {
      sendSuccess();
    }
    else
    {
      sendError();
    }

    (void)req;
}

void txSend(uint16_t dstAddr, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t size, uint8_t *data)
{
  if(TxRingBuffer_isFull(&txPktBuffer)){ sendError(); return; } // lose packet...

  #ifdef BRIDGE_DEBUG
  Serial1.println("sending packet");
  #endif

  static TxBufPacket packet;
  packet.dstAddr = dstAddr;
  packet.dstEndpoint = dstEndpoint;
  packet.srcEndpoint = srcEndpoint;
  memcpy(packet.data, data, size);
  packet.size = size;
  TxRingBuffer_insert(&txPktBuffer, &packet);

  if(!txDataReqBusy) txSendNow();
}

void txSendNow()
{
  if(txDataReqBusy)
  { 
    #ifdef BRIDGE_DEBUG
    Serial.println("txDataReqBusy");
    #endif
    return; 
  } // Means we are not ready to send yet.

  #ifdef BRIDGE_DEBUG
  Serial.println("really sending packet");
  #endif

  static TxBufPacket bufPacket;
  TxRingBuffer_remove(&txPktBuffer, &bufPacket);

  static NWK_DataReq_t packet;
  packet.dstAddr = bufPacket.dstAddr;
  packet.dstEndpoint = bufPacket.dstEndpoint;
  packet.srcEndpoint = bufPacket.srcEndpoint;

  uint8_t requestAck = 0;
  if(bufPacket.dstAddr == BROADCAST) requestAck = 0;
  else requestAck = NWK_OPT_ACK_REQUEST;

  if(Mesh.getSecurityEnabled())
    packet.options = NWK_OPT_ENABLE_SECURITY | requestAck;
  else
    packet.options = requestAck;

  packet.data = bufPacket.data;
  packet.size = bufPacket.size;
  packet.confirm = txCb;

  #ifdef BRIDGE_DEBUG
  Serial.println(packet.dstAddr);
  Serial.println(packet.dstEndpoint);
  Serial.println(packet.srcEndpoint);
  Serial.println(packet.size);
  #endif

  NWK_DataReq(&packet);

  txDataReqBusy = true;
}

// Called on rx success, gets rx data
static bool rxHandler(NWK_DataInd_t *ind)
{
  // if security enabled and the package was not secured, ignore it.
  if( Mesh.getSecurityEnabled() && !(ind->options & NWK_IND_OPT_SECURED) ) return false;
  // Adding received packet to buffer
  // If buffer is full, ignore packet (lost)
  if(!RingBuffer_isFull(&pktRxBuffer))
  {
    uint8_t i = 0, j = 0;
    RingBufferData data;
    data.size = ind->size + 10;

    data.data[j++] = CMD_DATA;
    data.data[j++] = ind->lqi;
    data.data[j++] = ind->rssi;
    data.data[j++] = ind->srcAddr;
    data.data[j++] = ind->srcAddr >> 8;
    data.data[j++] = ind->dstAddr;
    data.data[j++] = ind->dstAddr >> 8;
    data.data[j++] = ind->srcEndpoint;
    data.data[j++] = ind->dstEndpoint;
    data.data[j++] = ind->size;

    for(i = 0; i < ind->size; i++)
    {
        data.data[j++] = ind->data[i];
    }
    RingBuffer_insert(&pktRxBuffer, &data);
    #ifdef BRIDGE_DEBUG
    Serial.println("got packet");
    #endif
    return true;
  }
  #ifdef BRIDGE_DEBUG
  else Serial.println("packet lost");
  #endif

  return false;
}

bool serialHandler()
{
  uint8_t temp, command, size, data[MAX_FRAME_SIZE], longAddr[8], srcEndpoint, dstEndpoint, secKey[16];
  uint16_t pan, shortAddr, recCrc, calcCrc, dstAddr;
  int i;
  //Getting header
  if(!getChTimeOut(&temp, TIMEOUT)) return false;
  if(temp != 0xAA) return false;
  if(!getChTimeOut(&temp, TIMEOUT)) return false;
  if(temp != 0x55) return false;
  if(!getChTimeOut(&temp, TIMEOUT)) return false;
  if(temp != 0xAA) return false;
  if(!getChTimeOut(&temp, TIMEOUT)) return false;
  if(temp != 0x55) return false;
  //Header ok, getting command
  if(!getChTimeOut(&command, TIMEOUT)) return false;

  switch(command)
  {
    case CMD_DATA:
      //get lqi and ignore
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      //get rssi and ignore
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      //get srcAddr Low byte and igonre
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      //get srcAddr High byte and igonre
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      //get dstAddr Low byte:
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      dstAddr = temp;
      //get dstAddr High byte:
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      dstAddr |= ((uint16_t)temp << 8);
      //get srcEndpoint:
      if(!getChTimeOut(&srcEndpoint, TIMEOUT)) return false;
      //get dstEndpoint:
      if(!getChTimeOut(&dstEndpoint, TIMEOUT)) return false;
      //getting size:
      if(!getChTimeOut(&size, TIMEOUT)) return false;
      //getting data:
      for(i = 0; i < size; i++)
      {
        if(!getChTimeOut(&data[i], TIMEOUT)) return false;
      }
      //getting CRC:
      /*if(!getChTimeOut(&temp, TIMEOUT)) return false;
      recCrc = (temp << 8) & 0xFF00;
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      recCrc |= temp & 0x00FF;*/                          // TERMINAR DE IMPLEMENTAR

      txSend(dstAddr, srcEndpoint, dstEndpoint, size, data);

      break;

    case CMD_SET_PAN:
      //get low pan byte
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      pan = temp;
      //get high pan byte
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      pan |= ((uint16_t)temp << 8);
      //set pan
      NWK_SetPanId(pan);
      // confirm:
      sendPan(pan);

      break;

    case CMD_SET_PROM:
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      //PHY_SetPromiscuous((bool)temp);
      // confirm:
      sendProm((bool)temp);
      break;

    case CMD_SET_CHAN:
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      PHY_SetChannel(temp);
      // confirm:
      sendChan(temp);
      break;

    case CMD_PING:
      // confirm:
      sendStart();
      break;

    case CMD_SET_SHORT_ADDR:
      //get low pan byte
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      shortAddr = temp;
      //get high shortAddr byte
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      shortAddr |= ((uint16_t)temp << 8);
      //set shortAddr
      NWK_SetAddr(shortAddr);
      // confirm:
      sendShortAddr(shortAddr);
      break;

    case CMD_GET_LONG_ADDR:
      sendLongAddr(bridgeAddress);
      break;

    case CMD_GET_SECURITY:
      sendSecurityEnabled();
      break;

    case CMD_SET_SECURITY:
      // get if enable security
      if(!getChTimeOut(&temp, TIMEOUT)) return false;
      Mesh.setSecurityEnabled((bool)temp);
      sendSecurityEnabled();
      break;

    case CMD_SET_KEY:
      for(i = 0; i < 16; i++)
      {
        if(!getChTimeOut(&temp, TIMEOUT)) return false;
        secKey[i] = temp;
      }
      Mesh.setSecurityKey(secKey);
      Mesh.setSecurityEnabled(true);
      sendSecurityEnabled();
      break;

    default:
      return false;
      break;
  }
  return true;

}

bool Bridge_init(uint16_t addr, uint8_t channel, uint16_t pan, bool promiscuous)
{
  ID_init();
  if(addr == NULL) Mesh.begin();
  else Mesh.begin(addr);

    Mesh.setChannel(channel);
    Mesh.setPanId(pan);
    if( !ID_getId(bridgeAddress) ) return false;    // Error, couldnt get address from chip
  //PHY_SetPromiscuous(promiscuous);
    RingBuffer_init(&pktRxBuffer);
    TxRingBuffer_init(&txPktBuffer);

    // Subscribe handler for all endpoints
    // 0 is reserved for LWM commands, dont use
    for(int i = 1; i < 16; i++)
    {
      NWK_OpenEndpoint(i, rxHandler);
    }

  //Serial_init();
  Serial1.begin(57600);
  // Anounce bridge ready to PC
  sendStart();
  delay(10);
  sendLongAddr(bridgeAddress);

  #ifdef BRIDGE_DEBUG
    Serial.begin(9600);
    Serial.println("Bridge Debug");
  #endif

  return true;
}

void Bridge_loop()
{
  Mesh.loop();
  if(Serial1.available()) serialHandler();
  if(!RingBuffer_isEmpty(&pktRxBuffer))
  {
    RingBufferData data;
    RingBuffer_remove(&pktRxBuffer, &data);
    sendBuffer(data.data, data.size);
  }
  if(!TxRingBuffer_isEmpty(&txPktBuffer))
  {
    txSendNow();
  }
}
