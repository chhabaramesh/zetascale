/*
 * Main in core structure for write serialization
 *
 * Author : Ramesh Chander
 */

#ifndef __WS_MAIN_H_
#define __WS_MAIN_H_

//#include <sys/uio.h>
//#include <linux/aio_abi.h>    
//#include <libaio.h>
#include "ws_utils.h"
#include "ws_alloc.h"
#include "ws_chunks.h"
#include "ws_map_table.h"
#include "ws_common.h"
#include "ws_lock.h"

#include "ws_log.h"

#define MAX_USER_THREADS 512
#define WS_NUM_STREAMS 7 

typedef struct ws_core_stats {
	uint64_t ws_total_app_writes;
	uint64_t ws_total_flushes;
	uint64_t ws_total_part_writes;
	uint64_t ws_total_ios;
	uint64_t ws_total_iios;
	uint64_t ws_partial_writes;
	uint64_t ws_internal_writes;
	uint64_t ws_internal_reads;
	uint64_t ws_total_writes;
	uint64_t ws_total_reads;
	uint64_t ws_blocks_used;

} ws_core_stats_t;

typedef struct ws_core_info {

	uint64_t signature;
	uint64_t storage_size; // In blks 
	uint64_t block_size; // in bytes
	uint64_t chunk_size; // In blocks
	uint64_t chunk_u_size; // In blocks
	uint64_t num_chunks;
	uint64_t max_offset;
	int op_pct;

	/*
	 * Chunk info
	 */
	int num_streams;
	ws_chunk_info_t *chunk_info; //data corresponding to each ws stream.


	/*
	 * Allocation info
	 */	
	
	ws_alloc_info_t *alloc_info;

	/*
	 * Map table
	 */
	ws_map_table_t *map_table;

	ws_map_table_t *map_table_rev;
	/*
	 * Heat map for blocks.
	 */
	void *hmap;
	bool hmap_init;

	/*
	 * WS logical range locking table.
	 */
	ws_chunk_locks_t *ws_locks;

	/*
	 * Throttle meta id
	 */
	int ops_thro_id;
	void *ops_thro_meta;
	uint64_t thro_speed;
	uint64_t ops_since_last_adjustment;
	uint64_t time_since_last_adjustment;
	uint64_t io_ops_since_last_adjustment;

	pthread_rwlock_t check_lock;

	int fd_links;

	ws_core_stats_t stats;
	uint64_t stream_counts[WS_NUM_STREAMS];
	ws_log_t log;

} ws_core_info_t;

int
ws_link_fd(int ws_fd, int link_fd);

bool
ws_init(uint64_t total_blks, uint64_t blk_size, uint64_t chunk_size, 
	ws_gc_config_t *gc_cfg, char *nvram_area, uint64_t nvram_area_size,
	 int dev_fd, int64_t thro_speed, uint64_t *max_logical_blk, FILE *log_fd,
	 bool log_enabled, bool reformat);
void
ws_close(int fd);

ssize_t
ws_read(int fd, char *buffer, size_t count, off_t offset);

ssize_t
ws_write(int fd, char *buffer, size_t count, off_t offset);

void
ws_get_stats(int fd, char *msg);

ssize_t ws_pwritev(int fd, const struct iovec *iov, int iovcnt,
                       off_t offset);
//int sd_io_setup(unsigned nr_events, aio_context_t *ctx_idp);
#if 0
int sd_io_setup(unsigned nr_events, void *ctx_idp);

//int sd_io_submit(aio_context_t ctx_id, long nr, struct iocb **iocbpp);
int sd_io_submit(uint64_t ctx_id, long nr, struct iocb **iocbpp);

//int sd_io_getevents(aio_context_t ctx_id, long min_nr, long nr,
int sd_io_getevents(uint64_t ctx_id, long min_nr, long nr,
		struct io_event *events, struct timespec *timeout);

//int sd_io_destroy(aio_context_t ctx_id);
int sd_io_destroy(uint64_t ctx_id);
#endif 

#endif //end of file
