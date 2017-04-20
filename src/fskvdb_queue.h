#ifndef COMMON_FSKVDB_TYPEQUEUE_h
#define COMMON_FSKVDB_TYPEQUEUE_h

#include "fskvdb_common.h"

#pragma pack(push, 1) // exact fit - no padding

typedef struct fskvdb_queue_msg{
	U32 msg_size;
	U16 msg_crc;
	
	U16 crc;
}fskvdb_queue_msg;

typedef struct fskvdb_queue_ctx{
	U32 queue_size;
	DB_PTR queue_next_elem;
	DB_PTR queue_read_index;
	U32 queue_elem_size; // the size max of each element on the queue
	U32 queue_elem_max; // the max number of element on the queue
	DB_PTR dataposition; //position of the data
	DB_PTR queue_position; //position of the queue
	
	U16 crc;
}fskvdb_queue_ctx;

#pragma pack(pop) //back to whatever the previous packing mode was

typedef struct fskvdb_queue_info{
	U32 queue_msg_num;
}fskvdb_queue_info;

void fskvdb_queue_reset(fskvdb_search_ctx* dbsctx,fskvdb_queue_ctx* ctx);
fskvdb_stat fskvdb_queue_update_ctx(fskvdb_search_ctx* dbsctx);
fskvdb_stat fskvdb_queue_read_ctx(fskvdb_search_ctx* dbsctx,fskvdb_queue_ctx* ctx);

fskvdb_stat fskvdb_queue_add(fskvdb_search_ctx* dbsctx,void* data,U32 datasize);
fskvdb_stat fskvdb_queue_delete(fskvdb_search_ctx* dbsctx);
fskvdb_stat fskvdb_queue_get(fskvdb_search_ctx* dbsctx,void* data,U32* datasize);
fskvdb_stat fskvdb_queue_get_position(fskvdb_search_ctx* dbsctx,U32 position,void* data,U32* datasize);
fskvdb_stat fskvdb_queue_getInfo(fskvdb_search_ctx* dbsctx,fskvdb_queue_info* info);

#endif /*COMMON_FSKVDB_LINEARAUTO_h*/
