/* 
 * Based on LibTomCrypt -- Tom St Denis
 * Tom St Denis, tomstdenis@gmail.com, http://libtom.org
 * Modified by Rodrigo Méndez, rmendez@makerlab.mx
 */

#include "ccm.h"

/**
   CCM encrypt/decrypt and produce an authentication tag (MUI)
   @param key        The secret key to use
   @param keylen     The length of the secret key (octets)
   @param nonce      The session nonce [use once]
   @param noncelen   The length of the nonce
   @param header     The header for the session
   @param headerlen  The length of the header (octets)
   @param data         [out] The plaintext
   @param datalen      The length of the plaintext (octets)
   @param encrypted         [out] The ciphertext
   @param tag        [out] The destination tag (MUI)
   @param taglen     [in/out] The max size and resulting size of the authentication tag
   @param direction  Encrypt or Decrypt direction (0 or 1)
   @return CRYPT_OK if successful
*/

// Compatibility purposes, this code was originaly for 32 bit PCs, adapted to 8 bit with this.
#define INT_T unsigned char

int ccm(
    const unsigned char *key,    uint8_t keylen,
    const unsigned char *nonce,  uint8_t noncelen,
    const unsigned char *header, uint8_t headerlen,
          unsigned char *data,     uint8_t datalen,
          unsigned char *encrypted,
          unsigned char *tag,    uint8_t *taglen,
                    int  direction)
{
   INT_T  PAD[16], ctr[16], CTRPAD[16], b;
   aes128_ctx_t ctx;
   int            err=0;
   unsigned char  len, L, x, y, z, CTRlen;

   if ( (key == NULL) | (nonce == NULL) | (data == NULL) | (encrypted == NULL) | (tag == NULL) ) return CRYPT_INVALID_ARG;
   if (headerlen > 0 && header == NULL) return CRYPT_INVALID_ARG;

   /* make sure the taglen is even and <= 16 */
   *taglen &= ~1;
   if (*taglen > 16) {
      *taglen = 16;
   }

   /* can't use < 4 */
   if (*taglen < 4) {
      return CRYPT_INVALID_ARG;
   }

   /* let's get the L value */
   len = datalen;
   L   = 0;
   while (len) {
      ++L;
      len >>= 8;
   }
   if (L <= 1) {
      L = 2;
   }

   /* increase L to match the nonce len */
   noncelen = (noncelen > 13) ? 13 : noncelen;
   if ((15 - noncelen) > L) {
      L = 15 - noncelen;
   }

   /* decrease noncelen to match L */
   if ((noncelen + L) > 15) {
      noncelen = 15 - L;
   }

   /* init context where round keys are stored */
   aes128_init(key, &ctx);

   /* form B_0 == flags | Nonce N | l(m) */
   x = 0;
   PAD[x++] = (unsigned char)(((headerlen > 0) ? (1<<6) : 0) |
            (((*taglen - 2)>>1)<<3)        |
            (L-1));

   /* nonce */
   for (y = 0; y < (16 - (L + 1)); y++) {
       PAD[x++] = nonce[y];
   }

   /* store len */
   len = datalen;

   /* shift len so the upper bytes of len are the contents of the length */
   for (y = L; y < 4; y++) {
       len <<= 8;
   }

   /* store l(m) (only store 32-bits) */
   for (y = 0; L > 4 && (L-y)>4; y++) {
       PAD[x++] = 0;
   }
   for (; y < L; y++) {
       PAD[x++] = (unsigned char)((len >> (sizeof(INT_T) - 8)) & 255);
       len <<= 8;
   }

   /* encrypt PAD */
   aes128_enc(PAD, &ctx);

   /* handle header */
   if (headerlen > 0) {
      x = 0;
      
      /* store length */
      if (headerlen < ((1UL<<16) - (1UL<<8))) {
         PAD[x++] ^= (headerlen>>8) & 255;
         PAD[x++] ^= headerlen & 255;
      } 
      /*
      // Wont get here with headerLen defined as uint8_t
      else {
         PAD[x++] ^= 0xFF;
         PAD[x++] ^= 0xFE;
         PAD[x++] ^= (headerlen>>24) & 255;
         PAD[x++] ^= (headerlen>>16) & 255;
         PAD[x++] ^= (headerlen>>8) & 255;
         PAD[x++] ^= headerlen & 255;
      }*/

      /* now add the data */
      for (y = 0; y < headerlen; y++) {
          if (x == 16) {
             /* full block so let's encrypt it */
             aes128_enc(PAD, &ctx);
             x = 0;
          }
          PAD[x++] ^= header[y];
      }

      /* remainder? */
      if (x != 0) {
         aes128_enc(PAD, &ctx);
      }
   }

   /* setup the ctr counter */
   x = 0;

   /* flags */
   ctr[x++] = (unsigned char)L-1;
 
   /* nonce */
   for (y = 0; y < (16 - (L+1)); ++y) {
      ctr[x++] = nonce[y];
   }
   /* offset */
   while (x < 16) {
      ctr[x++] = 0;
   }

   x      = 0;
   CTRlen = 16;

   /* now handle the data */
   if (datalen > 0) {
      y = 0;

      for (; y < datalen; y++) {
          /* increment the ctr? */
          if (CTRlen == 16) {
             for (z = 15; z > 15-L; z--) {
                 ctr[z] = (ctr[z] + 1) & 255;
                 if (ctr[z]) break;
             }
             memcpy(CTRPAD, ctr, 16);
             aes128_enc(CTRPAD, &ctx);
             CTRlen = 0;
          }

          /* if we encrypt we add the bytes to the MAC first */
          if (direction == CCM_ENCRYPT) {
             b     = data[y];
             encrypted[y] = b ^ CTRPAD[CTRlen++];
          } else {
             b     = encrypted[y] ^ CTRPAD[CTRlen++];
             data[y] = b;
          }

          if (x == 16) {
             aes128_enc(PAD, &ctx);
             x = 0;
          }
          PAD[x++] ^= b;
      }
             
      if (x != 0) {
         aes128_enc(PAD, &ctx);
      }
   }

   /* setup CTR for the TAG (zero the count) */
   for (y = 15; y > 15 - L; y--) {
      ctr[y] = 0x00;
   }
   memcpy(CTRPAD, ctr, 16);
   aes128_enc(CTRPAD, &ctx);

   /* store the TAG */
   for (x = 0; x < 16 && x < *taglen; x++) {
       tag[x] = PAD[x] ^ CTRPAD[x];
   }
   *taglen = x;

   return err;
}
