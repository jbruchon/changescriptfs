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

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "jody_hash.h"
#include "version.h"

struct changescriptfs_data {
	FILE *script;		/* Shell script output file handle */
	char mountpoint[4096];	/* Path where the filesystem is mounted */
	int_fast16_t mountpoint_len;	/* Cached value of strlen() of above */
	pthread_mutex_t *lock;
};

/* Shortcut to pull changescriptfs_data structure into a function
   This MUST BE PLACED between variable declarations and code in ANY
   function that uses the changescriptfs_data structure */
#define LOAD_WD() struct changescriptfs_data *wd; \
		  wd = fuse_get_context()->private_data;

#define LOCK() pthread_mutex_lock(wd->lock)
#define UNLOCK() pthread_mutex_unlock(wd->lock)

#endif /* CHANGESCRIPTFS_H */
