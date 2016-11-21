/*
 *  Logical to physica block number mapping table related structure and function.
 * 
 * Author : Ramesh Chander 
 */
#ifndef __WS_MAP_TABLE_H_
#define __WS_MAP_TABLE_H_

#include "ws_utils.h"

typedef struct ws_map_table {
	uint64_t signature;
	uint64_t num_entries;
	uint64_t *entries;

	int fd;
	int64_t offset;
	int64_t block_size;
} ws_map_table_t;

void
ws_map_table_init(ws_map_table_t *table, uint64_t num_blks, 
	int64_t block_size, int fd, int64_t offset, bool reformat);

uint64_t
ws_map_table_set_entry(ws_map_table_t *table, uint64_t log_blk, uint64_t phys_blk);

void
ws_map_table_set_entries(ws_map_table_t *table, uint64_t log_blk, 
			int num_blks, uint64_t phys_blk, uint64_t *old_phys_entries);

uint64_t
ws_map_table_get_entry(ws_map_table_t *table, int log_blk);

void
ws_map_table_get_entries(ws_map_table_t *table, int log_blk, int num_blks, uint64_t *phys_blks);

uint64_t
ws_map_table_get_entry(ws_map_table_t *table, int log_blk);

void
ws_map_table_get_entries(ws_map_table_t *table, int log_blk, int num_blks, uint64_t *phys_blks);

void
ws_map_table_close(ws_map_table_t *table);

#endif
