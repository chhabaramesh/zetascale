#ifndef __ZS_AIO_H__
#define __ZS_AIO_H__

#define ZS_AIO_MAX_IOS 128

#include <stdint.h>
#include <string.h>
//#include <linux/aio_abi.h>
#include <libaio.h>
//#include <assert.h>

//#include <libaio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>

typedef struct zs_iocb {
	struct iocb *cb;
	int ret;
	int done;
} zs_iocb_t;

typedef struct zs_aio_ctxt {
	int64_t io_count;
	int pending_ios;
	zs_iocb_t zs_iocbs[ZS_AIO_MAX_IOS];
	struct io_event events[ZS_AIO_MAX_IOS];
	io_context_t ctxt;
} zs_aio_ctxt_t;

extern void zs_aio_init(zs_aio_ctxt_t *zs_aio);
extern int zs_aio_submit(zs_aio_ctxt_t *zs_aio, int fd, off_t offset, size_t length, void *buf);
extern int zs_aio_wait_completion(zs_aio_ctxt_t *zs_aio);
extern void zs_aio_reset(zs_aio_ctxt_t *zs_aio);

#endif
