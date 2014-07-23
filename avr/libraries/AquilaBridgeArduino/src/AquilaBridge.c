#include "AquilaBridge.h"

uint8_t bridgeAddress[8];

bool getChTimeOut(uint8_t *c, long timeOut)
{
	long lastTime = Hal_millis();
	long now;
	while(!Serial_available())
	{
		now = Hal_millis();
		if( (now - lastTime) > timeOut ) return false;
	}
	*c = getchar();
	return true;
}

void sendPreamble()
{
	putchar(0xAA);
	putchar(0x55);
	putchar(0xAA);
	putchar(0x55);
}

void sendStart()
{
	sendPreamble();
	putchar(CMD_START);
}

void sendSuccess()
{
	sendPreamble();
	putchar(CMD_SUCCESS);
}

void sendError()
{
	sendPreamble();
	putchar(CMD_ERROR);
}

void sendProm(bool isProm)
{
	sendPreamble();
	putchar(CMD_SET_PROM);
	putchar((uint8_t) isProm);
}

void sendPan(uint16_t pan)
{
	sendPreamble();
	putchar(CMD_SET_PAN);
	putchar((uint8_t) (pan & 0xFF));
	putchar((uint8_t) (pan >> 8));
}

void sendChan(uint8_t chan)
{
	sendPreamble();
	putchar(CMD_SET_CHAN);
	putchar(chan);
}

void sendShortAddr(uint16_t addr)
{
	sendPreamble();
	putchar(CMD_SET_SHORT_ADDR);
	putchar((uint8_t) (addr & 0xFF));
	putchar((uint8_t) (addr >> 8));
}

void sendLongAddr(uint8_t addr[])
{
	sendPreamble();
	putchar(CMD_SET_LONG_ADDR);
	putchar(addr[0]);
	putchar(addr[1]);
	putchar(addr[2]);
	putchar(addr[3]);
	putchar(addr[4]);
	putchar(addr[5]);
	putchar(addr[6]);
	putchar(addr[7]);
}

void PHY_DataConf(uint8_t status)
{
    /*
    TRAC_STATUS_SUCCESS             
    TRAC_STATUS_SUCCESS_DATA_PENDING  
    TRAC_STATUS_SUCCESS_WAIT_FOR_ACK  
    TRAC_STATUS_CHANNEL_ACCESS_FAILURE
    TRAC_STATUS_NO_ACK                
    TRAC_STATUS_INVALID
    */

    if(status == TRAC_STATUS_SUCCESS || status == TRAC_STATUS_SUCCESS_DATA_PENDING)
    {
    	sendSuccess();
    }
    else if(status == TRAC_STATUS_CHANNEL_ACCESS_FAILURE || status == TRAC_STATUS_NO_ACK || status == TRAC_STATUS_INVALID)
    {
    	sendError();
    }
}

// Called on rx success, gets rx data
void PHY_DataInd(PHY_DataInd_t *ind)
{
    // Sending data to PC:
    //delay(20);
    sendPreamble();
	putchar(CMD_DATA);
	putchar(ind->lqi);
	putchar(ind->rssi);
	putchar(ind->size);

    uint8_t i = 0;
    for(i = 0; i < ind->size; i++)
    {
        putchar(ind->data[i]);
    }
}

bool serialHandler()
{
	uint8_t temp, command, size, data[MAX_FRAME_SIZE], longAddr[8];
	uint16_t pan, shortAddr;
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

	//puts("gotCommand");

	switch(command)
	{
		case CMD_DATA:
			//get lqi and ignore
			if(!getChTimeOut(&temp, TIMEOUT)) return false;
			//get rssi and ignore
			if(!getChTimeOut(&temp, TIMEOUT)) return false;
			//getting size:
			if(!getChTimeOut(&size, TIMEOUT)) return false;
			//getting data:
			for(i = 0; i < size; i++)
			{
				if(!getChTimeOut(&data[i], TIMEOUT)) return false;
			}

    		PHY_DataReq(data, size);

			break;

		case CMD_SET_PAN:
			//get low pan byte
			if(!getChTimeOut(&temp, TIMEOUT)) return false;
			pan = temp;
			//get high pan byte
			if(!getChTimeOut(&temp, TIMEOUT)) return false;
			pan |= ((uint16_t)temp << 8);
			//set pan
			PHY_SetPanId(pan);
			// confirm:
			sendPan(pan);

			break;

		case CMD_SET_PROM:
			if(!getChTimeOut(&temp, TIMEOUT)) return false;
			PHY_SetPromiscuous((bool)temp);
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
			PHY_SetShortAddr(shortAddr);
			// confirm:
			sendShortAddr(shortAddr);
			break;

		case CMD_SET_LONG_ADDR:
			for(i = 0; i < 8; i++)
			{
				if(!getChTimeOut(&longAddr[i], TIMEOUT)) return false;
			}
			PHY_SetLongAddr(longAddr);
			// confirm:
			sendLongAddr(longAddr);
			break;

		case CMD_GET_LONG_ADDR:
			sendLongAddr(bridgeAddress);
			break;

		default:
			return false;
			break;
	}
	return true;

}

bool Bridge_init(uint8_t channel, uint16_t pan, bool promiscuous)
{
	ID_init();
	PHY_Init();
    PHY_SetChannel(channel);
    PHY_SetPanId(pan);
    if( !ID_getId(bridgeAddress) ) return false;		// Error, couldnt get address from chip
    PHY_SetLongAddr(bridgeAddress);
	PHY_SetPromiscuous(promiscuous);
    PHY_SetRxState(true);
	Serial_init();
	// Anounce bridge ready to PC
	sendStart();
	delay(10);
	sendLongAddr(bridgeAddress);
	return true;
}

void Bridge_loop()
{
	PHY_TaskHandler();
	if(Serial_available()) serialHandler();
}
