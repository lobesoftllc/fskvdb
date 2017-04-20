#ifndef COMMON_FSKVDB_h
#define COMMON_FSKVDB_h

#ifdef __cplusplus
extern "C" {
#endif 

//#include "fskvdb_platform.h"
#include "fskvdb_common.h"
#include "fskvdb_queue.h"
  
static DB_PTR fskvdb_get_next_free_pos(fskvdb_ctx* ctx);
static void fskvdb_kvsearch_new_insertion_set(fskvdb_ctx* ctx,fskvdb_search_ctx* kvsearch,void* key,void* value,U32 value_size);

fskvdb_stat fskvdb_GetCtx(DBID DeviceID,fskvdb_ctx* ctx);
fskvdb_stat fskvdb_UpdateCtx(fskvdb_ctx* ctx);

/*DB*/
fskvdb_stat fskvdb_DBOpen(DBID DeviceID,fskvdb_ctx* ctx);
fskvdb_stat fskvdb_DBClose(fskvdb_ctx* ctx);
fskvdb_stat fskvdb_DBCreate(fskvdb_options_ctx* options , U32 version);
U32 fskvdb_DBGetUserVersion(fskvdb_ctx* ctx);

void fskvdb_ResetOptions(fskvdb_options_ctx* options);

/*is add/update*/
fskvdb_stat fskvdb_DBAdd(fskvdb_ctx* ctx, void* Key, void* value, U32 value_size, void* attribute);
fskvdb_stat fskvdb_DBDelete(fskvdb_ctx* ctx, void* Key);

/**/
fskvdb_stat fskvdb_DBGetValue(fskvdb_ctx* ctx , void* Key, void* value, U32* value_size);
fskvdb_stat fskvdb_DBGetValue_pos(fskvdb_ctx* ctx , U32 position,void* Key, void* value, U32* value_size);
fskvdb_stat fskvdb_DBGetQueueInfo(fskvdb_ctx* ctx, void* Key,fskvdb_queue_info* info);

fskvdb_stat fskvdb_DBGetAttribute(fskvdb_search_ctx* kvsearchctx);

/*internal?*/
fskvdb_stat fskvdb_DBSearchByKey(fskvdb_ctx* ctx, void* Key, fskvdb_search_ctx* kvsearchctx);

fskvdb_stat fskvdb_DataWrite(fskvdb_ctx* ctx, fskvdb_search_ctx* kvsearchctx, void* value, U32 value_size, void* attribute);

fskvdb_stat fskvdb_InOutEndOp(fskvdb_ctx* ctx);
fskvdb_stat fskvdb_DbWrite(fskvdb_ctx* ctx,void* value, U32 value_size,U32 position, U8 option);
fskvdb_stat fskvdb_DbRead(fskvdb_ctx* ctx,void* value, U32 value_size,U32 position);

DB_PTR fskvdb_Align(fskvdb_ctx* ctx, DB_PTR value,U32 newblock);
DB_PTR fskvdb_AlignAdd(fskvdb_ctx* ctx, DB_PTR value, U32 pos,U32 newblock);
fskvdb_stat fskvdb_PrepareCtx(fskvdb_ctx* ctx);

fskvdb_stat fskvdb_kv_check(fskvdb_ctx* ctx , void* Key,fskvdb_search_ctx* kvsearch,U8* tempbuffer);

//fskvdb_stat fskvdb_ReadStruct(DBID DeviceID, void* value, U32 value_size, U32 position );

U32 FSKVDBStrToKey(uint8_t* str);
U32 FSKVDBDataToKey(void* data,U32 size);

#ifdef __cplusplus
} /* extern "C" */
#endif 

#endif /*COMMON_LOGS_h*/
