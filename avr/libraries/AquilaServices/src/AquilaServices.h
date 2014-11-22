#ifndef AQUILASERVICES_H
#define AQUILASERVICES_H

/*
	Aquila Services Specification:

	service: Function
	data: JSON proposed
	
	Packet:
		methods (2 bits):
			GET		0x00
			PUT		0x01
			POST	0x02
			DELETE	0x03
		size (8bits)
		data (0 to (MESH_MAXSIZE - 2))


	bool myService(uint16_t reqAddr, uint8_t method, uint8_t *data, uint8_t dataSize)
	{
		...
		response(reqAddr, NULL, 0);
	}
*/

#include <stdint.h>
#include <stdbool.h>
#include <Mesh.h>

#define AQUILASERVICES_MAX 20
#define AQUILASERVICES_ENDPOINT 12
#define AQUILASERVICES_VERSION 1

// request methods
#define GET 0x00
#define PUT 0x01
#define POST 0x02
#define DELETE 0x03
// responses
#define R200 0x04	// OK
#define R404 0x05	// Service not found
#define R500 0x06	// Service error
#define R408 0x07	// Timeout

#define AQUILASERVICES_MAXNAMESIZE 16
#define AQUILASERVICES_MAXDATASIZE AQUILAMESH_MAXPAYLOAD - 4

#define AQUILASERVICES_TIMEOUT 1000

typedef struct
{
	uint8_t version;
	uint8_t method;
	uint8_t nameSize;
	uint8_t dataSize;
	uint8_t name_data[AQUILASERVICES_MAXDATASIZE];
} ServicePacket;

typedef struct
{
	char *name;
	bool (*function)(uint16_t reqAddr, uint8_t method, uint8_t *data, uint8_t dataSize);
} Service;

class AquilaServices
{
private:
	
public:
	AquilaServices();
	void begin();
	void loop();
	// Inscribe un servicio a name
	void add(char *name, bool (*function)(uint16_t reqAddr, uint8_t method, uint8_t *data, uint8_t dataSize));
	// Petición
	void request(uint16_t destAddr, uint8_t method, char *name, void (*callback)(uint16_t srcAddr, uint8_t status, uint8_t *data, uint8_t dataSize), uint8_t *data = NULL, uint8_t dataSize = 0);
	// Para usar dentro del servicio
	void response(uint16_t destAddr, uint8_t method, uint8_t *data = NULL, uint8_t dataSize = 0);
};

extern AquilaServices Services;

#endif // AQUILASERVICES_H