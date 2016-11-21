/*
 * Functions and structures related to ws allocation and garbage collection
 */
#ifndef __WS_ALLOC_H_
#define __WS_ALLOC_H_

#include "ws_utils.h"
#include "ws_queue.h"


typedef ssize_t (*read_pchunk_callback_t) (void *context, uint64_t chunk_num,
					 uint64_t blk_num, int num_blks, char *buffer);

typedef void (*write_blk_callback_t) (void *context, uint64_t log_blk, int num_blks, char *buffer);

typedef int (*realloc_chunk_callback_t) (void *context, uint64_t chunk_num, int *lock_failed);


#define GC_SPEED_SAMPLES 10
typedef struct ws_gc_speed_info {
	int64_t speed_samples[GC_SPEED_SAMPLES];
	int insert_index;
} ws_gc_speed_info_t;

//#define WS_SPACE_AREA_CUNKS (1024)
#define WS_SPACE_AREA_CUNKS (16)
//#define WS_SPACE_AREA_CUNKS (10240)

typedef enum ws_alloc_chunk_state {
	WS_ALLOC_CHUNK_FREE = 10,
	WS_ALLOC_CHUNK_ACTIVE,
} ws_alloc_chunk_state_t;
#if 1
typedef struct ws_chunk_bmap {
	int chunk_used; //Used to alloc or free a chunk, used by chunk allocator
	uint32_t num_blks_used; //Keeps count of blk used, used by blk allocation within chunk
	void *hint; //hint to store pointers from histo queue
	ws_alloc_chunk_state_t state;
//	int signature;
} ws_chunk_bmap_t;
#endif 

typedef enum ws_zone_state {
	WS_ZONE_FREE = 1,
	WS_ZONE_ALLOCATING,
	WS_ZONE_GC,
} ws_zone_state_t;

typedef struct ws_space_zone_info {
	uint64_t start_chunk;
	int free_chunks; //changes from gc and allocation, atomic  variable
	uint32_t free_chunk_marker; //changes from gc and allocation, atomic variable
				// Chunk number within zone that can be allocated.
	uint32_t num_blks_in_chunk; //fixed
	int      gc_threads;
	//uint64_t chunk_blks_used[WS_SPACE_AREA_CUNKS]; // number of blks used in chunk
	ws_zone_state_t state;
	ws_chunk_bmap_t chunk_blks_used[WS_SPACE_AREA_CUNKS + 2]; // number of blks used in chunk

	// lock : taken write only during gc for this zone, taken read for allocation
//	pthread_rwlock_t lock;
} ws_space_zone_info_t;

typedef struct ws_alloc_info {

	uint64_t signature;

	uint64_t block_size;
	uint64_t blocks_in_chunk;

	uint64_t total_chunks; //never changes

	uint64_t allocated_chunks; //changes in chunk allocation, atomic

	uint64_t chunks_allocated; // used for flow control
	uint64_t chunks_freed; // used for flow control

	uint64_t usable_chunks; //chunks that be be allocated

	uint64_t total_zones; // total chunks / WS_SPACE_AREA_CUNKS



	//// Markers to maintain full storage as circular buffer. ////

	pthread_mutex_t markers_lock;

	uint64_t alloc_marker; //zone number where chance to get free chunks
			// Changes from allocation and gc, atomic variable
	int64_t gc_marker; // zone number where gc should start from
			// Changes from gc, atomic variable
#if 0
	int64_t ac_gc_dist; //distance in gc and alloc zone
			// We need this since other two counter rolls over.


#endif
	pthread_cond_t gc_wakeup_cv;
	pthread_mutex_t gc_wakeup_lock;
	pthread_cond_t alloc_wakeup_cv;	
//	pthread_mutex_t alloc_wakeup_lock;

	uint64_t ws_gc_chunk_threshold;
	uint64_t ws_gc_chunks_threshold_max;
	int gc_ops_throttle_id;
	int gc_free_throttle_id;
	bool throttled_gc; //Set when space used is < 80%
	int gc_state;
	bool xcopy_emu;
	ws_gc_speed_info_t gc_speed_info;

	// Stats for GC 
	uint64_t gc_chunks_freed;
	uint64_t gc_blocks_moved;
	uint64_t ws_chunks_freed_inline;
	uint64_t ws_chunk_alloced_from_list;
	uint64_t ws_chunk_alloced;
	uint64_t ws_chunks_try_free_inline;
	uint64_t gc_chunks_processed;
	void *alloc_histo;


	ws_space_zone_info_t *space_zone_info; //info about all space areas
	ws_gc_queue_t free_chunks_list;

	pthread_t * gc_pids;

	read_pchunk_callback_t read_pchunk_callback;
	write_blk_callback_t write_blks_callback;
	realloc_chunk_callback_t realloc_chunk_callback;	

	void *ws_info;

	// We dont need any lock for this structure, we will use atomic
	// operations to update marker.
} ws_alloc_info_t;

typedef enum {
	DEFAULT_GC = 0,
	ZONE_BASED_GC = 0,
	QUEUE_BASED_GC,
	HISTO_QUEUE_BASED,
	MAX_GC_ALGO_TYPE
} gcType_t ;

typedef struct ws_gc_config {
	int num_threads;
	int op_speed_psec;
	int free_speed_psec;
	int chunk_gc_threshold_pct;
	gcType_t  gc_type;
	int gc_start_pct;
	int gc_rush_pct;
	bool xcopy_emu;
	int num_streams;
} ws_gc_config_t;

#if 0
void
ws_alloc_init(uint64_t total_chunks, uint64_t start_chunk, uint64_t block_size, 
	      uint64_t chunk_size_blks, int num_gc_threads, void *ws_info, 
	      read_pchunk_callback_t read_callback, write_blk_callback_t write_callback,
	      ws_alloc_info_t *alloc_info, uint64_t *reserved_blks);
#endif 

void
ws_alloc_init(uint64_t total_chunks, uint64_t start_chunk, uint64_t block_size, 
	      uint64_t chunk_size_blks, ws_gc_config_t *gc_cfg, void *ws_info, 
	      realloc_chunk_callback_t realloc_callback,
	      ws_alloc_info_t *alloc_info, uint64_t *reserved_blks);
uint64_t 
ws_alloc_chunk(ws_alloc_info_t *ws_alloc_info, void *wait_lock, uint64_t *allocate_chunk, bool internal);

void
ws_free_chunk_blks(ws_alloc_info_t *alloc_info, uint64_t chunk_num, 
		   uint64_t num_blks, bool internal);

bool 
ws_get_gc_state(ws_alloc_info_t *alloc_info);

bool 
ws_gc_xcopy(ws_alloc_info_t *alloc_info);

int
ws_alloc_get_gc_speed(ws_alloc_info_t *alloc_info);

void
ws_set_chunk_inactive(ws_alloc_info_t *alloc_info, uint64_t chunk_num);

void
ws_rec_allo_chunk_blks(ws_alloc_info_t *alloc_info, uint64_t blk_num);

#endif //end of file
