#include "phy.h"

static uint8_t flagGotRx, flagGotTx;
static uint8_t txStatus;
static PHY_DataInd_t rxRawData;

void Phy_init(Phy *self)
{
	PHY_Init();
    ID_init();

    self->securityEnabled = PHY_SECURITY_ENABLED;

    #if PHY_SECURITY_ENABLED == 1

    //init framecount with random number
    srand(PHY_RANDOM_SEED);
    self->frameCount = rand();
    self->frameCount |= ((uint32_t)rand() << 16);

    #endif

    // Max tx power by default
    // Correct band by default
    // 3 retries by default
    Phy_setChannel(self, 11);

    //setup local address
    Phy__loadLocalAddress(self);
    PHY_SetRxState(true);

}

void Phy_setChannel(Phy *self, uint8_t channel)
{
    PHY_SetChannel(channel);
}

void Phy_setPan(Phy *self, uint16_t pan)
{
    PHY_SetPanId(pan);
    self->localPan = pan;
}

void Phy_setRx(Phy *self, bool enabled)
{
    PHY_SetRxState(enabled);
}

void Phy_setTxPower(Phy *self, uint8_t power)
{
    PHY_SetTxPower(power);
}

bool Phy_busy(Phy *self)
{
    self->busy = PHY_Busy();
	return self->busy;
}

void Phy_sleep(Phy *self)
{
    PHY_Sleep();
}

void Phy_wakeup(Phy *self)
{
	PHY_Wakeup();
}

void Phy_send(Phy *self, uint8_t *dest, uint8_t *data, uint8_t dataLen)
{
    // if security, encrypt data
    // Prepare headers
    // Send with:
    //  PHY_DataReq(uint8_t *data, uint8_t size)

    //wait for any pending tx to finish
    unsigned long lastTime = Hal_millis();
    unsigned long now = lastTime;
    while(Phy_busy(self) && (lastTime + TX_TIMEOUT) > now) now = Hal_millis();

    bool isBroadcast = Phy__isBroadcast(dest);
    uint8_t i, j;
    i = j = 0;
    uint8_t frame[128];

    self->txPacket.dataLen = dataLen;

    for(j = 0; j < dataLen; j++)
    {
        self->txPacket.data[j] = data[j];
    }

    if(isBroadcast)
    {
        //frame[i++] = (MAC_BROADCAST_HEADER_SIZE); // header length
        //frame[i++] = (MAC_BROADCAST_HEADER_SIZE + self->txPacket.dataLen + MUI_SIZE);    //Total length
        frame[i++] = (FRAME_CONTROL_BROADCAST_B1); // first byte of Frame Control
        frame[i++] = (FRAME_CONTROL_BROADCAST_B2); // second byte of frame control
    }
    else
    {
        //frame[i++] = (MAC_HEADER_SIZE); // header length
        //frame[i++] = (MAC_HEADER_SIZE + self->txPacket.dataLen + MUI_SIZE);    //Total length
        frame[i++] = (FRAME_CONTROL_B1); // first byte of Frame Control
        frame[i++] = (FRAME_CONTROL_B2); // second byte of frame control
    }
   
    frame[i++] = (1);  // sequence number 1

    frame[i++] = (self->localPan & 0xff);  // dest panid
    frame[i++] = (self->localPan >> 8);

    if(isBroadcast)
    {
        // Set 16 bit dest address as broatcast
        frame[i++] = (0xFF);
        frame[i++] = (0xFF);
    }
    else
    {
        for(j = 0; j < DEST_ADDRESS_SIZE ; j++)    //LSB first, MSB last
        {
            frame[i++] = (dest[j]);          //dest Address
        }
    }
    

    for(j = 0; j < SRC_ADDRESS_SIZE; j++)
    {
        frame[i++] = (self->localAddress[j]);          //src Address
    }

    #if PHY_SECURITY_ENABLED == 1
    //Security Header
    frame[i++] = (SECURITY_CONTROL_B1);  //Security Control
    for(j = 0; j < (SECURITY_HEADER_SIZE - 1); j++)
    {
        frame[i++] = ((self->frameCount >> 8*j) & 0xff );    //Frame counter
    }

    uint8_t nonce[SEC_NONCELEN];
    uint8_t MUI[MUI_SIZE];

    Security_setNonce(nonce, self->localAddress, self->frameCount, SECURITY_LEVEL);
    uint8_t muiSize = MUI_SIZE;
    Security_encrypt(self->secKey, nonce, NULL, 0, self->txPacket.data, self->txPacket.dataLen, MUI, muiSize);

    #endif

    //Append Data Payload
    for (j = 0; j < self->txPacket.dataLen; j++) {
        frame[i++] = (self->txPacket.data[j]);
    }

    #if PHY_SECURITY_ENABLED == 1
    //Append MUI
    for (j = 0; j < MUI_SIZE; j++) {
        frame[i++] = (MUI[j]);
    }
    #endif
    
    PHY_DataReq(frame, i);

    #if PHY_SECURITY_ENABLED == 1
    //increase frame count.
    self->frameCount++;
    #endif
}

#if PHY_SECURITY_ENABLED == 1
void Phy_setSecKey(Phy *self, uint8_t *key)
{
    int i;
    for(i = 0; i < SEC_KEYLEN; i++)
    {
        self->secKey[i] = key[i];
    }
}
#endif

//Private:
void Phy__loadLocalAddress(Phy *self)
{
	uint8_t chipId[SRC_ADDRESS_SIZE];
    if( !ID_getId(chipId) ) return;        //Error
    Phy__setLocalAddress(self, chipId);
}

void Phy__setLocalAddress(Phy *self, uint8_t *address)
{
	int i;

	//for Extended address (64bit/8Byte)
	for(i = 0; i < SRC_ADDRESS_SIZE; i++)
	{
		self->localAddress[i] = address[i];
	}

    PHY_SetLongAddr(address);

}

void Phy__setPromiscuous(Phy *self, bool prom)
{
	PHY_SetPromiscuous(prom);
}

void Phy__retrieveRx(Phy *self)
{
    self->rxPacket.error = false;
	uint8_t frameLength = rxRawData.size;
    self->rxPacket.frameLen = frameLength;

    // get frame control:
    self->rxPacket.frameControl = rxRawData.data[MAC_FRAME_CONTROL];
    self->rxPacket.frameControl += (uint16_t)rxRawData.data[MAC_FRAME_CONTROL + 1] << 8;

    int i, destIndex, srcIndex, secIndex, headerSize, destSize;

    if( ((self->rxPacket.frameControl >> (8+DEST_ADDR_MODE_B)) & 0x03) == DEST_ADDR_MODE_BROADCAST)
    {
        headerSize = MAC_BROADCAST_HEADER_SIZE;
        destIndex = MAC_DEST_ADDRESS;
        srcIndex = MAC_SRC_ADDRESS - 6;
        secIndex = MAC_SECURITY_HEADER - 6;
        destSize = 2;
    }
    else
    {
        headerSize = MAC_HEADER_SIZE;
        destIndex = MAC_DEST_ADDRESS;
        srcIndex = MAC_SRC_ADDRESS;
        secIndex = MAC_SECURITY_HEADER;
        destSize = DEST_ADDRESS_SIZE;
    }

    self->rxPacket.dataLen = frameLength - headerSize /*- FCS_SIZE*/;

    //buffer seq number:
    self->rxPacket.seqNumber = rxRawData.data[MAC_SEQ_NUMBER];

    //read dest PAN:
    self->rxPacket.destPAN = rxRawData.data[MAC_PAN];
    self->rxPacket.destPAN += (uint16_t)rxRawData.data[MAC_PAN + 1] << 8;

    //for Extended address (64bit/8Byte)
    //read dest Address
    for(i = 0; i < destSize; i++)
    {
        self->rxPacket.destAddress[i] = rxRawData.data[destIndex + i];
    }
    //read src Address
    for(i = 0; i < SRC_ADDRESS_SIZE; i++)
    {
        self->rxPacket.srcAddress[i] = rxRawData.data[srcIndex + i];
    }

    #if PHY_SECURITY_ENABLED == 1
    self->rxPacket.dataLen -= MUI_SIZE;
    //read security header
    for(i = 0; i < SECURITY_HEADER_SIZE; i++)
    {
        self->rxPacket.securityHeader[i] = rxRawData.data[secIndex + i];
    }

    self->rxPacket.secLevel = self->rxPacket.securityHeader[0];
    self->rxPacket.secFrameCount = 0x00000000;
  
    for(i=0; i < 4; i++)
    {
        self->rxPacket.secFrameCount |= ((uint32_t)self->rxPacket.securityHeader[1 + i]) << 8*i;  
    }

    #endif

    // buffer data bytes
    // from (headerSize) to (frameLength - bytes_nodata - 1)
    for (i = 0; i < self->rxPacket.dataLen; i++) 
    {
        self->rxPacket.data[i] = rxRawData.data[headerSize + i];
    }

    #if PHY_SECURITY_ENABLED == 1
    uint8_t nonce[SEC_NONCELEN];
    for(i = 0; i < MUI_SIZE; i++)
    {
        self->rxPacket.MUI[i] = rxRawData.data[headerSize + self->rxPacket.dataLen + i];
    }

    Security_setNonce(nonce, self->rxPacket.srcAddress, self->rxPacket.secFrameCount, self->rxPacket.secLevel);
    uint8_t muiSize = MUI_SIZE;
    if(Security_decrypt(self->secKey, nonce, NULL, 0, self->rxPacket.data, self->rxPacket.dataLen, self->rxPacket.MUI, muiSize) == SEC_AUTH_ERROR) self->rxPacket.error = true;

    #endif

    self->rxPacket.lqi = rxRawData.lqi;
    self->rxPacket.rssi = rxRawData.rssi;
}

void Phy_loop(Phy *self)
{
    PHY_TaskHandler();
    if(flagGotTx)
    {
        flagGotTx = 0;
        if(txStatus == TRAC_STATUS_SUCCESS || txStatus == TRAC_STATUS_SUCCESS_DATA_PENDING || txStatus == TRAC_STATUS_SUCCESS_WAIT_FOR_ACK)
            self->txPacket.txOk = true;
        else
            self->txPacket.txOk = false;

        if(txStatus == TRAC_STATUS_NO_ACK)
            self->txPacket.retries = 3;
        else
            self->txPacket.retries = 0;

        if(txStatus == TRAC_STATUS_CHANNEL_ACCESS_FAILURE)
            self->txPacket.channelBusy = true;
        else
            self->txPacket.channelBusy = false;

        if(self->txHandler != NULL) self->txHandler();
    }

    if(flagGotRx)
    {
        flagGotRx = 0;
        Phy__retrieveRx(self);

        if(self->rxHandler != NULL) self->rxHandler();
    }
}

bool Phy__isBroadcast(uint8_t *address)
{
    int i;
    for(i = 0; i < DEST_ADDRESS_SIZE; i++)
    {
        if(address[i] != Phy_broadcast[i]) return false;
    }
    return true;
}

// Callbacks from phyrfr2
// Called on tx end, gets if succeded or not
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
    txStatus = status;
    flagGotTx = 1;
}

// Called on rx success, gets rx data
void PHY_DataInd(PHY_DataInd_t *ind)
{
    rxRawData.size = ind->size;
    rxRawData.lqi  = ind->lqi;
    rxRawData.rssi = ind->rssi;
    rxRawData.data = ind->data;

    flagGotRx = 1;
}

