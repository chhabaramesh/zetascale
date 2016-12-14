#include "zs_aio.h"


void
zs_aio_init(zs_aio_ctxt_t *zs_aio)
{
	int r = 0;
	int i = 0;
	zs_aio->io_count = 0;
	zs_aio->pending_ios = 0;
	memset(&zs_aio->zs_iocbs, 0, sizeof(zs_aio->zs_iocbs));
	for (i = 0; i < ZS_AIO_MAX_IOS; i++) {
		zs_aio->zs_iocbs[i].cb = (struct iocb *) malloc(sizeof(struct iocb));
		memset(zs_aio->zs_iocbs[i].cb, 0, sizeof(struct iocb));
	}
	io_setup(ZS_AIO_MAX_IOS, &zs_aio->ctxt);
	if (r != 0) {
		perror("io setup failed:");
		exit(-1);
	}
}

void
zs_aio_reset(zs_aio_ctxt_t *zs_aio)
{
	zs_aio->io_count = 0;
	zs_aio->pending_ios = 0;
}

int
zs_aio_submit(zs_aio_ctxt_t *zs_aio, int fd, off_t offset, size_t length, void *buf)
{
	int r = 0;
	struct iocb **cb = &zs_aio->zs_iocbs[zs_aio->io_count].cb;

	if (zs_aio->io_count == ZS_AIO_MAX_IOS) {
		perror("aio limit per transaction exhausted.");
		exit(-1);
  }

	io_prep_pwrite(*cb, fd, buf, length, offset);
	(*cb)->data = (void *) zs_aio->io_count;

	r = io_submit(zs_aio->ctxt, 1, cb);
	if (r != 1) {
		perror("io submit failed:");
		exit(-1);
	}

	zs_aio->io_count++;
	zs_aio->pending_ios++;
	return 0;
}

int
zs_aio_wait_completion(zs_aio_ctxt_t *zs_aio)
{
	int r = 0;
	int i = 0;
	int res = 0;
	struct timespec ts = {5, 0};
	struct iocb *cb = NULL;
	int idx = 0;

	r = io_getevents(zs_aio->ctxt, zs_aio->pending_ios,
			 zs_aio->pending_ios, &zs_aio->events[0], &ts);
	if (r != zs_aio->pending_ios) {
		perror("io wait for events timedout:");
		exit(-1);
	}

	for (i = 0; i < zs_aio->pending_ios; i++) {
		cb = (struct iocb *) zs_aio->events[i].obj;
		res = zs_aio->events[i].res;

		if (res <= 0) {
			perror("io failed :");
			exit(-1);
		}
		idx = (uint64_t) cb->data;

		if (zs_aio->zs_iocbs[idx].cb != cb) {
			perror("Corrupted something in aio");
			abort();	
		}
		zs_aio->zs_iocbs[idx].ret = res;
		zs_aio->zs_iocbs[idx].done = 1;
	}

	zs_aio->pending_ios = 0;
	zs_aio->io_count = 0;
	return 0;
}
