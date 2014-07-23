#ifndef SECURITY_H
#define SECURITY_H

#include <stdint.h>
#include "ccm.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define SEC_KEYLEN 16
#define SEC_NONCELEN 13
#define SEC_TAGLEN 4

#define SEC_AUTH_ERROR -2

int Security_encrypt(uint8_t *key, uint8_t *nonce, uint8_t *header, uint8_t headerLen, uint8_t *data, uint8_t dataLen, uint8_t *MUI, uint8_t MUILen);

int Security_decrypt(uint8_t *key, uint8_t *nonce, uint8_t *header, uint8_t headerLen, uint8_t *data, uint8_t dataLen, uint8_t *MUI, uint8_t MUILen);

void Security_setNonce(uint8_t nonce[], uint8_t address[], uint32_t counter, uint8_t secLevel);

#ifdef  __cplusplus
}
#endif

#endif /* SECURITY_H */