#include <string.h>


#include "fskvdb/fskvdb.h"


uint8_t buffer512[512];
uint8_t buffer4096[4096];
uint8_t buffer4096temp[4096];


t_uint8 testbuff[] = {"01010101010101010101010101010101010101010101010110101010101010101001010101010101010101010101010101"};


#define CHECK_BYTE 0xBB

t_uint8 TestBuffer[4096];

#define FILETEST "0:/test.fil"


#define DB_IDENT 0
#define DB_TEST_KEY 0

#define KV_NUMBER_TEST 12000

t_uint32 dbCreate(t_uint32 size){
  counterDevTypedef Timer;
  CounterSetSync(&Timer,0);
  
    fskvdb_options_ctx options;
    fskvdb_stat ret_val;

    options.DeviceID = DB_IDENT;
    options.FixedLen = FSKVDB_OPT_KVSIZEMAXCLUSTER;
    options.DataCrc = 1;
    options.AutoKey = 0;
    options.SearchIndexMode = fskvdb_SearchIndexMode_hash /* fskvdb_SearchIndexMode_linear*/;
    options.DataAlign = 512;
    options.Preallocate = 1;
    options.DataMaxNum = KV_NUMBER_TEST; /* 0 for infinite*/
    options.AttributeLenght = 0;
    
    
  
  while(size != 0){
    ret_val = fskvdb_DBCreate(&options,1234);
    size--;
  }
  
  return CounterGet(&Timer);
}

t_uint32 dbAddKey(t_uint32 size){
  counterDevTypedef Timer;
  CounterSetSync(&Timer,0);
  fskvdb_stat ret_val;
  
  while(size != 0){
    ret_val = fskvdb_DBAdd( &dbctx , &size, TestBuffer, 480 , 0);
    if (ret_val != fskvdb_ok){
      APP_PRINT_INFO("DB failed create");
      return 0;
    }
    size--;
  }
  
  return CounterGet(&Timer);
}

t_uint32 dbGetKey(t_uint32 size){
  counterDevTypedef Timer;
  CounterSetSync(&Timer,0);
  t_uint32 size2;
  fskvdb_stat ret_val;
  while(size != 0){
    ret_val = fskvdb_DBGetValue( &dbctx , &size, TestBuffer, &size2);
      if (ret_val != fskvdb_ok){
      APP_PRINT_INFO("DB failed get");
      return 0;
    }
    size--;
  }
  
  return CounterGet(&Timer);
}

t_uint32 dbDeleteKey(t_uint32 size){
  counterDevTypedef Timer;
  CounterSetSync(&Timer,0);
  fskvdb_stat ret_val;
  while(size != 0){
    ret_val = fskvdb_DBDelete( &dbctx , &size);
         if (ret_val != fskvdb_ok){
      APP_PRINT_INFO("DB failed delete");
      return 0;
    }
    size--;
  }
  
  return CounterGet(&Timer);
}

#define KVSIZE 2900

int main(void)
{   
/*DB tests*/
    fskvdb_options_ctx options;
    fskvdb_ResetOptions(&options);
    
    fskvdb_stat ret_val = 0;

    options.AttributeLenght = 0;
    options.DeviceID = DB_IDENT;
    options.FixedLen = KVSIZE;
    options.DataMaxNum = 1024;
    
    
    ret_val = fskvdb_DBCreate(&options,1234);
    ret_val = fskvdb_DBOpen(DB_IDENT,&dbctx);
    
    U32 testkey = 0;
    U32 tempsize=0;
    
    counterDevTypedef CounterDB;
    CounterSetSync(&CounterDB,0);
    
    while(testkey < 1024 && ret_val == 0){
      ret_val = fskvdb_DBAdd( &dbctx , &testkey, TestBuffer, KVSIZE , 0);
      testkey++;
    }
    testkey = 0;
    
    uint32_t timetodb = ToMillis(CounterGet(&CounterDB));
    CounterSetSync(&CounterDB,0);
    
    while(testkey < 1024 && ret_val == 0){
      ret_val = fskvdb_DBGetValue( &dbctx , &testkey, TestBuffer, &tempsize );
      testkey++;
    }
    
    timetodb = ToMillis(CounterGet(&CounterDB));

  //MountFS();
  //f_mkfs("0:",0,32768);

/*integrity check*/
int i=0;
while(TestBuffer[i] == CHECK_BYTE) i++;

if (i != sizeof(TestBuffer))
    APP_PRINT_INFO("Integry check failed");
    else
      APP_PRINT_INFO("Integry check ok");

    
    ret_val = 0;
    ret_val = fskvdb_DBClose(&dbctx);
    ret_val = fskvdb_DBOpen(DB_IDENT,&dbctx);
    if (ret_val != 0) while(1==1);
    testkey = 0;
    while(testkey < 1024 && ret_val == 0){
      memset(TestBuffer,0,sizeof(TestBuffer));
      ret_val = fskvdb_DBGetValue( &dbctx , &testkey, TestBuffer, &tempsize );
      i=0;
      while(TestBuffer[i] == CHECK_BYTE) i++;
      if (i < 2900) break;
      testkey++;
    }
    
    if (testkey != 1024) while(1==1);
    
    /*main loop*/
    while(1==1) {

    };
}



