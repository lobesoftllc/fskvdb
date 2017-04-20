#include "fskvdb_platform.h"

#include "../../common/hash/common_hash.h"
#include "../../logger/app_logger.h"

#include <stdlib.h>

//uint8_t Mbuffer[1024*1024*500];
uint8_t MMalloc[4096];
uint32_t MMpos=0;
#define align 512

FILE* fp;

uint8_t* DbStringToOpen = "./testdb.db";

#define ERROR_WAIT() if (device != 0) while(1==1) usleep(1000);

/*calculate the crc of a buffer*/
U16 fskvdb_crc16(void* buffer, U32 size){
    return CRC16(buffer,size);
}

U32 fskvdb_hash32(void* buffer, U32 size){
    return HASH32(buffer,size);
}

/*read write*/
fskvdb_stat fskvdb_write(DBID device,void* buffer, U32 size, U32 position){
  ERROR_WAIT();
  //FSKVDB_PRINT_FUNC();
  //FSKVDB_PRINT_INFO("W %u-%u %u",position,position+size,size);
  //memcpy(Mbuffer+position,buffer,size);
  fflush(fp);
  fseek(fp,position,SEEK_SET);
  if (fwrite(buffer,size,1,fp) != 1) 
	  return fskvdb_hw_fault;
  
  return fskvdb_ok;
}

fskvdb_stat fskvdb_writefill(DBID device,void* buffer,U32 buffersize , U32 size, U32 position){
	ERROR_WAIT();
  //FSKVDB_PRINT_FUNC();
  //FSKVDB_PRINT_INFO("Wf %u-%u %u",position,position+buffersize*size,size);
  //memset(Mbuffer+position,((U8*)buffer)[0],size);
  fflush(fp);
  fseek(fp,position,SEEK_SET);
  while(size){
	  if (fwrite(buffer,1,buffersize,fp) != buffersize) 
		  return fskvdb_hw_fault;
	  size--;
  }
  
  return fskvdb_ok;
}

fskvdb_stat fskvdb_read(DBID device,void* buffer, U32 size, U32 position){
	ERROR_WAIT();
  //FSKVDB_PRINT_FUNC();
  //FSKVDB_PRINT_INFO("R %u-%u %u",position,position+size,size);
  //memcpy(buffer,Mbuffer+position,size);
  fflush(fp);
  fseek(fp,position,SEEK_SET);
  if (fread(buffer,size,1,fp) != 1)
	  return fskvdb_hw_fault;
  return fskvdb_ok;
}

fskvdb_stat fskvdb_open(DBID device){
	ERROR_WAIT();
  FSKVDB_PRINT_FUNC();
  MMpos = 0;
  fp = fopen(DbStringToOpen,"a+");
  fclose(fp);
  fp = fopen(DbStringToOpen,"rb+");
  return fskvdb_ok;
}
fskvdb_stat fskvdb_close(DBID device){
	ERROR_WAIT();
  //FSKVDB_PRINT_FUNC();
  //fclose(fp);
  return fskvdb_ok;
}
fskvdb_stat fskvdb_endop(DBID device){
	ERROR_WAIT();
    //SafeHeapClear(&HEAP_FSKV);
	//FSKVDB_PRINT_FUNC();
	
	fflush(fp);
	
    MMpos = 0;
    return fskvdb_close(device);
}

fskvdb_stat fskvdb_malloc(DBID device,void** ptr,U32 buffersize){
	ERROR_WAIT();
  FSKVDB_PRINT_FUNC();
  //SafeMalloc(&HEAP_FSKV,ptr,buffersize);
  //*ptr = calloc(1,buffersize);
  *ptr = &MMalloc[MMpos * align];
  MMpos++;
  return fskvdb_ok;
}
fskvdb_stat fskvdb_free(DBID device,void** ptr){
	ERROR_WAIT();
  FSKVDB_PRINT_FUNC();
  //if (*ptr != 0)
      //free(*ptr);
  //SafeFree(&HEAP_FSKV,*ptr);
  return fskvdb_ok;
}

/*callback for IN/OUT op*/
fskvdb_stat fskvdb_callback(DBID device, U16 req_id , fskvdb_stat req_result ,U8* buffer, U32 size){
return fskvdb_ok;
}
