#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef COMMON_HAL_HASH_md5_H
#define COMMON_HAL_HASH_md5_H

/*md5*/
/* Any 32-bit or wider unsigned integer data type will do */
typedef uint32_t MD5_u32plus;

typedef struct {
    MD5_u32plus lo, hi;
    MD5_u32plus a, b, c, d;
    unsigned char buffer[64];
    MD5_u32plus block[16];
} MD5_CTX;

void MD5_Init(MD5_CTX *ctx);
void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
void MD5_Final(unsigned char *result, MD5_CTX *ctx);

#endif /*COMMON_HAL_HASH_md5_H*/

#ifdef __cplusplus
} /* extern "C" */
#endif
