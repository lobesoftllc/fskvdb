#include "fskvdb.h"
#include "fskvdb_hash.h"
#include "fskvdb_queue.h"
#include <string.h>
#include <stdlib.h>

static fskvdb_stat fskvdb_DataAddedPost(fskvdb_ctx* ctx, fskvdb_search_ctx* kvsearchctx);
static U32 fskvdb_GetWritePosKvS(fskvdb_ctx* ctx, fskvdb_search_ctx* kvsearchctx);
static fskvdb_stat fskvdb_WriteKvCtx(fskvdb_ctx* ctx, fskvdb_search_ctx* kvsearchctx);

static DB_PTR fskvdb_get_next_free_pos(fskvdb_ctx* ctx){
	if (ctx->BucketType == fskvdb_BucketType_blob)
		return ctx->NextFreePos + sizeof(fskvdb_kv) + ctx->AttributeLenght;
	else
		return ctx->NextFreePos + ctx->AttributeLenght;
}

static void fskvdb_kvsearch_new_insertion_set(fskvdb_ctx* ctx,fskvdb_search_ctx* kvsearch,void* key,void* value,U32 value_size){
	     /*insert*/
     kvsearch->position = 0;
     //kvsearch.kvctx.Key = Key;
	 DB_SET_KEY_TO_KVC(kvsearch->kvctx.Key,key);
     kvsearch->kvctx.NextValue = 0;
     //kvctx.Status = 0;
     kvsearch->kvctx.ValueSize = value_size;
     /*set the value position*/
     kvsearch->kvctx.ValuePos = fskvdb_get_next_free_pos(ctx);

     if (ctx->FixedSize != 0)
         kvsearch->kvctx.SizeBlock = ctx->FixedSize;
         else
         kvsearch->kvctx.SizeBlock = value_size;

     SET_KV_CRC(kvsearch->kvctx);
}

/*DB*/
fskvdb_stat fskvdb_DBOpen(U16 DeviceID,fskvdb_ctx* ctx){

   fskvdb_stat ret = fskvdb_open(DeviceID);
   CHECH_RET_VAL_S_CTX();

   //fskvdb_ctx ctx;
   ret = fskvdb_GetCtx(DeviceID,ctx);
   CHECH_RET_VAL_S_CTX();
   CHECH_CTX_CRC();
   
   ctx->LastErr = 0;

  if (ctx->Ready == 0)
	  EXIT_ERROR_CODE(fskvdb_db_notready);
	  
  if (ctx->DeviceID != DeviceID)
      EXIT_ERROR_CODE(fskvdb_invalid_ctx);

  //fskvdb_InOutEndOp(&ctx);
  EXIT_ERROR_CODE(fskvdb_ok);
}

U32 fskvdb_DBGetUserVersion(fskvdb_ctx* ctx){
  return ctx->UserVersion;
}

fskvdb_stat fskvdb_DBClose(fskvdb_ctx* ctx){
   fskvdb_stat ret = fskvdb_close(ctx->DeviceID);
   CHECH_RET_VAL_S_CTX();
   return (ret);
}

fskvdb_stat fskvdb_DBCreate(fskvdb_options_ctx* options , U32 version){

       fskvdb_ctx ctx;
       memset(&ctx,0,sizeof(fskvdb_ctx));
	   ctx.UserVersion = version;
	   ctx.AttributeLenght = options->AttributeLenght;
	   ctx.DeviceID = options->DeviceID;
	   ctx.KeyNumber = 0;
	   ctx.NextFreePos = sizeof(fskvdb_ctx);
	   ctx.KeyIsString = options->KeyIsString;
	   ctx.KeySize = options->KeySize;
	   ctx.BucketType = options->BucketType;	
	   ctx.Version = FSKVDB_VERSION;
	   ctx.Ready = 0;
		
		if (ctx.KeySize > FSKVDB_MAX_KEY_SIZE){
			return fskvdb_err_param;
		}
		
	    //ctx.MemoryInOut = 0;

       /*calculate the table list*/
       if (options->DataAlign != 0)
				  options->SearchTableSize = options->DataAlign / (sizeof(U32)+sizeof(fskvdb_table_list_footer));
            else
				  options->SearchTableSize = 32; /*TODO*/

       /*optimized the table search size*/
        if (options->DataMaxNum != 0)
                options->SearchTableEntry = (options->DataMaxNum / FSKVDB_HASH_DIVISOR)+1; /**/
            else
                options->SearchTableEntry = 32000; /*default one*/

       if (options->FixedLen != FSKVDB_OPT_KVSIZEMAXCLUSTER)
            ctx.FixedSize = options->FixedLen;
       else{
            if (options->DataAlign != 0)
            ctx.FixedSize = options->DataAlign - (sizeof(fskvdb_kv) + options->AttributeLenght);
            else
                ctx.FixedSize = 255; /*default block*/
       }
	   
	   		/*check for buckettype*/
		if (ctx.BucketType == fskvdb_BucketType_queue){
			/*set attribute as queue ctx*/
			//ctx.AttributeLenght = sizeof(fskvdb_queue_ctx);
			options->ElemSizeQueue = ctx.FixedSize;
			ctx.QueueElemSize = ctx.FixedSize;
			ctx.FixedSize = ((options->ElemSizeQueue + sizeof(fskvdb_queue_msg))*options->ElemNumQueue) + sizeof(fskvdb_queue_ctx);
			ctx.QueueElemNum = options->ElemNumQueue;
		}else
			ctx.QueueElemSize = ctx.FixedSize;
	   
	   /*static allocation if needed*/
	   /*if (options->UseMallocForMemory){
	   if (options->FixedLen != 0 && options->DataMaxNum !=0){
		   fskvdb_malloc(ctx.DeviceID,&ctx.MemoryInOut,);
	   }
	   
	   }*/

       ctx.SearchMode = options->SearchIndexMode;
       ctx.Status = 0 ;
       ctx.Align = options->DataAlign;

  ctx.ManagerPtrIn.memory = 0;
  ctx.ManagerPtrIn.memorysize = 0;
  ctx.ManagerPtrIn.position = 0;

  ctx.ManagerPtrOut.memory = 0;
  ctx.ManagerPtrOut.memorysize = 0;
  ctx.ManagerPtrOut.position = 0;


       fskvdb_stat ret = fskvdb_open(ctx.DeviceID);
       CHECH_RET_VAL();

       if (options->DataCrc)
           ctx.Status |= fskvdb_opt_datacrc;
        
       ret = fskvdb_UpdateCtx(&ctx);
       CHECH_RET_VAL();
       ret = fskvdb_InOutEndOp(&ctx);
       CHECH_RET_VAL();
       ret = fskvdb_GetCtx(options->DeviceID,&ctx);
       CHECH_RET_VAL();


       if (ctx.SearchMode == fskvdb_SearchIndexMode_hash){
            fskvdb_table_ctx hashctx;
            /*create the index for the main key*/
            ret = fskvdb_createHashTable(&ctx,options->SearchTableEntry,options->SearchTableSize, /*sizeof(U32)*/ ctx.KeySize ,ctx.NextFreePos,&hashctx);
            CHECH_RET_VAL();
            //ctx.NextFreePos = fskvdb_Align(&ctx,hashctx.TableByteSize + sizeof(fskvdb_ctx) ,1);
			ctx.NextFreePos = fskvdb_Align(&ctx,hashctx.TableByteSize + ctx.NextFreePos + sizeof(fskvdb_table_ctx),1);
       }
	
	 ctx.Ready = 1; /*DB is ready*/
    ret = fskvdb_UpdateCtx(&ctx);
    CHECH_RET_VAL();

	fskvdb_InOutEndOp(&ctx);
       EXIT_ERROR_CODE_noptr(fskvdb_ok);
}

/*kv filter*/
//static fskvdb_stat fskvdb_action_filter(fskvdb_ctx* ctx, void* Key, void* value, U32 value_size, void* attribute,fskvdb_search_ctx *kvsearch,fskvdb_stat ret){
	
//}

/*is add/update*/
fskvdb_stat fskvdb_DBAdd(fskvdb_ctx* ctx, void* Key, void* value, U32 value_size, void* attribute){
   /*check for fixed size*/
   if (ctx->FixedSize != 0)
       if (value_size > ctx->FixedSize || value_size > ctx->QueueElemSize)
           EXIT_ERROR_CODE(fskvdb_2big);

   //fskvdb_kv kvctx;
   fskvdb_stat ret;
   fskvdb_search_ctx kvsearch;
   memset(&kvsearch,0,sizeof(fskvdb_search_ctx));
   
   ret = fskvdb_DBSearchByKey(ctx,Key,&kvsearch);

   switch(ret){
   case fskvdb_ok:
     /*update*/
     //ret = fskvdb_write(ctx->DeviceID,
     kvsearch.kvctx.ValueSize = value_size;
     SET_KV_CRC(kvsearch.kvctx);

if (ctx->BucketType == fskvdb_BucketType_blob){
	
     ret = fskvdb_WriteKvCtx(ctx,&kvsearch);
     CHECH_RET_VAL_S_CTX();	
	
     ret = fskvdb_DataWrite(ctx,&kvsearch,value,value_size,attribute);
     CHECH_RET_VAL_S_CTX();

     if (kvsearch.tablesearch.ret != fskvdb_kvdeleted)
        ret = fskvdb_updated;
}else
if (ctx->BucketType == fskvdb_BucketType_queue){
		fskvdb_queue_ctx queuectx;
		ret = fskvdb_queue_read_ctx(&kvsearch,&queuectx);
		CHECH_RET_VAL_S_CTX();
		ret = fskvdb_queue_add(&kvsearch,value,value_size);
		CHECH_RET_VAL_S_CTX();
}
		ret = fskvdb_DataAddedPost(ctx,&kvsearch);
		CHECH_RET_VAL_S_CTX();
		
		EXIT_ERROR_CODE(fskvdb_ok);
     break;

   case fskvdb_noexists:
     /*insert*/
	 
		fskvdb_kvsearch_new_insertion_set(ctx,&kvsearch,Key,value,value_size);
		
		ret = fskvdb_WriteKvCtx(ctx,&kvsearch);
		CHECH_RET_VAL_S_CTX();
		
		if (ctx->BucketType == fskvdb_BucketType_blob){
			ret = fskvdb_DataWrite(ctx,&kvsearch,value,value_size,attribute);
			CHECH_RET_VAL_S_CTX();
		}
		else 
		if (ctx->BucketType == fskvdb_BucketType_queue){
			
			fskvdb_queue_ctx queuectx;
			fskvdb_queue_reset(&kvsearch,&queuectx);
			ret = fskvdb_queue_update_ctx(&kvsearch);
			CHECH_RET_VAL_S_CTX();
			ret = fskvdb_queue_add(&kvsearch,value,value_size);
			CHECH_RET_VAL_S_CTX();
		}
		
			ret = fskvdb_DataAddedPost(ctx,&kvsearch);
			CHECH_RET_VAL_S_CTX();

     break;

   default:
     EXIT_ERROR_CODE(ret);
   }

   //fskvdb_InOutEndOp(&ctx);
   EXIT_ERROR_CODE(ret);
}

fskvdb_stat fskvdb_DBDelete(fskvdb_ctx* ctx, void* Key){
   fskvdb_stat ret;
   fskvdb_search_ctx kvsearch;
   ret = fskvdb_DBSearchByKey(ctx,Key,&kvsearch);

   if (ret == fskvdb_ok){
       if (kvsearch.tablesearch.ret == fskvdb_kvdeleted)
           EXIT_ERROR_CODE(fskvdb_noexists);

       switch(ctx->SearchMode){
       case fskvdb_SearchIndexMode_hash:
			
			if (ctx->BucketType == fskvdb_BucketType_queue){
				
				fskvdb_queue_ctx queuectx;
				ret = fskvdb_queue_read_ctx(&kvsearch,&queuectx);
				CHECH_RET_VAL_S_CTX();
				ret = fskvdb_queue_delete(&kvsearch);
				CHECH_RET_VAL_S_CTX();
				
			}else
			if (ctx->BucketType == fskvdb_BucketType_blob){
				
				ret = fskvdb_hashDeleteKey(&kvsearch);
				
			}else
				EXIT_ERROR_CODE(fskvdb_interr); /*TODO*/
        break;

        case fskvdb_SearchIndexMode_linear:
            EXIT_ERROR_CODE(fskvdb_interr); /*TODO*/
        break;

        default:
            EXIT_ERROR_CODE(fskvdb_interr);
       }
   }else
   EXIT_ERROR_CODE(ret);

   //fskvdb_InOutEndOp(&ctx);
   EXIT_ERROR_CODE(ret);
}

static U32 fskvdb_GetWritePosKvS(fskvdb_ctx* ctx, fskvdb_search_ctx* kvsearchctx){
	if (kvsearchctx->position == 0)
        return ctx->NextFreePos;
	 else
        return kvsearchctx->position;
}

static fskvdb_stat fskvdb_DataAddedPost(fskvdb_ctx* ctx, fskvdb_search_ctx* kvsearchctx){
	     /*new insert*/
	  fskvdb_stat ret;
	  U32 write_pos = fskvdb_GetWritePosKvS(ctx,kvsearchctx);
	  
     if (kvsearchctx->position == 0 /*&& ctx->BucketType == fskvdb_BucketType_blob*/ ){

        /*the kv position*/
        kvsearchctx->position = write_pos;

		ctx->KeyNumber+=1;
		//ctx->NextFreePos += sizeof(fskvdb_kv) + ctx->AttributeLenght + ctx->FixedSize;
		ctx->NextFreePos = fskvdb_AlignAdd(ctx,ctx->NextFreePos,sizeof(fskvdb_kv) + ctx->AttributeLenght + ctx->FixedSize,0);
		ctx->LastKey = kvsearchctx->kvctx.Key;
		ret = fskvdb_UpdateCtx(ctx);
		CHECH_RET_VAL_S_CTX();

     }

          /*hash table management*/
     if (ctx->SearchMode == fskvdb_SearchIndexMode_hash){
        ret = fskvdb_hashAddKey(kvsearchctx);
        CHECH_RET_VAL_S_CTX();
     }
	 
	 ret = fskvdb_InOutEndOp(ctx);
	 return ret;
}

static fskvdb_stat fskvdb_WriteKvCtx(fskvdb_ctx* ctx, fskvdb_search_ctx* kvsearchctx){
	     U32 write_pos = fskvdb_GetWritePosKvS(ctx,kvsearchctx);
		 fskvdb_stat ret = fskvdb_DbWrite(ctx,&kvsearchctx->kvctx,sizeof(fskvdb_kv),write_pos,KVDB_UPDATE);
        CHECH_RET_VAL_S_CTX();
		 return ret;
}

fskvdb_stat fskvdb_DataWrite(fskvdb_ctx* ctx, fskvdb_search_ctx* kvsearchctx, void* value, U32 value_size, void* attribute){

     fskvdb_stat ret;
	  U32 write_pos = fskvdb_GetWritePosKvS(ctx,kvsearchctx);

    if (write_pos == 0)
        return fskvdb_interr;

        //ret = fskvdb_DbWrite(ctx,&kvsearchctx->kvctx,sizeof(fskvdb_kv),write_pos,KVDB_UPDATE);
        //CHECH_RET_VAL();


     if (ctx->AttributeLenght != 0 && attribute != 0){
        ret = fskvdb_DbWrite(ctx,attribute,ctx->AttributeLenght,write_pos + sizeof(fskvdb_kv),KVDB_UPDATE);
        CHECH_RET_VAL_S_CTX();
     }

     ret = fskvdb_DbWrite(ctx,value,value_size,write_pos + sizeof(fskvdb_kv),KVDB_UPDATE);

     CHECH_RET_VAL_S_CTX();

     return ret;
}

fskvdb_stat fskvdb_kv_check(fskvdb_ctx* ctx , void* Key,fskvdb_search_ctx* kvsearch,U8* tempbuffer){
  U32 tempsize = 0;
  fskvdb_stat ret = fskvdb_DBGetValue(ctx,Key,tempbuffer,&tempsize);
  return ret;
}

/**/
fskvdb_stat fskvdb_DBGetValue(fskvdb_ctx* ctx , void* Key, void* value, U32* value_size){
  /*clear destination value for small data*/

  fskvdb_stat ret;
  fskvdb_search_ctx kvsearch;
  ret= fskvdb_DBSearchByKey(ctx,Key,&kvsearch);
  CHECH_RET_VAL_S_CTX();
  
  *value_size = 0;
  
  if (kvsearch.tablesearch.ret == fskvdb_kvdeleted)
       EXIT_ERROR_CODE(fskvdb_noexists);

if (ctx->BucketType == fskvdb_BucketType_blob){	   
  ret = fskvdb_DbRead(ctx,value,kvsearch.kvctx.ValueSize,kvsearch.kvctx.ValuePos);
  CHECH_RET_VAL_S_CTX();
  
  if (kvsearch.kvctx.crc_data != fskvdb_crc16(value,kvsearch.kvctx.ValueSize)){
	  EXIT_ERROR_CODE(fskvdb_datacrc_error);
  }
  
  if (value_size != 0)
  *value_size = kvsearch.kvctx.ValueSize;
  
}else if (ctx->BucketType == fskvdb_BucketType_queue){
	fskvdb_queue_ctx queuectx;
	ret = fskvdb_queue_read_ctx(&kvsearch,&queuectx);
	CHECH_RET_VAL_S_CTX();
	ret = fskvdb_queue_get(&kvsearch,value,value_size);
        
        if (ret != 0) *value_size = 0;
}

  //fskvdb_InOutEndOp(&ctx);
  EXIT_ERROR_CODE(ret);
}

fskvdb_stat fskvdb_DBGetValue_pos(fskvdb_ctx* ctx , U32 position,void* Key, void* value, U32* value_size){
  /*clear destination value for small data*/

  fskvdb_stat ret;
  fskvdb_search_ctx kvsearch;
  ret= fskvdb_DBSearchByKey(ctx,Key,&kvsearch);
  CHECH_RET_VAL_S_CTX();

  *value_size = 0;
  
  if (kvsearch.tablesearch.ret == fskvdb_kvdeleted)
       EXIT_ERROR_CODE(fskvdb_noexists);

if (ctx->BucketType == fskvdb_BucketType_blob){	   
	EXIT_ERROR_CODE(fskvdb_err_param);
  
}else if (ctx->BucketType == fskvdb_BucketType_queue){
	fskvdb_queue_ctx queuectx;
	ret = fskvdb_queue_read_ctx(&kvsearch,&queuectx);
	CHECH_RET_VAL_S_CTX();
	ret = fskvdb_queue_get_position(&kvsearch,position,value,value_size);
        if (ret != 0) *value_size = 0;
}

  //fskvdb_InOutEndOp(&ctx);
  EXIT_ERROR_CODE(ret);
}


fskvdb_stat fskvdb_DBGetQueueInfo(fskvdb_ctx* ctx, void* Key,fskvdb_queue_info* info){
  fskvdb_stat ret;
  fskvdb_search_ctx kvsearch;
  
  memset(info,0,sizeof(fskvdb_queue_info));
  
  ret= fskvdb_DBSearchByKey(ctx,Key,&kvsearch);
  CHECH_RET_VAL_S_CTX();

  if (kvsearch.tablesearch.ret == fskvdb_kvdeleted)
       EXIT_ERROR_CODE(fskvdb_noexists);

if (ctx->BucketType == fskvdb_BucketType_blob){	   
	EXIT_ERROR_CODE(fskvdb_notsupported);
}else if (ctx->BucketType == fskvdb_BucketType_queue){
	fskvdb_queue_ctx queuectx;
	ret = fskvdb_queue_read_ctx(&kvsearch,&queuectx);
	CHECH_RET_VAL_S_CTX();
	ret = fskvdb_queue_getInfo(&kvsearch,info);
}

  //fskvdb_InOutEndOp(&ctx);
  EXIT_ERROR_CODE(ret);
}

fskvdb_stat fskvdb_DBGetAttribute(fskvdb_search_ctx* kvsearchctx){
  return 0;
}

fskvdb_stat fskvdb_PrepareCtx(fskvdb_ctx* ctx){
if (CTX_NEED_ALIGN()){
      fskvdb_malloc(ctx->DeviceID,&ctx->ManagerPtrIn.memory,ctx->Align );
      ctx->ManagerPtrIn.memorysize = ctx->Align;
      ctx->ManagerPtrIn.position = FSKVDB_INVALID_POS;

    fskvdb_malloc(ctx->DeviceID,&ctx->ManagerPtrOut.memory,ctx->Align );
      ctx->ManagerPtrOut.memorysize = ctx->Align;
      ctx->ManagerPtrOut.position = FSKVDB_INVALID_POS;
  }

  return fskvdb_ok;
}

fskvdb_stat fskvdb_GetCtx(DBID DeviceID,fskvdb_ctx* ctx){
  memset(ctx,0,sizeof(fskvdb_ctx));
  ctx->DeviceID = DeviceID;
  
  if ( (ctx->ManagerPtrIn.memory == 0 || ctx->ManagerPtrOut.memory == 0) && ctx->Align != 0){
  ctx->Align = 512;
  ctx->DeviceID = DeviceID;
  ctx->ManagerPtrIn.memorysize = 512;
  ctx->ManagerPtrIn.position = FSKVDB_INVALID_POS;
  fskvdb_PrepareCtx(ctx);
  }

  fskvdb_stat ret = fskvdb_DbRead(ctx,ctx,sizeof(fskvdb_ctx) - sizeof(fskvdb_ManagerMemory)*2,0);
  
  CHECH_RET_VAL_S_CTX();
  CHECH_CTX_CRC();
  
  /*check version*/
  if (ctx->Version != FSKVDB_VERSION){
	  return fskvdb_wrongversion; /**/
  }

  return ret;
}

fskvdb_stat fskvdb_UpdateCtx(fskvdb_ctx* ctx){
  SET_CTX_CRC();

  if (ctx->ManagerPtrIn.memory == 0 || ctx->ManagerPtrOut.memory == 0)
      fskvdb_PrepareCtx(ctx);

  //memcpy(ctx->ManagerPtrIn.memory,ctx,sizeof(fskvdb_ctx) - sizeof(fskvdb_ManagerMemory)*2);

  //fskvdb_stat ret = fskvdb_DbWrite(ctx,ctx->ManagerPtrIn.memory,ctx->ManagerPtrIn.memorysize,0,KVDB_UPDATE);
  fskvdb_stat ret = fskvdb_DbWrite(ctx,ctx,sizeof(fskvdb_ctx),0,KVDB_UPDATE);

  return ret;
}

/*internal?*/
fskvdb_stat fskvdb_DBSearchByKey(fskvdb_ctx* ctx, void* Key, fskvdb_search_ctx* kvsearchctx){

fskvdb_stat ret;
//fskvdb_kv kvctx;

kvsearchctx->ctx = ctx;
kvsearchctx->Key = Key;
//kvsearchctx->

  if (ctx->SearchMode == fskvdb_SearchIndexMode_linear){

  kvsearchctx->position = 0;

  if (ctx->KeyNumber == 0) return fskvdb_noexists;



  /*for now we use a slow linear search*/
  ret = fskvdb_ok;
  U32 size = 0;
  U32 readptr = sizeof(fskvdb_ctx);
  while(ret == fskvdb_ok && size<ctx->KeyNumber){
      ret = fskvdb_DbRead(ctx,&kvsearchctx->kvctx,sizeof(fskvdb_kv),readptr);
      CHECH_RET_VAL_S_CTX();
      CHECH_KV_CRC_arg(kvsearchctx->kvctx);
      if (/* *Key == kvsearchctx->kvctx.Key*/ memcmp(Key,kvsearchctx->kvctx.Key,ctx->KeySize) != 0) break;
      size++;
      readptr+=kvsearchctx->kvctx.SizeBlock+sizeof(fskvdb_kv);
  }

  if (size >= ctx->KeyNumber)
      return fskvdb_noexists;

  //kvsearchctx->kvctx = kvctx;
  kvsearchctx->position = readptr;


  }else
  if (ctx->SearchMode == fskvdb_SearchIndexMode_hash){
    /*load the hash ctx and search for the key*/
     fskvdb_table_ctx hashctx;
     ret = fskvdb_readHashCtx(ctx,&hashctx,sizeof(fskvdb_ctx));
     CHECH_RET_VAL_S_CTX();
     //fskvdb_table_ctx_search searchctx;
     ret = fskvdb_hashGetKeyPosition(ctx,&hashctx,Key,&kvsearchctx->tablesearch);
     CHECH_RET_VAL_S_CTX();
     ret = fskvdb_DbRead(ctx,&kvsearchctx->kvctx,sizeof(fskvdb_kv),kvsearchctx->tablesearch.KvPosition);
     CHECH_RET_VAL_S_CTX();
	 CHECH_KV_CRC_arg(kvsearchctx->kvctx); //was missed! what a shame..
     if (/**Key != kvsearchctx->kvctx.Key*/ memcmp(Key,kvsearchctx->kvctx.Key,ctx->KeySize) != 0)
        return fskvdb_interr; /*keys not match*/
	 
     //kvsearchctx->kvctx = kvctx;
     /*TODO check*/
     kvsearchctx->position = kvsearchctx->tablesearch.KvPosition;
  }

  return fskvdb_ok;
}

/*fskvdb_stat fskvdb_ReadStruct(DBID DeviceID, void* value, U32 value_size, U32 position ){
    fskvdb_stat ret = fskvdb_read(DeviceID,value,value_size,position);
    CHECH_RET_VAL();
    DB_CHECK_CRC(hashctx);
}*/

fskvdb_stat fskvdb_DbWrite(fskvdb_ctx* ctx,void* value, U32 value_size,U32 position, U8 option){
    if (CTX_NEED_ALIGN()){
        fskvdb_stat ret;
        if ( fskvdb_Align(ctx,position,0) == ctx->ManagerPtrOut.position ) //check if we have the block on memory
        {
            if ((ctx->ManagerPtrOut.position != fskvdb_Align(ctx,position + value_size,0)) && (position !=0 && value_size!=512) )
                return fskvdb_interr; /*alignment error*/
            if ((position- ctx->ManagerPtrOut.position) + value_size > CTX_ALIGN())
               return fskvdb_interr; /*write error*/

            memcpy( ((U8*)ctx->ManagerPtrOut.memory) + (position- ctx->ManagerPtrOut.position) ,value,value_size );
            return fskvdb_ok;
        }
        /*we need to write the old value, if any, and load the new sector*/
        /*write*/
        if (ctx->ManagerPtrOut.position != FSKVDB_INVALID_POS){
        ret = fskvdb_write(ctx->DeviceID,ctx->ManagerPtrOut.memory,ctx->ManagerPtrOut.memorysize,ctx->ManagerPtrOut.position);
        CHECH_RET_VAL_S_CTX();
        }
        /*load*/
        ret = fskvdb_read(ctx->DeviceID,ctx->ManagerPtrOut.memory,ctx->ManagerPtrOut.memorysize,fskvdb_Align(ctx,position,0));
        ctx->ManagerPtrOut.position = fskvdb_Align(ctx,position,0);
        CHECH_RET_VAL_S_CTX();
        return fskvdb_DbWrite(ctx,value,value_size,position,option);
    }

    return fskvdb_write(ctx->DeviceID,value,value_size,position);
}
fskvdb_stat fskvdb_DbRead(fskvdb_ctx* ctx,void* value, U32 value_size,U32 position){
    if (CTX_NEED_ALIGN()){
        if ( fskvdb_Align(ctx,position,0) == ctx->ManagerPtrIn.position ) //check if we have the block on memory
        {
            if ((position- ctx->ManagerPtrIn.position) + value_size > CTX_ALIGN())
               return fskvdb_interr; /*write error*/

            memcpy(value , ((U8*)ctx->ManagerPtrIn.memory) + (position- ctx->ManagerPtrIn.position) ,value_size );
            return fskvdb_ok;
        }
        fskvdb_stat ret;
        /*we need to write the old value, if any, and load the new sector*/
        ret = fskvdb_read(ctx->DeviceID,ctx->ManagerPtrIn.memory,ctx->ManagerPtrIn.memorysize,fskvdb_Align(ctx,position,0));
        ctx->ManagerPtrIn.position = fskvdb_Align(ctx,position,0);
        CHECH_RET_VAL_S_CTX();
        return fskvdb_DbRead(ctx,value,value_size,position);
    }

    return fskvdb_read(ctx->DeviceID,value,value_size,position);
}

fskvdb_stat fskvdb_InOutEndOp(fskvdb_ctx* ctx){
      if (CTX_NEED_ALIGN()){

        /*write*/
        if (ctx->ManagerPtrOut.position != FSKVDB_INVALID_POS){
        fskvdb_stat ret = fskvdb_write(ctx->DeviceID,ctx->ManagerPtrOut.memory,ctx->ManagerPtrOut.memorysize,ctx->ManagerPtrOut.position);
        CHECH_RET_VAL_S_CTX();
        }

         //fskvdb_free(ctx->DeviceID,&ctx->ManagerPtrIn.memory);
         //fskvdb_free(ctx->DeviceID,&ctx->ManagerPtrOut.memory);
      }

    return fskvdb_endop(ctx->DeviceID);
}

DB_PTR fskvdb_Align(fskvdb_ctx* ctx, DB_PTR value,U32 newblock){
    if (CTX_NEED_ALIGN())
         return ( (value/CTX_ALIGN()) + newblock)*CTX_ALIGN();
         else
         return value;
}

DB_PTR fskvdb_AlignAdd(fskvdb_ctx* ctx, DB_PTR value, U32 pos, U32 newblock){
    if (CTX_NEED_ALIGN()){

         if (fskvdb_Align(ctx,value,0) != fskvdb_Align(ctx,value+pos,0) ){
             value = fskvdb_Align(ctx,value,1);
             return value;
         }

         return fskvdb_Align(ctx,value,newblock) + pos + value % CTX_ALIGN();
         }
         else
         return value+pos;
}

U32 FSKVDBStrToKey(unsigned char* str){
	return fskvdb_hash32(str,strlen(str));
}

U32 FSKVDBDataToKey(void* data,U32 size){
	return fskvdb_hash32(data,size);
}

void fskvdb_ResetOptions(fskvdb_options_ctx* options){
	options->FixedLen = 1024;
	options->AttributeLenght = 0;
	options->AutoKey = 0;
	options->DataAlign = 0;
	options->DataMaxNum = 1024;
	options->UseMallocForMemory = 0;
	options->SearchIndexMode = fskvdb_SearchIndexMode_hash;
	options->DeviceID = 0;
	options->DataCrc = 1;
	options->KeyIsString = 0;
	options->KeySize = sizeof(U32);
	
	options->BucketType = fskvdb_BucketType_blob;
	
	/*for the queue*/
	options->ElemNumQueue = 1024;
	options->ElemSizeQueue = 1024;
}
