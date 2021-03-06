//----------------------------------------------------------------------------
// ZetaScale
// Copyright (c) 2016, SanDisk Corp. and/or all its affiliates.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published by the Free
// Software Foundation;
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License v2.1 for more details.
//
// A copy of the GNU Lesser General Public License v2.1 is provided with this package and
// can also be found at: http://opensource.org/licenses/LGPL-2.1
// You should have received a copy of the GNU Lesser General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA.
//----------------------------------------------------------------------------

/*
 * ZS Logging Container
 *
 * Copyright 2015 SanDisk Corporation.  All rights reserved.
 */
#include "common/zstypes.h"
#include "api/fdf_internal_cb.h"
#include "api/fdf_internal.h"

#ifndef __LC_H
#define __LC_H


#define LOGSTATS_ITEMS() \
/* 0 */		item(LOGSTAT_NUM_OBJS, = 0) \
		item(LOGSTAT_DELETE_CNT, /* default */) \
		item(LOGSTAT_FLUSH_CNT, /* default */) \
		item(LOGSTAT_MPUT_IO_SAVED, /* default */)  \
		item(LOGSTAT_MPUT_CNT, /* default */)  \
/* 5 */		item(LOGSTAT_NUM_MPUT_OBJS, /* default */)  \
		item(LOGSTAT_READ_CNT, /* default */)  \
		item(LOGSTAT_WRITE_CNT, /* default */)  \
		item(LOGSTAT_WRITE_SLOWPATH, /* default */)  \
		item(LOGSTAT_MPUT_SLOWPATH_TMPBUF, /* default */)  \
/* 10 */	item(LOGSTAT_MPUT_SLOWPATH_GTTMPBUF, /* default */)  \
		item(LOGSTAT_MPUT_SLOWPATH_FIRSTREC, /* default */)  \
		item(LOGSTAT_MPUT_SLOWPATH_NOSPC, /* default */)  \
		item(LOGSTAT_MPUT_SLOWPATH_DIFFSTR, /* default */)  \
		item(LOGSTAT_SYNC_DONE, /* default */)  \
		item(LOGSTAT_SYNC_SAVED, /* default */)  \

typedef enum {
#define item(caps, value) \
	    caps value,
	    LOGSTATS_ITEMS()
#undef item
	    N_LOGSTATS
} logstats_t;

typedef struct log_stats {
	uint64_t	stat[N_LOGSTATS];
} log_stats_t;


ZS_status_t	lc_init(struct ZS_state *zs_state, bool),
		lc_open( struct ZS_thread_state *, ZS_cguid_t),
		lc_delete_container( ZS_cguid_t),
		lc_write( struct ZS_thread_state *, ZS_cguid_t, char *, uint32_t, char *, uint64_t),
		lc_delete( struct ZS_thread_state *, ZS_cguid_t, char *, uint32_t),
		lc_trim( struct ZS_thread_state *, ZS_cguid_t, char *, uint32_t),
		lc_read( struct ZS_thread_state *, ZS_cguid_t, char *, uint32_t, char **, uint64_t *),
		lc_enum_start( struct ZS_thread_state *, ZS_cguid_t, void *, char *, uint32_t),
		lc_enum_next( struct ZS_thread_state *, void *, char **, uint32_t *, char **, uint64_t *),
		lc_enum_finish( void *), 
		lc_mput(struct ZS_thread_state *zs_ts, ZS_cguid_t cguid, uint32_t num_objs, ZS_obj_t *objs,
					        uint32_t flags, uint32_t *objs_written);

ZS_status_t LC_init(void **lc, ZS_cguid_t cguid);
ZS_status_t get_lc_num_objs(ZS_cguid_t cguid, uint64_t *num_objs);
ZS_status_t get_log_container_stats(ZS_cguid_t cguid, ZS_stats_t *stats);
#endif
