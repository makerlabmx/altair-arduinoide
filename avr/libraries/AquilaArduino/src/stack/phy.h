#ifndef PHY_H
#define PHY_H

#include "config.h"

#ifndef PHY_SECURITY_ENABLED
#define PHY_SECURITY_ENABLED 1
#endif

#include <stdint.h>
#include <stdbool.h>
#include "hal.h"
#include "halID.h"
#include "mac.h"
#include "phyrfr2.h"

#if PHY_SECURITY_ENABLED == 1
#include "Security.h"
#endif

#define PHY_RANDOM_SEED analogRead(A5)	//TODO Find another random source

#ifdef  __cplusplus
extern "C" {
#endif

static const uint8_t Phy_broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


typedef struct
{
	uint8_t data[MAX_DATA_SIZE];
	uint8_t dataLen;

	//status:
	uint8_t txOk:1;
    uint8_t retries:2;
    uint8_t channelBusy:1;

} TxPacket;

typedef struct
{
	uint8_t frameLen;
	uint8_t dataLen;

	uint16_t frameControl;
	uint8_t seqNumber;
	uint16_t destPAN;
	uint8_t destAddress[DEST_ADDRESS_SIZE];
	uint8_t srcAddress[SRC_ADDRESS_SIZE];
	#if PHY_SECURITY_ENABLED == 1
	uint8_t securityHeader[SECURITY_HEADER_SIZE];
	#endif
	uint8_t data[MAX_DATA_SIZE];

	//bool secured;
	bool error;
	#if PHY_SECURITY_ENABLED == 1
	uint8_t secLevel;
	uint32_t secFrameCount;
	uint8_t MUI[MUI_SIZE];
	#endif
	
	uint8_t lqi;
	uint8_t rssi;

} RxPacket;

typedef struct 
{
	bool securityEnabled;
	#if PHY_SECURITY_ENABLED == 1
	uint8_t secKey[SEC_KEYLEN];
	unsigned long frameCount;
	#endif
	uint8_t localAddress[SRC_ADDRESS_SIZE];
	uint16_t localPan;
	bool busy;

	TxPacket txPacket;
	RxPacket rxPacket;

	void (*rxHandler)(void);
	void (*txHandler)(void);

} Phy;

void Phy_init(Phy *self);

void Phy_setChannel(Phy *self, uint8_t channel);

void Phy_setPan(Phy *self, uint16_t pan);

void Phy_setRx(Phy *self, bool enabled);

void Phy_setTxPower(Phy *self, uint8_t power);

bool Phy_busy(Phy *self);

void Phy_sleep(Phy *self);

void Phy_wakeup(Phy *self);

void Phy_send(Phy *self, uint8_t *dest, uint8_t *data, uint8_t dataLen);

void Phy_loop(Phy *self);

#if PHY_SECURITY_ENABLED == 1
void Phy_setSecKey(Phy *self, uint8_t *key);
#endif

//Private:
void Phy__loadLocalAddress(Phy *self);

void Phy__setLocalAddress(Phy *self, uint8_t *address);

void Phy__setPromiscuous(Phy *self, bool prom);

void Phy__retrieveRx(Phy *self);

bool Phy__isBroadcast(uint8_t *address);

#ifdef  __cplusplus
}
#endif

#endif /* PHY_H */