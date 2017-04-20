#ifndef COMMON_FSKVDB_LINEARAUTO_h
#define COMMON_FSKVDB_LINEARAUTO_h

#include "fskvdb.h"

fskvdb_stat fskvdb_createLinearTable(fskvdb_ctx* ctx,U32 TableSize, U32 ListSize, U16 ByteSearchValue, U32 write_position, fskvdb_table_ctx* hashctx);
fskvdb_stat fskvdb_readLinearCtx(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx, U32 position);
fskvdb_stat fskvdb_LinearGetKeyPosition(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx, void* key, U32* position);
fskvdb_stat fskvdb_LinearAddKey(fskvdb_ctx* ctx,fskvdb_table_ctx* hashctx, void* key,DB_PTR DataPosition);

U32 fskvdb_linear_TableSize(fskvdb_table_ctx* hashctx);

#endif /*COMMON_FSKVDB_LINEARAUTO_h*/
