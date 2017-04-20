#ifndef COMMON_FSKVDB_PLAT_h
#define COMMON_FSKVDB_PLAT_h

#include "fskvdb_common.h"

#define fskvdbMAX(a,b) (((a)>(b))?(a):(b))

extern uint8_t* DbStringToOpen;

/*calculate the crc of a buffer*/
U16 fskvdb_crc16(void* buffer, U32 size);

U32 fskvdb_hash32(void* buffer, U32 size);

/*read write, device is the ID of the db context, and is set in the creation phase*/
fskvdb_stat fskvdb_open(DBID device);
fskvdb_stat fskvdb_close(DBID device);
fskvdb_stat fskvdb_endop(DBID device);

fskvdb_stat fskvdb_write(DBID device,void* buffer, U32 size, U32 position);
fskvdb_stat fskvdb_read(DBID device,void* buffer, U32 size, U32 position);
fskvdb_stat fskvdb_writefill(DBID device,void* buffer,U32 buffersize , U32 size, U32 position);
fskvdb_stat fskvdb_malloc(DBID device,void** ptr,U32 buffersize);
fskvdb_stat fskvdb_free(DBID device,void** ptr);

/*callback for IN/OUT op*/
fskvdb_stat fskvdb_callback(DBID device, U16 req_id , fskvdb_stat req_result ,U8* buffer, U32 size);


#endif /*COMMON_LOGS_h*/
