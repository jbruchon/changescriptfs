/*
 * Change Scripting FUSE Filesystem
 *
 * Copyright (C) 2017 by Jody Bruchon <jody@jodybruchon.com>
 *
 * Licensed under GNU GPL v2. See LICENSE and README for details.
 *
 */

#ifndef CHANGESCRIPTFS_H
#define CHANGESCRIPTFS_H

#include "version.h"

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "jody_hash.h"

struct changescriptfs_data {
	FILE *script;
	pthread_mutex_t *lock;
};

/* Shortcut to pull changescriptfs_data structure into a function
   This MUST BE PLACED between variable declarations and code in ANY
   function that uses changescriptfs logging or data */
#define LOAD_WD() struct changescriptfs_data *wd; \
		  wd = fuse_get_context()->private_data;

/* Threaded mode mutex */
#if ENABLE_THREADED
#define LOCK() pthread_mutex_lock(wd->lock)
#define UNLOCK() pthread_mutex_unlock(wd->lock)
#else
#define LOCK()
#define UNLOCK()
#endif

#endif /* CHANGESCRIPTFS_H */
