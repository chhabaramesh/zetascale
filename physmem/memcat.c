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

/**
 * File:   memcat.c
 *
 * Author: drew
 *
 * Created on July 29, 2008
 *
 * (c) Copyright 2008, Schooner Information Technology, Inc.
 * http://www.schoonerinfotech.com/
 *
 * $Id: memcat.c 2477 2008-07-29 17:17:40Z drew $
 */

/**
 * Copy mmapped device to/from a file
 */

#include <sys/mman.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum direction {
    DIRECTION_FROM_MEM,
    DIRECTION_TO_MEM,
    DIRECTION_UNKNOWN
};

enum ret {
    RET_SUCCESS = 0,
    RET_FAILURE = 1,
    RET_USAGE = 2
};

static void usage();

int
main(int argc, char **argv) {
    int fd = -1;
    enum ret ret = RET_SUCCESS;
    ssize_t status;
    unsigned long size = 0;
    char *file = NULL;
    char *equals;
    void *ptr = NULL;
    enum direction direction = DIRECTION_UNKNOWN;
    char *ioptr, *ioend;
    int i;
    size_t memskip = 0;

    for (i = 1; ret == RET_SUCCESS && i < argc; ++i) {
        equals = strchr(argv[i], '=');
        if (!equals) {
            fprintf(stderr, "bad option format: %s\n", argv[i]);
            ret = RET_USAGE;
        } else if (strncmp(argv[i], "if", equals - argv[i]) == 0) {
            if (direction == DIRECTION_FROM_MEM) {
                ret = RET_USAGE;
            } else {
                direction = DIRECTION_FROM_MEM;
                file = equals + 1;
            }
        } else if (strncmp(argv[i], "of", equals - argv[i]) == 0) {
            if (direction == DIRECTION_FROM_MEM) {
                ret = RET_USAGE;
            } else {
                direction = DIRECTION_TO_MEM;
                file = equals + 1;
            }
        /* XXX: required, should probably probably offer ioctl option */
        } else if (strncmp(argv[i], "size", equals - argv[i]) == 0) {
            errno = 0;
            size = strtoul(equals + 1, NULL, 0);
            if (errno) {
                ret = RET_USAGE;
            }
        /* Not quite dd semantics - always skip/seek this far in memory */
        } else if (strncmp(argv[i], "memskip", equals - argv[i]) == 0) {
            errno = 0;
            memskip = strtoul(equals + 1, NULL, 0);
            if (errno) {
                ret = RET_USAGE;
            }
        } else {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            ret = RET_USAGE;
        }
    }

    if (ret == RET_SUCCESS && (!file || direction == DIRECTION_UNKNOWN ||
                               size <= 0)) {
        ret = RET_USAGE;
    }

    if (!ret) {
        fd = open(file, direction == DIRECTION_FROM_MEM ? O_RDONLY : 
#ifdef notyet
                  O_WRONLY
#else
                  O_RDWR
#endif
                  );
        if (fd == -1) {
            perror("open");
            ret = RET_FAILURE;
        }
    }

    if (!ret) {
        ptr = mmap(NULL, size, direction == DIRECTION_FROM_MEM ? 
                   PROT_READ : 
#ifdef notyet
                   PROT_WRITE
#else
                   PROT_READ|PROT_WRITE
#endif
                   , MAP_SHARED, fd, memskip /* offset */);
        if (ptr == MAP_FAILED) {
            perror("mmap");
            ret = RET_FAILURE;
        }
    }

    if (!ret) {
        ioptr = ptr;
        ioend = ioptr + size;
        do {
            if (direction == DIRECTION_FROM_MEM) {
                status = write(1, ioptr, ioend - ioptr);
            } else {
                status = read(0, ioptr, ioend - ioptr);
            }
            if (status > 0) {
                ioptr += status;
            } else if (status == -1) {
                perror("io failed");
            }
        } while (status > 0 && ioptr < ioend);
    }

    if (ptr) {
        status = munmap(ptr, size);
        if (status) {
            perror("munmap");
            if (!ret) {
                ret = RET_FAILURE;
            }
        }
    }

    if (fd != -1) {
        status = close(fd);
        if (status) {
            perror("close");
            if (!ret) {
                ret = RET_FAILURE;
            }
        }
    }

    if (ret == 2) {
        usage();
    }

    return (ret);
}

static void
usage() {
        fprintf (stderr, "usage memcat if=file|of=file size=bytes"
                 " [memskip=bytes]\n");
}
