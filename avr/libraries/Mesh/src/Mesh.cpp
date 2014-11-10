#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "Arduino.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "Mesh.h"

// Begin with automatic address
void AquilaMesh::begin()
{
	// Not implemented
}

// Begin with manual address addr
void AquilaMesh::begin(uint16_t addr)
{
	SYS_Init();
	PHY_SetRxState(true);
	NWK_SetAddr(addr);
	NWK_SetPanId(AQUILAMESH_DEFPAN);
	PHY_SetChannel(AQUILAMESH_DEFCHAN);
}

void AquilaMesh::end()
{
	PHY_SetRxState(false);
}

void AquilaMesh::loop()
{
	SYS_TaskHandler();
}

void AquilaMesh::setAddr(uint16_t addr)
{
	NWK_SetAddr(addr);
}

void AquilaMesh::setPanId(uint16_t panId)
{
	NWK_SetPanId(panId);
}

void AquilaMesh::setChannel(uint8_t channel)
{
	PHY_SetChannel(channel);
}

// Leave this as advanced and require direct NWK_... calling??
// otherwhise we should redefine the Reqs etc... 
void AquilaMesh::openEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind))
{
	NWK_OpenEndpoint(id, handler);
}

AquilaMesh Mesh;