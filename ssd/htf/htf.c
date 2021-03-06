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
 * File:   htf.c
 * Author: Brian O'Krafka
 *
 * Created on January 8, 2009
 *
 * (c) Copyright 2009, Schooner Information Technology, Inc.
 * http://www.schoonerinfotech.com/
 *
 * $Id: htf.c 308 2008-02-20 22:34:58Z briano $
 */

#define _HTF_C

#include <stdint.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdarg.h>
#include <aio.h>
#include "platform/logging.h"
#include "fth/fth.h"
#include "ssd/ssd.h"
#include "ssd/ssd_local.h"
#include "htf.h"
#include "utils/hash.h"
#include "common/sdftypes.h"

struct flashDev *htf_flashOpen(char *devName, int flags) 
{
    plat_log_msg(21714, PLAT_LOG_CAT_FLASH, PLAT_LOG_LEVEL_FATAL,
		 "htf_flashOpen not implemented!");
    plat_abort();
}

struct shard *htf_shardCreate(struct flashDev *dev, uint64_t shardID, int flags, uint64_t quota, unsigned maxObjs) 
{
    plat_log_msg(21715, PLAT_LOG_CAT_FLASH, PLAT_LOG_LEVEL_FATAL,
		 "htf_shardCreate not implemented!");
    plat_abort();
}


int htf_flashGet(struct ssdaio_ctxt *pctxt, struct shard *shard, struct objMetaData *metaData,
     char *key, char **dataPtr, int flags)
{
    plat_log_msg(21716, PLAT_LOG_CAT_FLASH, PLAT_LOG_LEVEL_FATAL,
		 "htf_flashGet not implemented!");
    plat_abort();
}

int htf_flashPut(struct ssdaio_ctxt *pctxt, struct shard *shard, struct objMetaData *metaData,
     char *key, char *data, int flags)
{
    plat_log_msg(21717, PLAT_LOG_CAT_FLASH, PLAT_LOG_LEVEL_FATAL,
		 "htf_flashPut not implemented!");
    plat_abort();
}



