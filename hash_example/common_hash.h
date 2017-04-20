/*
livello di astrazione per funzioni di basso livello.
Usabili da lato applicativo, e per la creazione di driver.

Cristian Pietrobon
*/


#ifdef __cplusplus
extern "C" {
#endif

#ifndef COMMON_HAL_HASH_H
#define COMMON_HAL_HASH_H

#include <stdint.h>
#include "md5.h"
#include "xxhash.h"

uint16_t CRC16 (void *nData, uint32_t wLength);

uint32_t HASH32(void* data, uint32_t lenght);
uint64_t HASH64(void* data, uint32_t lenght);

/*
typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} MD5_CTX;

extern void MD5_Init(MD5_CTX *ctx);
extern void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
extern void MD5_Final(unsigned char *result, MD5_CTX *ctx);
*/


#endif /*COMMON_HAL_HASH_H*/

#ifdef __cplusplus
} /* extern "C" */
#endif
