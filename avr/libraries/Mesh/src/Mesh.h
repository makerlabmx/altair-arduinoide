#ifndef AQUILAMESH_H
#define AQUILAMESH_H

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"

#define BROADCAST 0xFFFF

#define AQUILAMESH_DEFPAN 0xCA5A
#define AQUILAMESH_DEFCHAN 0x1A

// According to Atmel-42028-Lightweight-Mesh-Developer-Guide_Application-Note_AVR2130,
// Max payload size is 105 with security and 32bit MIC and 109 without security
#define AQUILAMESH_MAXPAYLOAD (NWK_MAX_PAYLOAD_SIZE - 4)/*Security 32bit MIC*/

// Frendlier names for LWM types
#define TxPacket NWK_DataReq_t
#define RxPacket NWK_DataInd_t

class AquilaMesh
{
private:
public:
	AquilaMesh();
	void begin();
	void begin(uint16_t addr);

	void end();

	void loop();

	void setAddr(uint16_t addr);
	void setPanId(uint16_t panId);
	void setChannel(uint8_t channel);
	void setSecurityKey(uint8_t *key);
	void setSecurityEnabled(bool enabled);
	bool getSecurityEnabled();
	void openEndpoint(uint8_t id, bool (*handler)(RxPacket *ind));

	void sendPacket(TxPacket *packet);

	void announce(uint16_t dest);

	uint16_t getShortAddr();
	void getEUIAddr(uint8_t* address);

};

extern AquilaMesh Mesh;

#endif //AQUILAMESH_H
