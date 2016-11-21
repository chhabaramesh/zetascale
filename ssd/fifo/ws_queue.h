#ifndef __NV_QUEUE_H_
#define __NV_QUEUE_H_

#include "ws_utils.h"
#include "string.h"

/*
 * This file has structure related to NVRAM chunks in nvram area and in core
 * structure related to handling of chunks
 *
 * Author: Ramesh Chander 
 */


typedef struct ws_gc_list_entry {
	uint64_t chunk_num;
	struct ws_gc_list_entry *next;
} ws_gc_list_entry_t;

typedef struct ws_gc_queue {
	ws_gc_list_entry_t *mra_chunk; // Most recently allocated chunk
	ws_gc_list_entry_t *lra_chunk; // Least recently allocated chunk
	uint64_t    count;
	pthread_mutex_t lock;
	pthread_cond_t empty_cv;
} ws_gc_queue_t;

void
ws_gc_queue_init(ws_gc_queue_t *queue);

void
ws_chunk_queue_add(ws_gc_queue_t *queue, uint64_t chunk_num);

uint64_t 
ws_chunk_queue_remove(ws_gc_queue_t *queue);

uint64_t 
ws_chunk_queue_remove_ns(ws_gc_queue_t *queue);
#endif //end of file 
