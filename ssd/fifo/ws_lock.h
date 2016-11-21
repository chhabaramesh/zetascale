#ifndef __WS_LOCK_H__
#define __WS_LOCK_H__

#include "ws_utils.h"
#include "ws_alloc.h"
#include "ws_error.h"
#include "ws_common.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

typedef struct ws_chunk_locks {
	uint64_t chunk_size_blks;
	uint64_t blk_size;
	uint64_t num_entries;
	//pthread_mutex_t *locks;
	pthread_rwlock_t *locks;
} ws_chunk_locks_t;

void
ws_locks_init(ws_chunk_locks_t *ws_locks, uint64_t num_chunks, uint64_t chunk_size, uint64_t block_size);

bool
ws_lock_range(ws_chunk_locks_t *ws_locks, uint64_t start_blk, 
	     int num_blks, bool try, bool write);
void
ws_unlock_range(ws_chunk_locks_t *ws_locks, uint64_t start_blk, int num_blks);

bool
ws_locks_lock_blks(ws_chunk_locks_t *ws_locks, uint64_t *log_blks, int num_blks,
		   bool try, bool write);

void
ws_locks_unlock_blks(ws_chunk_locks_t *ws_locks, uint64_t *log_blks, int num_blks);


#endif
