#ifndef AQUILAMESH_H
#define AQUILAMESH_H

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"

#define AQUILAMESH_DEFPAN 0xCA5A
#define AQUILAMESH_DEFCHAN 0x1A

// According to Atmel-42028-Lightweight-Mesh-Developer-Guide_Application-Note_AVR2130,
// Max payload size is 105 with security and 32bit MIC and 109 without security
#define AQUILAMESH_MAXPAYLOAD 105

class AquilaMesh
{
private:
public:
	void begin();
	void begin(uint16_t addr);

	void end();

	void loop();

	void setAddr(uint16_t addr);
	void setPanId(uint16_t panId);
	void setChannel(uint8_t channel);
	void openEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind));
};

extern AquilaMesh Mesh;

#endif //AQUILAMESH_H