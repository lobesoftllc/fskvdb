#include "fskvdb_queue.h"

void fskvdb_queue_reset(fskvdb_search_ctx* dbsctx,fskvdb_queue_ctx* ctx){
	memset(ctx,0,sizeof(fskvdb_queue_ctx));
	dbsctx->User = ctx;
	
	/*modify kvsearch*/
	dbsctx->kvctx.ValuePos += sizeof(fskvdb_kv);
	
	ctx->queue_elem_max = dbsctx->ctx->QueueElemNum;
	ctx->queue_elem_size = ((dbsctx->ctx->FixedSize - sizeof(fskvdb_queue_ctx)) / dbsctx->ctx->QueueElemNum) - sizeof(fskvdb_queue_msg);
	ctx->dataposition = dbsctx->kvctx.ValuePos + sizeof(fskvdb_queue_ctx);
	ctx->queue_position = dbsctx->kvctx.ValuePos;
	
	ctx->queue_read_index = ctx->dataposition;
	
	DB_SET_CRC_type(ctx,fskvdb_queue_ctx);
}

fskvdb_stat fskvdb_queue_update_ctx(fskvdb_search_ctx* dbsctx){
	DB_SET_CRC_type(dbsctx->User,fskvdb_queue_ctx);
	return fskvdb_DbWrite(dbsctx->ctx,dbsctx->User,sizeof(fskvdb_queue_ctx),dbsctx->kvctx.ValuePos,KVDB_UPDATE);
}

fskvdb_stat fskvdb_queue_read_ctx(fskvdb_search_ctx* dbsctx,fskvdb_queue_ctx* ctx){
	dbsctx->User = ctx;
	
	/*modify kvsearch*/
	dbsctx->kvctx.ValuePos += sizeof(fskvdb_kv);
	
	fskvdb_stat ret = fskvdb_DbRead(dbsctx->ctx,ctx,sizeof(fskvdb_queue_ctx),dbsctx->kvctx.ValuePos);
	CHECH_RET_VAL();
	DB_CHECK_CRC_type(ctx,fskvdb_queue_ctx);
	
	return fskvdb_ok;
}

fskvdb_stat fskvdb_queue_add(fskvdb_search_ctx* dbsctx,void* data,U32 datasize){
	fskvdb_queue_ctx* ctx = (fskvdb_queue_ctx*)dbsctx->User;
	fskvdb_queue_msg msg;
	fskvdb_stat ret;
	
	if (data == 0) return fskvdb_err_param;
	
	if (ctx->queue_size >= ctx->queue_elem_max ) return fskvdb_full;
	
	msg.msg_size = datasize;
	msg.msg_crc = fskvdb_crc16(data,datasize);
	DB_SET_CRC(msg);
	
	/*get write position*/
	DB_PTR pos = ctx->dataposition + ctx->queue_next_elem;
	
	/*write data*/
	ret = fskvdb_DbWrite(dbsctx->ctx,&msg,sizeof(msg),pos,KVDB_UPDATE);
	CHECH_RET_VAL();
	ret = fskvdb_DbWrite(dbsctx->ctx,data,datasize,pos + sizeof(fskvdb_queue_msg),KVDB_UPDATE);
	CHECH_RET_VAL();
	
	ctx->queue_size++;
	ctx->queue_next_elem += (sizeof(fskvdb_queue_msg) + ctx->queue_elem_size);
	
	return fskvdb_queue_update_ctx(dbsctx);
}

fskvdb_stat fskvdb_queue_delete(fskvdb_search_ctx* dbsctx){
	fskvdb_queue_ctx* ctx = (fskvdb_queue_ctx*)dbsctx->User;
	//fskvdb_stat ret;
	
	if (ctx->queue_size <= 0 ) return fskvdb_kvempty;
	
	/*point next block of data*/
	ctx->queue_read_index += sizeof(fskvdb_queue_msg) + ctx->queue_elem_size;
	ctx->queue_size--;
	
	if (ctx->queue_size <= 0){
		ctx->queue_read_index = ctx->dataposition;
		ctx->queue_next_elem = 0;
	}
	
	return fskvdb_queue_update_ctx(dbsctx);
}

fskvdb_stat fskvdb_queue_get(fskvdb_search_ctx* dbsctx,void* data,U32* datasize){
	if (data == 0) return fskvdb_err_param;
	if (datasize == 0) return fskvdb_err_param;
	
	*datasize = 0;
	
	fskvdb_queue_ctx* ctx = (fskvdb_queue_ctx*)dbsctx->User;
	fskvdb_queue_msg msg;
	fskvdb_stat ret;
	
	if (ctx->queue_size <= 0) return fskvdb_kvempty;
	
	ret = fskvdb_DbRead(dbsctx->ctx,&msg,sizeof(msg),ctx->queue_read_index);
	CHECH_RET_VAL();
	DB_CHECK_CRC(msg);
	
	ret = fskvdb_DbRead(dbsctx->ctx,data,msg.msg_size,ctx->queue_read_index + sizeof(fskvdb_queue_msg));
	CHECH_RET_VAL();
	
	/*check crc msg*/
	if (msg.msg_crc != fskvdb_crc16(data,msg.msg_size)){
		return fskvdb_datacrc_error;
	}
	
	*datasize = msg.msg_size;
	
	return fskvdb_ok;
}

fskvdb_stat fskvdb_queue_get_position(fskvdb_search_ctx* dbsctx,U32 position,void* data,U32* datasize){
	if (data == 0) return fskvdb_err_param;
	if (datasize == 0) return fskvdb_err_param;
	
	*datasize = 0;
	
	fskvdb_queue_ctx* ctx = (fskvdb_queue_ctx*)dbsctx->User;
	fskvdb_queue_msg msg;
	fskvdb_stat ret;
	
	/*calculate position*/
	ctx->queue_read_index += (sizeof(fskvdb_queue_msg) + ctx->queue_elem_size) * position;
        
	if (ctx->queue_size <= 0) return fskvdb_kvempty;
	if (ctx->queue_read_index - ctx->dataposition >= 
		ctx->queue_size * (sizeof(fskvdb_queue_msg) + ctx->queue_elem_size)) return fskvdb_index_error;
	
	ret = fskvdb_DbRead(dbsctx->ctx,&msg,sizeof(msg),ctx->queue_read_index);
	CHECH_RET_VAL();
	DB_CHECK_CRC(msg);
	
	ret = fskvdb_DbRead(dbsctx->ctx,data,msg.msg_size,ctx->queue_read_index + sizeof(fskvdb_queue_msg));
	CHECH_RET_VAL();
	
	/*check crc msg*/
	if (msg.msg_crc != fskvdb_crc16(data,msg.msg_size)){
		return fskvdb_datacrc_error;
	}
	
	*datasize = msg.msg_size;
	
	return fskvdb_ok;
}

fskvdb_stat fskvdb_queue_getInfo(fskvdb_search_ctx* dbsctx,fskvdb_queue_info* info){
	fskvdb_queue_ctx* ctx = (fskvdb_queue_ctx*)dbsctx->User;
	info->queue_msg_num = ctx->queue_size;
	return fskvdb_ok;
}