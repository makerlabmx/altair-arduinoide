#ifndef AQUILABRIDGEARDUINO_H
#define AQUILABRIDGEARDUINO_H

#include "stack/halID.h"
#include "stack/halSerial.h"
#include <Mesh.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define TIMEOUT 20
#define MAX_FRAME_SIZE 128

#define CMD_DATA 0
#define CMD_SET_PROM 1
#define CMD_SET_PAN 2
#define CMD_SET_CHAN 3
#define CMD_START 4
#define CMD_SET_SHORT_ADDR 5
#define CMD_SET_LONG_ADDR 6
#define CMD_PING 7
#define CMD_SUCCESS 8
#define CMD_ERROR 9
#define CMD_GET_LONG_ADDR 10
#define CMD_GET_SECURITY 11
#define CMD_SET_SECURITY 12
#define CMD_SET_KEY 13

bool getChTimeOut(uint8_t *c, long timeOut);

void sendPreamble();

void sendStart();

void sendSuccess();

void sendError();

void sendProm(bool isProm);

void sendPan(uint16_t pan);

void sendChan(uint8_t chan);

void sendShortAddr(uint16_t addr);

void sendLongAddr(uint8_t addr[]);

void txCb(uint8_t status);

void txSend(uint16_t dstAddr, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t size, uint8_t *data);

void txSendNow();

// Called on rx success, gets rx data
void rxHandler(PHY_DataInd_t *ind);

bool serialHandler();

bool Bridge_init(uint16_t addr, uint8_t channel, uint16_t pan, bool promiscuous);

void Bridge_loop();

// #define DEBUG

#ifdef  __cplusplus
}
#endif

#endif //AQUILABRIDGEARDUINO_H
