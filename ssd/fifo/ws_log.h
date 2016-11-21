#ifndef __WS_LOG_H__
#define __WS_LOG_H__

#include <stdint.h>
#include <pthread.h>

#include "ws_map_table.h"

#define WS_LOG_SIZE (4L * 1024 * 1024 * 1024)
#define WS_LOG_FLUSH_CHUNK_SIZE (1024 * 1024)

typedef struct ws_core_info ws_core_info_t;

typedef union {
	struct {
		uint64_t o;
		uint64_t n;
	};
	struct {
		uint64_t lsn;
		uint64_t checksum;
	};
} ws_log_rec_t;

typedef struct {
	uint64_t lsn;
	uint64_t checksum;
} ws_log_blk_hdr_t;

typedef struct {
	uint64_t lsn;
	uint64_t start_offset;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	ws_log_rec_t* bufs[2];
	int flush_buf, cur_buf;
	uint64_t buf_ptr;
	uint64_t map_ptr;
	uint64_t map_ratio;
	uint64_t num_chunks;
	int dev_fd;

	uint64_t stat_num_flushes;
	uint64_t stat_num_writes;

	pthread_t thread_id;
	int shutdown;

	ws_map_table_t* map;
} ws_log_t;

uint64_t ws_log_init(ws_core_info_t *core_info, int dev_fd, uint64_t total_blks, uint64_t blk_size);

uint64_t ws_log_write_no_lock(ws_log_t* log, uint64_t o, uint64_t n);

int ws_log_recovery(ws_log_t* log);

void ws_log_destroy(ws_log_t* log);

#endif // __WS_LOG_H__
