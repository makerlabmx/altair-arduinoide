#ifndef CCM_H
#define CCM_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "gf256mul.h"
#include "aes.h"

#ifdef  __cplusplus
extern "C" {
#endif

//direction:
#define CCM_ENCRYPT 1
#define CCM_DECRYPT 0

//errors:
#define CRYPT_OK 0
#define CRYPT_INVALID_ARG -1

int ccm(
    const unsigned char *key,    uint8_t keylen,
    const unsigned char *nonce,  uint8_t noncelen,
    const unsigned char *header, uint8_t headerlen,
          unsigned char *data,     uint8_t datalen,
          unsigned char *encrypted,
          unsigned char *tag,    uint8_t *taglen,
                    int  direction);

#ifdef  __cplusplus
}
#endif

#endif /* CCM_H */