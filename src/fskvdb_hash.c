#include "fskvdb_hash.h"


fskvdb_stat fskvdb_createHashTable(fskvdb_ctx* ctx,U32 TableSize, U32 ListSize, U16 ByteSearchValue, U32 write_position, fskvdb_table_ctx* hashctx){
  fskvdb_table_ctx hash_ctx;
  hash_ctx.TableNumber = TableSize;
  hash_ctx.LenghtData = ByteSearchValue;
  hash_ctx.TableEntry = ListSize;

  /*check alignment*/
  if (CTX_NEED_ALIGN()){
  if (ByteSearchValue+sizeof(fskvdb_table_list_footer) > ctx->Align)
      return fskvdb_alignerr;
  }

  DB_PTR InitPos = write_position;
  fskvdb_stat ret;

  hash_ctx.position = write_position;

  U32 Entry,Tab;
  //U32 EntryNum = TableSize * ListSize;

  Entry = fskvdb_Align(ctx,write_position,1); /*next block*/
  /*hack for alignment*/
  if (write_position == Entry )
			write_position+=sizeof(fskvdb_table_ctx);
       else
			write_position = Entry;

  hash_ctx.tableposition = write_position;
  hash_ctx.TableByteSize = write_position;

  fskvdb_table_list_footer footer;
  memset(&footer,0,sizeof(fskvdb_table_list_footer));
  //U32 BytesOnBlock = 0;
  Tab = 0;
  
  footer.status = fskvdb_kvempty;
  DB_SET_CRC(footer);
  /*{
  uint8_t tempbuffer[FSKVDB_MAX_KEY_SIZE];
  memset(tempbuffer,0,sizeof(tempbuffer));
  footer.crcdata = fskvdb_crc16(tempbuffer,ByteSearchValue);
  }*/
  
  /*this code is ok only for alignment*/
  if (CTX_NEED_ALIGN()){
  while(Tab <= TableSize){
  Entry = 0;
  while(Entry < ListSize){
        ret = fskvdb_DbWrite(ctx,&footer,sizeof(fskvdb_table_list_footer),write_position + ByteSearchValue ,KVDB_UPDATE);
        CHECH_RET_VAL();
        write_position = fskvdb_AlignAdd(ctx,write_position,ByteSearchValue + sizeof(fskvdb_table_list_footer),0);
        Entry++;
  }
  write_position = fskvdb_Align(ctx,write_position,1);
  Tab++;
  }
  
  }else{
	  uint8_t tempbuffer[FSKVDB_MAX_KEY_SIZE + sizeof(fskvdb_table_list_footer)];
	  memset(tempbuffer,0,sizeof(tempbuffer));
	  memcpy(tempbuffer,&footer,sizeof(fskvdb_table_list_footer));
	  
	  ret = fskvdb_writefill(ctx->DeviceID,tempbuffer,sizeof(fskvdb_table_list_footer) + ByteSearchValue,(TableSize*ListSize)+1,write_position + ByteSearchValue);
	  CHECH_RET_VAL();
	  write_position += ( ((TableSize*ListSize)+1)*(sizeof(fskvdb_table_list_footer) + ByteSearchValue))+ByteSearchValue+1;
  }
  
  /*calculate space usage*/
  write_position = fskvdb_Align(ctx,write_position,1); /*next block*/
  hash_ctx.TableByteSize = write_position - hash_ctx.TableByteSize;

   DB_SET_CRC(hash_ctx);
  *hashctx = hash_ctx;

   ret = fskvdb_DbWrite(ctx,hashctx,sizeof(fskvdb_table_ctx),InitPos,KVDB_UPDATE);

  return ret;
}

fskvdb_stat fskvdb_readHashCtx(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx, U32 position){
    fskvdb_stat ret = fskvdb_DbRead(ctx,hashctx,sizeof(fskvdb_table_ctx),position);
    CHECH_RET_VAL();
    DB_CHECK_CRC(*hashctx);
    return ret;
}

U32 fskvdb_hash_TableSize(fskvdb_table_ctx* hashctx){
    return /*(hashctx->TableNumber * sizeof(fskvdb_table_ctx_list)) +*/ (hashctx->TableNumber * hashctx->TableEntry * hashctx->LenghtData * sizeof(fskvdb_table_list_footer)) + sizeof(fskvdb_table_ctx);
}

/*TODO direct access, check*/
fskvdb_stat fskvdb_hashGetTablePos(fskvdb_table_ctx* hashctx ,void* key , U32* position){
	U32 modthis = (hashctx->TableEntry * hashctx->TableNumber -hashctx->TableEntry);
	if (modthis == 0) modthis = 1;
    U32 Block = (fskvdb_hash32(key,hashctx->LenghtData) % modthis) / hashctx->TableEntry;
    *position = Block;
    return fskvdb_ok;
}

U32 fskvdb_hash_listpos(fskvdb_table_ctx* hashctx,U32 hash){
    return hash * sizeof(fskvdb_table_ctx_list) * hashctx->TableEntry * hashctx->LenghtData * sizeof(fskvdb_table_list_footer);
}

static fskvdb_stat fskvdb_hashSearchPos(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx,void* key,fskvdb_table_list_footer_search* footer , fskvdb_stat value, fskvdb_stat value2){
    /*get the position of the list*/
    U32 hash_position;
    fskvdb_hashGetTablePos(hashctx, key , &hash_position);

    if (CTX_NEED_ALIGN()){
        //U32 shift_byte = (hash_position-1) * (CTX_ALIGN()%hashctx->TableEntry);
        hash_position -= (hash_position*(CTX_ALIGN()%hashctx->TableEntry)) / CTX_ALIGN();
        hash_position *= CTX_ALIGN();
        hash_position += hashctx->tableposition;
        //hash_position -= shift_byte;
    }else{
        hash_position *= hashctx->TableEntry * (sizeof(fskvdb_table_list_footer)+hashctx->LenghtData);
        hash_position += hashctx->tableposition;
    }

    //searchctx->KvPosition = 0;
    U32 Num = 0;
    U8 KeyRead[FSKVDB_MAX_KEY_SIZE];
    fskvdb_stat ret;
while( hash_position < (hashctx->tableposition +hashctx->TableByteSize) ){
    ret = fskvdb_DbRead(ctx,&footer->footer,sizeof(fskvdb_table_list_footer),hash_position + hashctx->LenghtData);
    Num++;
    CHECH_RET_VAL();
    DB_CHECK_CRC(footer->footer);
    footer->position = hash_position /*+ hashctx->LenghtData*/;

    if (footer->footer.status == fskvdb_kvempty){

        return fskvdb_ok;
    }

    ret = fskvdb_DbRead(ctx,KeyRead,hashctx->LenghtData,hash_position);
    CHECH_RET_VAL();

    if (memcmp(key,KeyRead,hashctx->LenghtData) == 0 )
    if (footer->footer.status == value || footer->footer.status == value2) {
    if (footer->footer.status == fskvdb_kvdeleted || footer->footer.status == fskvdb_kvok){

        return fskvdb_ok;
    }else
        {
            /*key is corrupted*/
            return fskvdb_tabinv;
        }
    }

    hash_position = fskvdb_AlignAdd(ctx,hash_position,hashctx->LenghtData + sizeof(fskvdb_table_list_footer),0);
}
    return fskvdb_interr;
}

fskvdb_stat fskvdb_hashGetKeyPosition(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx, void* key, fskvdb_table_ctx_search* searchctx){
    fskvdb_table_list_footer_search footer;
    fskvdb_stat ret = fskvdb_hashSearchPos(ctx,hashctx,key,&footer,fskvdb_kvdeleted,fskvdb_kvok);
    CHECH_RET_VAL();

    searchctx->exKey = key;
    searchctx->LenghtData = hashctx->LenghtData;

    if (footer.footer.status == fskvdb_kvempty){
        searchctx->KvPosition = 0;
        searchctx->EntryPosition = footer.position/* + hashctx->LenghtData*/;
        searchctx->ret = fskvdb_noexists;
        return fskvdb_noexists;
    }

    if (footer.footer.status == fskvdb_kvdeleted || footer.footer.status == fskvdb_kvok){
        searchctx->KvPosition = footer.footer.pointer;
        searchctx->EntryPosition = footer.position/* + hashctx->LenghtData*/;
        searchctx->ret = footer.footer.status;
        return fskvdb_ok;
    }

    return fskvdb_interr;
}

fskvdb_stat fskvdb_hashDeleteKey(fskvdb_search_ctx* searchctx){
    fskvdb_stat ret;
    if (searchctx->tablesearch.ret == fskvdb_kvempty || searchctx->tablesearch.ret == fskvdb_kvdeleted){
        return fskvdb_noexists;
    }
    if (searchctx->tablesearch.ret == fskvdb_kvok){
        fskvdb_table_list_footer footer;
		 memset(&footer,0,sizeof(fskvdb_table_list_footer));
        footer.status = fskvdb_kvdeleted;
        footer.pointer = searchctx->tablesearch.KvPosition;
        DB_SET_CRC(footer);
        ret = fskvdb_DbWrite(searchctx->ctx,&footer,sizeof(fskvdb_table_list_footer),searchctx->tablesearch.EntryPosition + searchctx->tablesearch.LenghtData,KVDB_UPDATE);
        CHECH_RET_VAL();
        return fskvdb_ok;
    }
    return fskvdb_interr;
}

fskvdb_stat fskvdb_hashAddKey(fskvdb_search_ctx* searchctx){
    fskvdb_stat ret;
    if (searchctx->tablesearch.ret == fskvdb_kvempty || searchctx->tablesearch.ret == fskvdb_kvdeleted || searchctx->tablesearch.ret == fskvdb_noexists){
        fskvdb_table_list_footer footer;
		 memset(&footer,0,sizeof(fskvdb_table_list_footer));
        footer.status = fskvdb_kvok;
        footer.pointer = searchctx->position;
        DB_SET_CRC(footer);

        ret = fskvdb_DbWrite(searchctx->ctx,searchctx->tablesearch.exKey,searchctx->tablesearch.LenghtData,searchctx->tablesearch.EntryPosition,KVDB_UPDATE);
        CHECH_RET_VAL();

        ret = fskvdb_DbWrite(searchctx->ctx,&footer,sizeof(fskvdb_table_list_footer),searchctx->tablesearch.EntryPosition+ searchctx->tablesearch.LenghtData,KVDB_UPDATE);
        CHECH_RET_VAL();
        return fskvdb_ok;
    }
    if (searchctx->tablesearch.ret == fskvdb_kvok){
        /*the key is already added*/
        return fskvdb_ok;
    }
    return fskvdb_interr;
}

static fskvdb_stat fskvdb_Crcfskvdb_table_ctx(fskvdb_table_ctx* hashctx){
	DB_CHECK_CRC(*hashctx);
	return 0;
}

#define HASH_TAB_ERROR(x,y,args...){\
	if (y != 0){\
	FSKVDB_PRINT_ERROR(x,args);\
	hash_scan_errors++;\
	}\
}

#define HASH_TAB_PERM_ERROR(y,x...){\
	HASH_TAB_ERROR("unrecoverable error %s",1,x);\
	FSKVDB_PRINT_INFO("errors: %i\n errors to recover: %i",hash_scan_errors,hash_error_to_recover);\
	return y;\
}

#define HASH_SET_ERROR_TO_RECOVER() hash_error_to_recover++

#define HASH_TO_READ() (sizeof(fskvdb_table_list_footer)+hash_key_size)

#define HASH_CONSECUTIVE_ERROR_MAX (maxconsecutiverr)

static fskvdb_stat fskvdb_HashCheckCrcFooter(fskvdb_table_list_footer* footer){
	DB_CHECK_CRC(*footer);
	return 0;
}

#define HASH_KEY() (tempbuffer)

#define HASH_PRINT_KEY(){\
		FSKVDB_PRINT_INFO("key found pos: %i",foot->pointer);\
		FSKVDB_PRINT_BUFFER("key",HASH_KEY(),-hash_key_size);\
		FSKVDB_PRINT_INFO("key is deleted: %i",foot->status == fskvdb_kvdeleted);\
}

#define HASH_KEY_SLOT_SUM() (hash_key_empty_found + hash_key_delete_found + hash_key_found)



fskvdb_stat fskvdb_hashTableCheck(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx,uint32_t maxconsecutiverr,uint8_t printcrcerror,U8* kvreadbuffer,uint8_t verbose){
	FSKVDB_PRINT_FUNC();
	/**/
	uint8_t repaire_hash_ctx = 0;
	int32_t hash_table_keys = -1;
	int32_t hash_table_list_size = -1;
	
	uint32_t hash_scan_errors = 0;
	uint32_t hash_error_to_recover = 0;
	
	uint32_t hash_key_size = 0;
	
	uint32_t hash_key_delete_found = 0;
	uint32_t hash_key_found = 0;
	uint32_t hash_key_empty_found = 0;
        
        int64_t hash_scan_limit = 1024;
	
	uint32_t hash_consecutive_error = 0;
	DB_PTR hash_current_readpos = sizeof(fskvdb_ctx);
	DB_PTR hash_last_know_good_position = 0;
	DB_PTR hash_first_know_good_position = 0;
	
	int64_t hash_key_limit = -1;
	int64_t hash_exprected_content_num = -1;
	DB_PTR hash_table_position = 0;
	//int64_t hash_key_limit_backup = -1;
	
	fskvdb_search_ctx searchctx;
	
	fskvdb_stat ret = fskvdb_readHashCtx(ctx,hashctx,hash_current_readpos);
	if (ret != 0 && ret != fskvdb_crc_error){
		HASH_TAB_PERM_ERROR(ret,"reading hash context","");
		return ret;
	}
	if (ret == fskvdb_crc_error){
		HASH_TAB_ERROR("hash context is corrupted",ret,"");
		repaire_hash_ctx = 1;
		
		hash_key_size = ctx->KeySize;
		if (verbose)
		FSKVDB_PRINT_INFO("key size %i from main db context",hash_key_size);
	}else{
		hash_key_size = hashctx->LenghtData;
		hash_table_keys = hashctx->TableNumber;
		hash_table_list_size = hashctx->TableEntry;
		
		hash_key_limit = hash_table_keys * hash_table_list_size; //hashctx->TableNumber * hashctx->TableEntry
		hash_exprected_content_num = hash_key_limit;
                
                hash_scan_limit = hash_exprected_content_num * (sizeof(fskvdb_table_list_footer)+hash_key_size);
		
		FSKVDB_PRINT_INFO("key size %i from hash context",hash_key_size);
		FSKVDB_PRINT_INFO("keys on the hash table %i, chunk list of %i, total: %u",hash_table_keys,hash_table_list_size,hash_key_limit);
		
		hash_table_position = hashctx->tableposition;
		
		if (hashctx->LenghtData != ctx->KeySize){
			HASH_TAB_ERROR("key size from db ctx is different from hash ctx",1,"");
			/*TODO heal main ctx?*/
			HASH_SET_ERROR_TO_RECOVER();
		}
		
		if (hashctx->position != hash_current_readpos){
			HASH_TAB_ERROR("hash ctx position set is wrong %i instead of %i",1,hashctx->position,hash_current_readpos);
			/*TODO heal main ctx?*/
			HASH_SET_ERROR_TO_RECOVER();
		}
	}
	
	hash_current_readpos += sizeof(fskvdb_table_ctx);
	
	FSKVDB_PRINT_INFO("scanning hash table from %i",hash_current_readpos);
	uint8_t tempbuffer[FSKVDB_MAX_KEY_SIZE + sizeof(fskvdb_table_list_footer)];
	
	while(hash_consecutive_error < HASH_CONSECUTIVE_ERROR_MAX ){
		ret = fskvdb_DbRead(ctx,tempbuffer,HASH_TO_READ(),hash_current_readpos);
		if (verbose)
		HASH_TAB_ERROR("Reading hash key pos %i",ret,hash_current_readpos);
		
		/*getting the key on the db*/
		if (ret == 0){
			
			fskvdb_table_list_footer* foot = (fskvdb_table_list_footer*)(tempbuffer+hash_key_size);
			
			/*check crc*/
			if (fskvdb_HashCheckCrcFooter(foot) != 0){
				//HASH_TAB_ERROR("Reading hash key pos %i crc error",1,hash_current_readpos);
				//HASH_SET_ERROR_TO_RECOVER();
				if (hash_first_know_good_position != 0){
					if (printcrcerror){
						if (verbose)
						HASH_TAB_ERROR("Reading hash key pos %i crc error",1,hash_current_readpos);
					}else
						hash_scan_errors++;
					HASH_SET_ERROR_TO_RECOVER();
					hash_consecutive_error++;
					goto cont;
				}
				goto searchbyte;
			}
                        hash_scan_limit -= sizeof(fskvdb_table_list_footer)+hash_key_size;
			
			if (hash_first_know_good_position == 0){
				hash_first_know_good_position = hash_current_readpos;
				if (hash_table_position != -1 && hash_first_know_good_position != hash_table_position){
					HASH_TAB_ERROR("hash table data position is wrong %i instead of %i",1,hash_first_know_good_position,hash_table_position);
					hash_table_position = hash_first_know_good_position;
				}
			}
			
			/*last good know position*/
			hash_last_know_good_position = hash_current_readpos;
			hash_consecutive_error = 0;
			
			if (foot->status == fskvdb_kvempty) {
				hash_key_empty_found++;
				goto cont;
			}else
			if (foot->status == fskvdb_kvdeleted){
				/*TODO check for the key deleted*/
				hash_key_delete_found++;
				if (verbose)
				HASH_PRINT_KEY();
				
				ret = fskvdb_kv_check(ctx,HASH_KEY(),&searchctx,kvreadbuffer);
				if (ret == 0){
					HASH_TAB_ERROR("the key is deleted but can be retrive, error %i",1,0);
					HASH_PRINT_KEY();
				}
				if (ret == 0) HASH_SET_ERROR_TO_RECOVER();
				goto cont;
			}else
			if (foot->status != fskvdb_kvok){
				HASH_TAB_ERROR("hash key found with unknow status %i",1,foot->status);
				HASH_SET_ERROR_TO_RECOVER();
				goto cont;
			}
			hash_key_found++;
			if (verbose)
			HASH_PRINT_KEY();

			ret = fskvdb_kv_check(ctx,HASH_KEY(),&searchctx,kvreadbuffer);
			if (ret != fskvdb_kvempty)
				HASH_TAB_ERROR("getting the key, error %i",ret,ret);
			if (ret != 0 && ret != fskvdb_kvempty){
				FSKVDB_PRINT_BUFFER("key",HASH_KEY(),-hash_key_size);\
			}
			if (ret != 0 && ret != fskvdb_kvempty) HASH_SET_ERROR_TO_RECOVER();
			/*TODO check for the key to be correctly stored*/
		}else{
			//hash_consecutive_error++;
			break; //read error
		}
		
		cont:
		
		if (hash_key_limit != -1 && hash_key_limit >= 1){
			hash_key_limit--;
		}
                if (hash_key_limit == 0)
                    break; /*scan completed*/
		
		hash_current_readpos+=HASH_TO_READ();
		continue;
		
		searchbyte:
		hash_current_readpos+=1;
                hash_scan_limit--;
                if (hash_scan_limit <= 0) break;
		continue;
		
	}
	
	if (hash_exprected_content_num != -1 && hash_exprected_content_num != HASH_KEY_SLOT_SUM()){
		HASH_TAB_ERROR("hash keys slot found is not expected %d instead of %d",1,HASH_KEY_SLOT_SUM(),(uint32_t)hash_exprected_content_num);
	}
	
	/*print stat*/
	FSKVDB_PRINT_INFO("HashCtx to repair: %i",repaire_hash_ctx);
	FSKVDB_PRINT_INFO("Keys deleted: %i",hash_key_delete_found);
	FSKVDB_PRINT_INFO("Keys empty: %i",hash_key_empty_found);
	FSKVDB_PRINT_INFO("Keys: %i",hash_key_found);
	FSKVDB_PRINT_INFO("Keys sum: %i",HASH_KEY_SLOT_SUM());
	FSKVDB_PRINT_INFO("Errors: %i",hash_scan_errors);
	FSKVDB_PRINT_INFO("Errors to recover: %i",hash_error_to_recover);
	FSKVDB_PRINT_INFO("Hash start %i",hash_first_know_good_position);
	FSKVDB_PRINT_INFO("Hash stop %i",hash_last_know_good_position);
	FSKVDB_PRINT_INFO("Hash consecutive error %i",hash_consecutive_error);
	
	/*if errors to recover return error*/
	if (hash_error_to_recover)
		return fskvdb_crc_error;
	
	return 0;
}

fskvdb_stat fskvdb_hashMapReduce_init(fskvdb_ctx* ctx,fskvdb_mapreduce_ctx* mrctx){
	mrctx->position = sizeof(fskvdb_ctx) + sizeof(fskvdb_table_ctx);
	mrctx->size = 0;
	return 0;
}

/*fskvdb_stat fskvdb_hashMapReduce_nextkey(fskvdb_ctx* ctx,fskvdb_mapreduce_ctx* mrctx,void* key){
		fskvdb_stat ret = fskvdb_DbRead(ctx,mrctx->readbuffer,sizeof(fskvdb_table_list_footer)+ctx->KeySize,hash_current_readpos);
		//HASH_TAB_ERROR("Reading hash key pos %i",ret,hash_current_readpos);
		
		//getting the key on the db
		if (ret == 0){
			
			fskvdb_table_list_footer* foot = (fskvdb_table_list_footer*)(mrctx->readbuffer+ctx->KeySize);
			
			//check crc
			if (fskvdb_HashCheckCrcFooter(foot) != 0){
				return fskvdb_crc_error;
			}
			
			hash_scan_limit -= sizeof(fskvdb_table_list_footer)+hash_key_size;
			
			if (hash_first_know_good_position == 0){
				hash_first_know_good_position = hash_current_readpos;
				if (hash_table_position != -1 && hash_first_know_good_position != hash_table_position){
					HASH_TAB_ERROR("hash table data position is wrong %i instead of %i",1,hash_first_know_good_position,hash_table_position);
					hash_table_position = hash_first_know_good_position;
				}
			}
			
			//last good know position
			hash_last_know_good_position = hash_current_readpos;
			hash_consecutive_error = 0;
			
			if (foot->status == fskvdb_kvempty) {
				hash_key_empty_found++;
				goto cont;
			}else
			if (foot->status == fskvdb_kvdeleted){
				//TODO check for the key deleted
				hash_key_delete_found++;
				HASH_PRINT_KEY();
				
				ret = fskvdb_kv_check(ctx,HASH_KEY(),&searchctx,kvreadbuffer);
				HASH_TAB_ERROR("getting the key, error %i",ret,ret);
				if (ret != 0) HASH_SET_ERROR_TO_RECOVER();
			}else
			if (foot->status != fskvdb_kvok){
				HASH_TAB_ERROR("hash key found with unknow status %i",1,foot->status);
				HASH_SET_ERROR_TO_RECOVER();
				goto cont;
			}
			hash_key_found++;
			HASH_PRINT_KEY();
						
			ret = fskvdb_kv_check(ctx,HASH_KEY(),&searchctx,kvreadbuffer);
			HASH_TAB_ERROR("getting the key, error %i",ret,ret);
			if (ret != 0) HASH_SET_ERROR_TO_RECOVER();
			//TODO check for the key to be correctly stored
		}else{
			//hash_consecutive_error++;
			return ret; //read error
		}
		
		cont:
		
		if (hash_key_limit != -1 && hash_key_limit >= 1){
			hash_key_limit--;
		}
                if (hash_key_limit == 0)
                    break; //scan completed
		
		hash_current_readpos+=HASH_TO_READ();
		continue;
		
		searchbyte:
		hash_current_readpos+=1;
                hash_scan_limit--;
                if (hash_scan_limit <= 0) break;
		
}*/
