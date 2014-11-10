#include "Security.h"

int Security_encrypt(uint8_t *key, uint8_t *nonce, uint8_t *header, uint8_t headerLen, uint8_t *data, uint8_t dataLen, uint8_t *MUI, uint8_t MUILen)
{
	int result = ccm(
						key, SEC_KEYLEN,
						nonce, SEC_NONCELEN,
						header, headerLen,
						data, dataLen,
						data,
						MUI, &MUILen,
						CCM_ENCRYPT
					);

	if(result != CRYPT_OK) return -1;
	else return 0;
}

int Security_decrypt(uint8_t *key, uint8_t *nonce, uint8_t *header, uint8_t headerLen, uint8_t *data, uint8_t dataLen, uint8_t *MUI, uint8_t MUILen)
{
	uint8_t newMUI[MUILen];
	int result = ccm(
						key, SEC_KEYLEN,
						nonce, SEC_NONCELEN,
						header, headerLen,
						data, dataLen,
						data,
						newMUI, &MUILen,
						CCM_DECRYPT
					);

	int i;
	for(i = 0; i < MUILen; i++)
	{
		if(MUI[i] != newMUI[i]) return SEC_AUTH_ERROR;
	}

	if(result != CRYPT_OK) return -1;
	else return 0;
}

void Security_setNonce(uint8_t nonce[], uint8_t address[], uint32_t counter, uint8_t secLevel)
{

	memcpy(nonce, address, 8);
	memcpy(&nonce[8], &counter, 4);

	nonce[12] = secLevel;
}