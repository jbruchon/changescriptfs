/*
 * Change Scripting FUSE Filesystem
 *
 * Copyright (C) 2017 Jody Bruchon <jody@jodybruchon.com>
 *
 * Creates a shell script to reflect changes performed which can
 * be used to replicate those changes on other systems or volumes.
 *
 * Licensed under GNU GPL v2. See LICENSE and README for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>
#include "jody_hash.h"
#include "jody_string.h"
#include "changescriptfs.h"

/* Avoid str[n]cmp calls by doing this simple check directly */
#define PATH_IS_ROOT(a) (a[0] == '/' && a[1] == '\0')


/*** Non-FUSE helper functions ***/


/*** End helper functions ***/


/*** FUSE functions ***/

static int changescriptfs_access(const char * const restrict path, int mode)
{
	return -ENOENT;
}


static int changescriptfs_flush(const char *path, struct fuse_file_info *fi)
{
	return 0;
}


static int changescriptfs_fsync(const char *unused1, int unused2, struct fuse_file_info *fi)
{
	return 0;
}


static int changescriptfs_getattr(const char * const restrict path,
		struct stat * const restrict stbuf)
{
	return -ENOENT;
}


static int changescriptfs_opendir(const char * const restrict path, struct fuse_file_info *fi)
{
	DIR *dp;

	errno = 0;
	dp = opendir(path);

	if (!dp) return errno;
	fi->fh = (uintptr_t)dp;
	return 0;
}


static int changescriptfs_readdir(const char * const restrict path,
		void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	dp = (DIR *)fi->fh;
	return 0;
}


static int changescriptfs_open(const char * const restrict path,
		struct fuse_file_info *fi)
{
	return -ENOENT;
}


static int changescriptfs_read(const char * const restrict path,
		char * const restrict buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{
	return 0;
}


static int changescriptfs_write(const char * const restrict path,
		const char * const restrict buf,
		size_t size, off_t offset, struct fuse_file_info *fi)
{
	return size;
}


static int changescriptfs_mknod(const char * const restrict path,
		mode_t mode, dev_t dev)
{
	return 0;
}


static int changescriptfs_unlink(const char * const restrict path)
{
	return 0;
}


static int changescriptfs_mkdir(const char * const restrict path,
		mode_t mode)
{
	return 0;
}


static int changescriptfs_rmdir(const char * const restrict path)
{
	return 0;
}


static int changescriptfs_utimens(const char * const restrict path,
		const struct timespec tv[2])
{
	return 0;
}


static int changescriptfs_truncate(const char * const restrict path,
		off_t len)
{
	return 0;
}

/* FUSE debugging placeholders for when things get "fun" */
/*
static int changescriptfs_readlink(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: readlink\n"); return -1; }
static int changescriptfs_symlink(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: symlink\n"); return -1; }
static int changescriptfs_rename(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: rename\n"); return -1; }
static int changescriptfs_link(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: link\n"); return -1; }
static int changescriptfs_chmod(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: chmod\n"); return -1; }
static int changescriptfs_chown(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: chown\n"); return -1; }
static int changescriptfs_statfs(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: statfs\n"); return -1; }
static int changescriptfs_release(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: release\n"); return -1; }
static int changescriptfs_releasedir(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: releasedir\n"); return -1; }
static int changescriptfs_fsyncdir(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: fsyncdir\n"); return -1; }
static int changescriptfs_ftruncate(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: ftruncate\n"); return -1; }
static int changescriptfs_fgetattr(void)
{ LOAD_WD(); LOG("ERROR: Not implemented: fgetattr\n"); return -1; }
*/


/* Required for FUSE to use these functions */
static struct fuse_operations changescriptfs_oper = {
	.getattr	= changescriptfs_getattr,
/*	.readlink	= changescriptfs_readlink, */
	.mknod		= changescriptfs_mknod,
	.opendir	= changescriptfs_opendir,
	.readdir	= changescriptfs_readdir,
	.mkdir		= changescriptfs_mkdir,
	.unlink		= changescriptfs_unlink,
	.rmdir		= changescriptfs_rmdir,
/*	.symlink	= changescriptfs_symlink,
	.rename		= changescriptfs_rename,
	.link		= changescriptfs_link,
	.chmod		= changescriptfs_chmod,
	.chown		= changescriptfs_chown, */
	.truncate	= changescriptfs_truncate,
	.open		= changescriptfs_open,
	.read		= changescriptfs_read,
	.write		= changescriptfs_write,
	.access		= changescriptfs_access,
	.utimens	= changescriptfs_utimens,
/*	.statfs		= changescriptfs_statfs, */
	.fsync		= changescriptfs_fsync,
	.flush		= changescriptfs_flush,
/*	.release	= changescriptfs_release,
	.releasedir	= changescriptfs_releasedir,
	.fsyncdir	= changescriptfs_fsyncdir,
	.ftruncate	= changescriptfs_ftruncate,
	.fgetattr	= changescriptfs_fgetattr,
	.lock		= changescriptfs_lock, */
};


int main(int argc, char *argv[])
{
	struct changescriptfs_data * restrict wd;
	char file[4096];
	FILE *script;
	int i;

	/* Show version and return successfully if requested */
	if (argc == 2 && !strcaseeq(argv[1], "-v")) {
		fprintf(stderr, "Change Scripting Filesystem %s (%s)\n", VER, VERDATE);
		return EXIT_SUCCESS;
	}

	if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-')) {
		fprintf(stderr, "Change Scripting Filesystem %s (%s)\n", VER, VERDATE);
		fprintf(stderr, "\nUsage: %s [-o ro] [fuse_options] directory mountpoint\n\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Get file name from command line and pass to FUSE */
	xstrcpy(file, argv[argc-2]);

	wd = (struct changescriptfs_data *) malloc(sizeof(struct changescriptfs_data));
	if (!wd) goto oom;

	wd->script = fopen(file, "wb"); /* FIXME */

	if (!script) {
		fprintf(stderr, "Error: couldn't open %s\n", file);
		return EXIT_FAILURE;
	}

	i = fuse_main(argc, argv, &changescriptfs_oper, wd);

	fclose(wd->script);
	free(wd);
	return i;
oom:
	fprintf(stderr, "Error: out of memory\n");
	return EXIT_FAILURE;
}
