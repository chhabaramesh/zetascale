#ifndef __NV_CHUNKS_H_
#define __NV_CHUNKS_H_

#include "ws_utils.h"
#include "ws_queue.h"
#include "string.h"

/*
 * This file has structure related to NVRAM chunks in nvram area and in core
 * structure related to handling of chunks
 *
 * Author: Ramesh Chander 
 */



typedef enum ws_chunk_state {
	WS_CHUNK_FREE = 1,
	WS_CHUNK_ACTIVE,
	WS_CHUNK_FLUSHING
} ws_chunk_state_t;

typedef struct ws_phys_chunk {
	int chunk_num;
	int slot_num;
	int num_blks;
	int next_blk;
	ws_chunk_state_t state;

} ws_phys_chunk_t;

typedef struct ws_mem_chunk {
	ws_phys_chunk_t *pchunk;
	struct ws_mem_chunk *next;

	int ref_count;
	pthread_rwlock_t flush_lock;
} ws_mem_chunk_t;

typedef struct ws_chunk_list {
	ws_mem_chunk_t *chunk;
	pthread_mutex_t lock;
	pthread_cond_t empty_cv;
} ws_chunk_list_t;

typedef struct ws_active_chunk {
	ws_mem_chunk_t *chunk;
	pthread_mutex_t lock;
	uint64_t allocated_chunk; //placeholder to alloc chunk atomically
} ws_active_chunk_t;

typedef uint64_t (*alloc_callback_t) (void *context, void *wait_lock, uint64_t *allocated_chunk, bool internal);

typedef void (*active_callback_t) (void *context, uint64_t allocated_chunk);


typedef struct ws_chunk_info {

	int stream_num;
	ws_chunk_list_t *free_list;	// free list
	ws_chunk_list_t *flush_list;	// free list
	ws_gc_queue_t   *gc_queue;      // chunk list for gc
	ws_active_chunk_t *active_chunk;// active chunk info
	char *nvram_area;
	uint64_t nvram_area_size;
	uint64_t chunk_size;
	uint64_t chunk_size_blks;
	int blk_size;
	uint64_t num_chunks;
	uint64_t total_chunks;
	uint64_t max_offset_written;
	bool bypass_ops;

	int dev_fd;

	ws_mem_chunk_t *mem_chunks;	// Allocate mem chunks
	ws_phys_chunk_t **phys_chunks;


	/*
	 * Pointer to ws_info struct
	 */
	void *ws_info;
	alloc_callback_t alloc_callback;
	active_callback_t active_callback;

	uint64_t allocated_chunks;
} ws_chunk_info_t;

/*
 * Forward declarations
 */

void ws_chunk_list_init(ws_chunk_list_t *list);
void ws_chunk_list_add(ws_chunk_list_t *list, ws_mem_chunk_t *chunk);
void ws_init_chunk_active(ws_chunk_info_t *ws_chunk_info, ws_active_chunk_t *active_chunk);

void
ws_chunk_mem_init(char *nvram_area, uint64_t nvram_area_size,
		  uint64_t chunk_size, int dev_fd, int blk_size, uint64_t total_chunks,
		  alloc_callback_t alloc_callback, active_callback_t active_callback, void *ws_info, ws_chunk_info_t *chunk_info);

ssize_t
ws_read_chunk_io(ws_chunk_info_t *chunk_info, uint64_t chunk_num, uint64_t blk_num, int num_blks, char *buffer);

void
ws_write_chunk_io(ws_chunk_info_t *chunk_info, ws_mem_chunk_t *chunk,
	    int start_blk, int num_blks, char *buffer, uint64_t log_blk);

bool 
ws_active_chunk_get_blks(ws_chunk_info_t *ws_chunk_info, int blks, uint64_t *blk,
			 uint64_t *phys_chunk_num, ws_mem_chunk_t **chunk, uint64_t *blks_allocated,
			 bool internal, uint64_t offset);


ws_mem_chunk_t *
ws_chunk_list_remove_ns(ws_chunk_list_t *list, int wait_ref);

void
ws_flush_chunk_data(ws_chunk_info_t *chunk_info, ws_mem_chunk_t *chunk);

void
ws_chunk_attemp_flush(ws_chunk_info_t *chunk_info);

ssize_t
ws_read_phys_chunk(ws_chunk_info_t *chunk_info, uint64_t chunk_num, 
		   uint64_t blk_num, int num_blks, char *buffer);
ssize_t
ws_read_chunk_io_streams(ws_chunk_info_t *chunk_infos, int num_streams, uint64_t chunk_num, uint64_t blk_num, int num_blks, char *buffer);

void
ws_chunk_mem_close(ws_chunk_info_t *chunk_info);

#endif //end of file 
