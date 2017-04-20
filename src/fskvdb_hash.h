#ifndef COMMON_FSKVDB_HASH_h
#define COMMON_FSKVDB_HASH_h


#include "fskvdb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

fskvdb_stat fskvdb_createHashTable(fskvdb_ctx* ctx,U32 TableSize, U32 ListSize, U16 ByteSearchValue, U32 write_position, fskvdb_table_ctx* hashctx);
fskvdb_stat fskvdb_readHashCtx(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx, U32 position);
fskvdb_stat fskvdb_hashGetKeyPosition(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx, void* key, fskvdb_table_ctx_search* searchctx);

fskvdb_stat fskvdb_hashDeleteKey(fskvdb_search_ctx* searchctx);
fskvdb_stat fskvdb_hashAddKey(fskvdb_search_ctx* searchctx);

fskvdb_stat fskvdb_hashGetTablePos(fskvdb_table_ctx* hashctx ,void* key , U32* position);

U32 fskvdb_hash_TableSize(fskvdb_table_ctx* hashctx);

fskvdb_stat fskvdb_hashTableCheck(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx,uint32_t maxconsecutiverr,uint8_t printcrcerror,U8* kvreadbuffer,uint8_t verbose);

fskvdb_stat fskvdb_hashMapReduce_init(fskvdb_ctx* ctx,fskvdb_mapreduce_ctx* mrctx);
fskvdb_stat fskvdb_hashMapReduce_nextkey(fskvdb_ctx* ctx,fskvdb_mapreduce_ctx* mrctx,void* key);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*COMMON_FSKVDB_HASH_h*/
