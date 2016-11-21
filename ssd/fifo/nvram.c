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
 * NVRAM device related utility functions.
 *
 * Author: Ramesh Chander.
 * Created on June, 2014.
 * (c) Sandisk Inc.
 */
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "nvram.h"
#include <platform/assert.h>
#include <platform/logging.h>

#undef  mcd_log_msg
#define mcd_log_msg(id, args...)                                        \
    plat_log_msg( id,                                                   \
                  PLAT_LOG_CAT_SDF_APP_MEMCACHED_RECOVERY,              \
                  ##args)

#define NUM_FLOG_FILES 4
#define NV_START_FD 1
static nv_fd_t nvd_fd[NUM_FLOG_FILES];
static int nvd_fd_count = NV_START_FD;
static int nv_init_done = 0;
static uint64_t nv_dev_offset = 0;
static int nv_dev_fd = -1;
int nv_dev_block_size = 0;
uint64_t nv_flog_file_size = 0;
static uint64_t nv_flog_total_size = 0;

pthread_mutex_t nv_mutex = PTHREAD_MUTEX_INITIALIZER;

void inline
nv_flush(char *buf, size_t len)
{
	//clflush_test(buf, len);
}

void
nv_fdatasync(int fd)
{
	plat_assert(fd <= NUM_FLOG_FILES);
	/*
 	 * fsync the main device.
 	 */
	fdatasync(nv_get_dev_fd());
}


int 
nv_get_dev_fd()
{
	return nv_dev_fd;
}

void
nv_set_dev_fd(int fd)
{
	nv_dev_fd = fd;
}

int 
nv_get_block_size()
{
	struct stat s;
	int block_size = 0;

	int ret = fstat(nv_dev_fd, &s);
	if(ret >= 0) {
	    block_size = s.st_blksize;
	}

	return block_size;
}

int nv_check_min_size(int fd)
{
	return nv_flog_file_size < nv_dev_block_size;
}

bool
nv_init(uint64_t block_size, int num_recs, uint64_t dev_offset, int fd)
{

	mcd_log_msg(160314, PLAT_LOG_LEVEL_INFO,
			"Init NVLOG for flog dev offset=%ld, fd=%d.\n", dev_offset, fd);

	pthread_mutex_lock(&nv_mutex);
	if (nv_init_done) {
		goto exit;
	}

	nv_init_done = 1;

	nv_dev_block_size = block_size;
	nv_flog_file_size = block_size * num_recs;
	nv_flog_total_size = nv_flog_file_size * NUM_FLOG_FILES;	
	nv_dev_offset = dev_offset;

	nv_dev_fd = fd;
	plat_assert(nv_dev_fd >= 0);
	
	memset(nvd_fd, 0, sizeof(nv_fd_t) * NUM_FLOG_FILES);

exit:
	pthread_mutex_unlock(&nv_mutex);
	return true;
}

void
nv_reformat_flog()
{
	int dev_fd = nv_get_dev_fd();
	char *zbuf = NULL;
	int i =0;
	int ret = 0;

	uint64_t blks = nv_flog_total_size/nv_dev_block_size;


//	fprintf(stderr, "Reformat nvram with %ld blks.\n", blks);

	zbuf = (char *) malloc(nv_dev_block_size);
	plat_assert(zbuf != NULL);
	memset(zbuf, 0, nv_dev_block_size);
	for (i = 0 ; i < blks; i++) {
		ret = pwrite(dev_fd, zbuf, nv_dev_block_size, i * nv_dev_block_size);
		plat_assert(ret > 0);
	}

	nv_fdatasync(2); //fd is  useless here 
	free(zbuf);
}

static void
nv_zero_out(int fd, uint64_t length)
{
	int i = 0;
	int ret = 0;
	uint64_t blks = length / nv_dev_block_size;
	
	char *zbuf = NULL;

	plat_assert(!(length % nv_dev_block_size));

	zbuf = (char *) malloc(nv_dev_block_size);
	plat_assert(zbuf != NULL);
	memset(zbuf, 0, nv_dev_block_size);

	for (i = 0 ; i < blks; i++) {
		ret = nv_write(fd, zbuf, nv_dev_block_size, i * nv_dev_block_size);
		if (ret <= 0) {
			mcd_log_msg(160315, PLAT_LOG_LEVEL_INFO,
				"NVLOG failed to truncate file fd=%d.\n", fd);
			exit(-1);
		}
	}
	nv_fdatasync(2); //fd is  useless here 
	free(zbuf);

}

int
nv_open(char *name, int flags, mode_t mode)
{
	int i = 0;
	int fd = nvd_fd_count;
	pthread_mutex_lock(&nv_mutex);

	/*
	 * Check if file already exists, if yes, return that.
	 */
	for (i = 1 ; i < NUM_FLOG_FILES; i++) {
		if (strcmp(nvd_fd[i].name, name) == 0) {
			fd = i;
			nvd_fd[fd].size = nv_flog_file_size;
			nvd_fd[fd].offset = 0;
			if (flags & O_CREAT|O_TRUNC|O_WRONLY) {
				/*
				 * open in truncate mode Clear the range of blocks
				 */
				mcd_log_msg(160316, PLAT_LOG_LEVEL_INFO, 
						"Truncating/new create fd=%d size=%lu.\n", nvd_fd[fd].fd, nv_flog_file_size);
				nv_zero_out(fd, nv_flog_file_size);
			}

			goto exit;

		}	
	}

	nvd_fd_count++;
	nvd_fd[fd].fd = fd;


	nvd_fd[fd].offset_in_dev = nv_dev_offset + 
					nv_flog_file_size * (nvd_fd[fd].fd - NV_START_FD);
	nvd_fd[fd].size = nv_flog_file_size;
	nvd_fd[fd].offset = 0;

	mcd_log_msg(160317, PLAT_LOG_LEVEL_INFO,
		"NVLOG file open fdf =%d, offset_in_dev=%ld, size=%ld.\n",
		 nvd_fd[fd].fd,  nvd_fd[fd].offset_in_dev, nvd_fd[fd].size);

	strcpy(nvd_fd[fd].name, name);

exit:
	pthread_mutex_unlock(&nv_mutex);

	return fd;
}

void
nv_close(int fd)
{
	return;
}


ssize_t 
nv_read(int fd, void *buf, size_t nbytes)
{
	int ret = 0;
	off_t off = nvd_fd[fd].offset + nvd_fd[fd].offset_in_dev;

	mcd_log_msg(160318, PLAT_LOG_LEVEL_DEBUG,
		"NVLOG file read fd=%d, offset=%ld, length=%ld.\n", fd, off, nbytes);

	plat_assert(fd > 0 && fd < NUM_FLOG_FILES);
	plat_assert(fd == nvd_fd[fd].fd);
	if (off >= (nvd_fd[fd].size + nvd_fd[fd].offset_in_dev)) {
		return 0;
	}

	plat_assert(off <= (nv_flog_file_size + nvd_fd[fd].offset_in_dev));

	ret = pread(nv_dev_fd, buf, nbytes, off);
	if (ret <= 0) {
		mcd_log_msg(160319, PLAT_LOG_LEVEL_INFO,
			"NVLOG file read failed on fd=%d, offset=%ld, length=%ld.\n", fd, off, nbytes);
		return 0;
	}

	nvd_fd[fd].offset += nbytes;	

	return nbytes;
}

ssize_t 
nv_write(int fd, const void *buf, size_t nbytes, off_t off)
{
	int ret = 0;

	plat_assert(fd > 0 && fd < NUM_FLOG_FILES);
	plat_assert(fd == nvd_fd[fd].fd);
	if (off >= nvd_fd[fd].size) {
		plat_assert(0);
		return 0;
	}

	off += nvd_fd[fd].offset_in_dev;

	mcd_log_msg(160320, PLAT_LOG_LEVEL_DEBUG,
		"NVLOG file write fd=%d, offset=%ld, length=%ld.\n", fd, off, nbytes);

	ret = pwrite(nv_dev_fd, buf, nbytes, off);
	if (ret <= 0) {
		mcd_log_msg(160321, PLAT_LOG_LEVEL_INFO,
			"NVLOG file write failed on fd=%d, offset=%ld, length=%ld.\n", fd, off, nbytes);
		exit(-1);	
	}

	return nbytes;
}

int 
nv_fseek(int fd, long offset, int whence)
{
	plat_assert(0);
	return 0;
}

long 
nv_ftell(int fd)
{
	plat_assert(fd > 0 && fd < NUM_FLOG_FILES);
	plat_assert(fd == nvd_fd[fd].fd);

	return nvd_fd[fd].offset;
}
