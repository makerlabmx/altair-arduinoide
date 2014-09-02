#include "AquilaBridge.h"
#include "RingBuffer.h"

#define TX_BUFFER_SIZE 132

uint8_t bridgeAddress[8];

uint8_t comTxBuffer[TX_BUFFER_SIZE];
RingBuffer pktRxBuffer;

// Based on version from: http://www.gaw.ru/pdf/Atmel/app/avr/AVR350.pdf
uint16_t calcrc(uint8_t *ptr, int count)
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
	while(!Serial_available())
	{
		now = Hal_millis();
		if( (now - lastTime) > timeOut ) return false;
	}
	*c = getchar();
	return true;
}

void appendCRC(uint8_t buffer[], uint8_t size)
{
	uint16_t crc = calcrc(buffer, size);
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
	putchar(0xAA);
	putchar(0x55);
	putchar(0xAA);
	putchar(0x55);
	// Data:
	for(i = 0; i < size; i++)
	{
		putchar(buffer[i]);
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
	// Adding received packet to buffer
	// If buffer is full, ignore packet (lost)
    if(!RingBuffer_isFull(&pktRxBuffer))
    {
    	uint8_t i = 0, j = 0;
    	RingBufferData data;
    	data.size = ind->size + 4;

    	data.data[j++] = CMD_DATA;
    	data.data[j++] = ind->lqi;
    	data.data[j++] = ind->rssi;
    	data.data[j++] = ind->size;
	    for(i = 0; i < ind->size; i++)
	    {
	        data.data[j++] = ind->data[i];
	    }
    	RingBuffer_insert(&pktRxBuffer, &data);
    }

}

bool serialHandler()
{
	uint8_t temp, command, size, data[MAX_FRAME_SIZE], longAddr[8];
	uint16_t pan, shortAddr, recCrc, calcCrc;
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
			//getting CRC:
			/*if(!getChTimeOut(&temp, TIMEOUT)) return false;
			recCrc = (temp << 8) & 0xFF00;
			if(!getChTimeOut(&temp, TIMEOUT)) return false;
			recCrc |= temp & 0x00FF;*/													// TERMINAR DE IMPLEMENTAR

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
    RingBuffer_init(&pktRxBuffer);
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
	if(!RingBuffer_isEmpty(&pktRxBuffer))
	{
		RingBufferData data;
		RingBuffer_remove(&pktRxBuffer, &data);
		sendBuffer(data.data, data.size);
	}
}
