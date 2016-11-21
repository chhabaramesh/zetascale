/*
 * Utility functions for WS
 *
 * Author: Ramesh Chander
 */

#ifndef __WS_UTILS_H_
#define __WS_UTILS_H_

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h> 
//#include "platform/assert.h"
//#include <assert.h>
#include <stdio.h>


#define BYTES_IN_MB (1024 * 1024)
#define BYTES_IN_KB (1024)

#define USECSC_IN_SEC 1000000

#define WS_INVALID_BLOCK 0 //This is invalid physical block number 


extern int __ws_log_enabled;
extern FILE * __ws_log_fd;
#define ws_dbg_printf(fmt, args...) do {		\
			if (__ws_log_enabled) {		\
				fprintf(__ws_log_fd, fmt, args);\
			}				\
			} while (0)			\



//#define plat_assert(a) assert(a)


#define atomic_get(var)  __sync_fetch_and_add(&var, 0)

#define atomic_fetch_and_add(var, val)  __sync_fetch_and_add(&var, val)

//#define atomic_set(var, num)  __sync_fetch_and_add(&var, num)
//#define atomic_set(var, num)  __atomic_store_n(&var, num, 0)
#define atomic_set(var, num) (var = num)

#define atomic_incr(var) __sync_fetch_and_add(&var, 1)

#define atomic_decr(var)  __sync_fetch_and_sub(&var, 1)

//#define atomic_sub(var, val)  __sync_fetch_and_sub(&var, val)

#define atomic_test_and_set(var, num) __sync_lock_test_and_set(&var, num)

uint64_t get_time_usecs(void);

void *
ws_alloc(size_t bytes);

void
ws_free(void *p);

uint64_t
get_property(char *prop, uint64_t def_val);

void
ws_create_thread(pthread_t *pt, const pthread_attr_t *attr,
		void *(*start_routine) (void *), void *arg, char *tag);

#endif // end of file
