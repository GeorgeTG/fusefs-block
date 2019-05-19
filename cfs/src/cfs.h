#ifndef __CFS_H__
#define __CFS_H__

#include <linux/limits.h>

#include "storage.h"

typedef struct {
    char path[PATH_MAX];
    off_t offset;
    size_t size;
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
    cfs_file_state_t* files;
    long* fds;
    size_t n_fds;
} cfs_state_t;


#endif