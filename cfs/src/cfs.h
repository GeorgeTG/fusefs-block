#ifndef __CFS_H__
#define __CFS_H__

#define CFS_ROOT ".cfs"
#define CFS_DIRECTORY "cfs_dir"

#include <linux/limits.h>

struct cfs_state {
    char *root;
    char *directory;
};

struct cfs_file_state {
    char path[PATH_MAX];
    off_t offset;
    size_t size;
    size_t n_blocks;
    int* blocks;
    int fd;
};

struct cfs_file_block {
    long index;
    char data[4096];
    size_t size;
};


#endif