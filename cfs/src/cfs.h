#ifndef __CFS_H__
#define __CFS_H__

#include <linux/limits.h>
#include <sys/stat.h>

#include "storage.h"

#define FDS_STORE_INITIAL 20

#define MAGIC "CFS0.1"
#define SIZE_START sizeof(MAGIC)
#define BLOCK_START sizeof(off_t) * 2 + sizeof(MAGIC)

typedef struct {
    char path[PATH_MAX];
    off_t offset;
    off_t size;
    off_t total_blocks;
    int fd;
} cfs_file_state_t;

typedef struct {
    long index;
    char data[4096];
    size_t size;
} cfs_file_block_t ;

typedef struct {
    char *root;
    long max_fds;
    cfs_blk_store_t* storage;

    /* file state */
    cfs_file_state_t* files;
    int* fds;
    size_t n_fds;
    size_t fds_cap; /* current size of dynamic array */
    /* ----------- */
} cfs_state_t;

int cfs_init(cfs_state_t *state, const char* rootdir);
int cfs_destroy(cfs_state_t* state);
cfs_file_state_t* cfs_get_file_state(cfs_state_t* state, int fd);
int cfs_create_file(cfs_state_t* state, const char* path, mode_t mode);

#endif