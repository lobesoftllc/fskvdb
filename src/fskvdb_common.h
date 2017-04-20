#ifndef COMMON_FSKVDB_COMMON_h
#define COMMON_FSKVDB_COMMON_h

/*options*/

/**/
#define CACHE_CTX

#include <stdint.h>
#include <string.h>

//#include "../logger/app_logger.h"
#include "common_logs.h"

#define FSKVDB_PRINT_FUNC APP_PRINT_FUNC
#define FSKVDB_PRINT_BUFFER APP_PRINT_BUFFER
#define FSKVDB_PRINT_INFO APP_PRINT_INFO
#define FSKVDB_PRINT_ERROR APP_PRINT_ERROR


/*defines for data types*/
#define U64 uint64_t
#define U32 uint32_t
#define U16 uint16_t
#define U8 uint8_t
#define DBID U16
#define DB_PTR U32

#define FSKVDB_HASH_MAP_SIZE_DEF 16000
#define FSKVDB_HASH_MAP_SIZE_LIST_DEF 3
#define FSKVDB_MAX_KEY_SIZE 8
#define FSKVDB_HASH_DIVISOR 10

#define FSKVDB_VERSION 121

#pragma pack(push, 1) // exact fit - no padding
typedef struct fskvdb_kv{
  /*the size off the value*/
  U32 ValueSize;
  
  /*the key id*/
  U8 Key[FSKVDB_MAX_KEY_SIZE];
  
  /*virtual pointer of the next block that containt the data*/
  U32 NextValue;

  /*value position*/
  U32 ValuePos;

  /*value block size*/
  U32 SizeBlock;

  /*Deleted modified ect ect*/
  U8 Status;

    /*crc of the data*/
  U16 crc_data;

  /*crc of kv ctx*/
  U16 crc_kv;
}fskvdb_kv;

typedef struct fskvdb_options_ctx{
    U16  AttributeLenght;
    U32  FixedLen;
    U32  FixedEntry;
    U8   AutoKey;
    U8   SearchIndexMode;
    U8   DataCrc;
	 U8 UseMallocForMemory;
    DBID DeviceID;

    U32 SearchTableSize;
    U32 SearchTableEntry;

    U32 DataAlign;
    U8  Preallocate;
    U32 DataMaxNum;
	
	U16 KeySize;
	U8 KeyIsString;
	
	U8 BucketType;
	U32 ElemNumQueue;
	U32 ElemSizeQueue;
	
}fskvdb_options_ctx;

#define FSKVDB_OPT_KVNOLIMIT 0
#define FSKVDB_OPT_KVSIZENOLIMIT 0
#define FSKVDB_OPT_KVSIZEMAXCLUSTER 0xffffffff

typedef enum fskvdb_SearchIndexMode{
  fskvdb_SearchIndexMode_linear = 0,
  fskvdb_SearchIndexMode_hash ,
}fskvdb_SearchIndexMode;

typedef enum fskvdb_BucketType{
	fskvdb_BucketType_blob = 0,
	fskvdb_BucketType_queue,
	
}fskvdb_BucketType;

typedef struct fskvdb_ManagerMemory{
    void* memory;
    U32 memorysize;
    DB_PTR position;
}fskvdb_ManagerMemory;

typedef struct fskvdb_ctx{
  U32 Version;	
  U16 KeySize;
  U8 KeyIsString;
  U32 UserVersion;	
  U32 KeyNumber; /*the number of key in the db*/
  U16 DeviceID; /*identifies the db ctx*/
  U16 AttributeLenght; /*lenght of the attribute*/
  U32 FixedSize; /*lenght of an eventually fixed size pkt*/
  U32 NextFreePos;
  U8 SearchMode;
  U8 BucketType;
  U8 Status;
  void* LastKey; /*the last inserted key*/
  U32 Align; /*the align of the data*/
  U32 QueueElemNum;
  U32 QueueElemSize;
  U8 Ready;
  
  U16 crc; /*the crc of the ctx*/
  fskvdb_ManagerMemory ManagerPtrIn;/*pointer in the memory for operations*/
  fskvdb_ManagerMemory ManagerPtrOut;/*pointer in the memory for operations*/
  U8 LastErr;
  //U8* MemoryInOut;/*pointer in the memory for read writes*/
}fskvdb_ctx;


typedef enum fskvdb_ctx_opt{
    fskvdb_opt_datacrc = 1,
    fskvdb_opt_fixed = 2,
    fskvdb_opt_autokey = 4,
    fskvdb_opt_linearsearch = 8,
    fskvdb_opt_hashsearch = 16
}fskvdb_ctx_opt;

typedef enum fskvdb_kv_status{
    fskvdb_kv_status_ok = 0,
    fskvdb_kv_status_crcerr = 1,
    fskvdb_kv_status_readonly = 2,
    fskvdb_kv_status_modified = 4,

    fskvdb_kv_status_deleted = 0xff
}fskvdb_kv_status;

typedef enum fskvdb_stat{
  fskvdb_ok = 0,
  fskvdb_exists = 1,
  fskvdb_err_param = 2,
  fskvdb_hw_fault = 3,
  fskvdb_temp_error = 4,
  fskvdb_invalid_ctx = 5,
  fskvdb_db_notready = 6,
  fskvdb_crc_error = 7,
  fskvdb_kvcrc_error =8 ,
  fskvdb_ctxcrc_error = 9,
  fskvdb_datacrc_error = 10,
  fskvdb_noexists = 11,
  fskvdb_updated = 12,
  fskvdb_2big = 13,
  fskvdb_full =14 ,
  fskvdb_kvdeleted=15,
  fskvdb_kvempty=16,
  fskvdb_kvok=17,
  fskvdb_tabinv=18,
  fskvdb_alignerr=19,
  fskvdb_interr=20,
  fskvdb_notsupported=21,
  fskvdb_wrongversion=22,
  fskvdb_index_error=23,
  fskvdb_op_end=24,
}fskvdb_stat;

typedef struct fskvdb_table_ctx{
    U32 TableNumber;
    U32 TableEntry;
    U16 LenghtData;
    U32 TableByteSize;
    U32 position;
    U32 tableposition;
    U16 crc;
}fskvdb_table_ctx;

typedef struct fskvdb_table_ctx_list{
    U32 Size;
    U32 MaxSize;
    U32 DeletedSize;
    U32 LenghtData;
    U16 crc;
}fskvdb_table_ctx_list;

typedef struct fskvdb_table_ctx_search{
    DB_PTR EntryPosition;
    U32 EntryNumber;
    DB_PTR ListPosition;
    DB_PTR KvPosition;
    fskvdb_stat ret;
    void* exKey;
    U32 LenghtData;
    //fskvdb_table_ctx_list lista;
}fskvdb_table_ctx_search;

typedef struct fskvdb_search_ctx{
  fskvdb_kv kvctx;
  fskvdb_ctx* ctx;
  fskvdb_table_ctx_search tablesearch;
  U32 position;
  void* Key;
  void* User;
}fskvdb_search_ctx;

typedef struct fskvdb_table_list_footer{
    /*data N*/
    DB_PTR pointer;
    U8 status;
    U16 crcdata;
    U16 crc;
}fskvdb_table_list_footer;

typedef struct fskvdb_table_list_footer_search{
    /*data N*/
    DB_PTR position;
    fskvdb_table_list_footer footer;

}fskvdb_table_list_footer_search;

/* typedef enum fskvdb_keystatus{
    fskvdb_key_ok = 0,
    fskvdb_key_crc,
    fskvdb_key_deleted
}fskvdb_keystatus; */

#pragma pack(pop) //back to whatever the previous packing mode was

/*this struct is used for map reduce*/
typedef struct fskvdb_mapreduce_ctx{
	DB_PTR position;
	
	void* readbuffer;
	U32 readbuffersize;
	
	U32 size;
}fskvdb_mapreduce_ctx;

/*calculate the crc of a buffer*/
extern U16 fskvdb_crc16(void* buffer, U32 size);

extern U32 fskvdb_hash32(void* buffer, U32 size);

/*read write, device is the ID of the db context, and is set in the creation phase*/
extern fskvdb_stat fskvdb_open(DBID device);
extern fskvdb_stat fskvdb_close(DBID device);
extern fskvdb_stat fskvdb_endop(DBID device);

extern fskvdb_stat fskvdb_write(DBID device,void* buffer, U32 size, U32 position);
extern fskvdb_stat fskvdb_read(DBID device,void* buffer, U32 size, U32 position);
extern fskvdb_stat fskvdb_writefill(DBID device,void* buffer,U32 buffersize , U32 size, U32 position);
extern fskvdb_stat fskvdb_malloc(DBID device,void** ptr,U32 buffersize);
extern fskvdb_stat fskvdb_free(DBID device,void** ptr);


/*callback for IN/OUT op*/
extern fskvdb_stat fskvdb_callback(DBID device, U16 req_id , fskvdb_stat req_result ,U8* buffer, U32 size);  

extern fskvdb_stat fskvdb_kv_check(fskvdb_ctx* ctx , void* Key,fskvdb_search_ctx* kvsearch,U8* tempbuffer);
  
  
/*for hash,linear,queue*/
extern fskvdb_stat fskvdb_DbWrite(fskvdb_ctx* ctx,void* value, U32 value_size,U32 position, U8 option);
extern fskvdb_stat fskvdb_DbRead(fskvdb_ctx* ctx,void* value, U32 value_size,U32 position);
extern DB_PTR fskvdb_Align(fskvdb_ctx* ctx, DB_PTR value,U32 newblock);
extern DB_PTR fskvdb_AlignAdd(fskvdb_ctx* ctx, DB_PTR value, U32 pos,U32 newblock);

  
/*this is a key value db storage for embedded system, it will run under a fs or flash eeprom ram.. (now only fs implemented)*/

#define DB_SET_KEY_TO_KVC(x,y) {memset(x,0,sizeof(x));memcpy(x,y,ctx->KeySize);}

#define DB_SET_CRC(x) {(x).crc = fskvdb_crc16(&(x),sizeof((x)) - sizeof(U16));}
#define DB_SET_CRC_type(x,y) {((y*)(x))->crc = fskvdb_crc16((x),sizeof(y) - sizeof(U16));}

#define DB_CHECK_CRC(x) {if ((x).crc != fskvdb_crc16(&(x),sizeof((x)) - sizeof(U16)))\
                            return fskvdb_crc_error;};
							
#define DB_CHECK_CRC_type(x,y) {if (((y*)(x))->crc != fskvdb_crc16((x),sizeof(y) - sizeof(U16)))\
                            return fskvdb_crc_error;};

#define CHECH_CTX_CRC()   {if (fskvdb_crc16(ctx,sizeof(fskvdb_ctx) - (sizeof(U16) + sizeof(fskvdb_ManagerMemory)*2 +sizeof(U8)) ) != ctx->crc){\
                            if (ret != fskvdb_noexists)\
                            ctx->LastErr = ret;\
                            return fskvdb_ctxcrc_error;\
                          }};

#define CHECH_CTX_CRC_noptr()   {if (fskvdb_crc16(&ctx,sizeof(fskvdb_ctx)  - (sizeof(U16) + sizeof(fskvdb_ManagerMemory)*2 +sizeof(U8)) ) != ctx.crc)\
                                    return fskvdb_ctxcrc_error;};

#define CHECH_KV_CRC_ptr()   {if (fskvdb_crc16(kvctx,sizeof(fskvdb_kv) - sizeof(U16)) != kvctx->crc_kv)\
                          return fskvdb_kvcrc_error;};

#define CHECH_KV_CRC()   {if (fskvdb_crc16(&kvctx,sizeof(fskvdb_kv) - sizeof(U16)) != kvctx.crc_kv)\
                          return fskvdb_kvcrc_error;};
#define CHECH_KV_CRC_arg(x)   {if (fskvdb_crc16(&(x),sizeof(fskvdb_kv) - sizeof(U16)) != (x).crc_kv)\
                          return fskvdb_kvcrc_error;};

#define SET_CTX_CRC() {ctx->crc = fskvdb_crc16(ctx,sizeof(fskvdb_ctx)  - (sizeof(U16) + sizeof(fskvdb_ManagerMemory)*2 +sizeof(U8)));};

#define SET_CTX_CRC_noptr() {ctx.crc = fskvdb_crc16(&ctx,sizeof(fskvdb_ctx)  - (sizeof(U16) + sizeof(fskvdb_ManagerMemory)*2 +sizeof(U8)));};

#define SET_KV_CRC_ptr() {kvctx->crc_data = fskvdb_crc16(value,value_size);\
                      kvctx->crc_kv = fskvdb_crc16(kvctx,sizeof(fskvdb_kv) - sizeof(U16));};
#define SET_KV_CRC(x) {if (ctx->Status & fskvdb_opt_datacrc)\
                      (x).crc_data = fskvdb_crc16(value,value_size);\
                      else\
                      (x).crc_data = 0;\
                      (x).crc_kv = fskvdb_crc16(&(x),sizeof(fskvdb_kv) - sizeof(U16));};

#define CHECH_RET_VAL()   {if (fskvdb_ok != ret){\
                            return ret;\
                            }\
                          }

#define CHECH_RET_VAL_S_CTX()   {if (fskvdb_ok != ret){\
                            if (ret != fskvdb_noexists)\
                            ctx->LastErr = ret;\
                            return ret;\
                            }\
                          }

#define EXIT_ERROR_CODE(x) {fskvdb_InOutEndOp(ctx);if ((x) != fskvdb_ok && (x) != fskvdb_noexists && (x) != fskvdb_exists)ctx->LastErr=(x);return (x);}
#define EXIT_ERROR_CODE_noptr(x) {fskvdb_InOutEndOp(&ctx);if ((x) != fskvdb_ok && (x) != fskvdb_noexists && (x) != fskvdb_exists)ctx.LastErr=(x);return (x);}

#define CTX_NEED_ALIGN() (ctx->Align != 0 && ctx->Align != 1)
#define CTX_ALIGN() (ctx->Align)


/*options for read/write*/
#define KVDB_UPDATE 1
#define KVDB_DISCARD 0

#define FSKVDB_INVALID_POS 0xFFFFFFFF


#endif /*COMMON_FSKVDB_COMMON_h*/
