#ifndef __CFS_H__
#define __CFS_H__

#include <linux/limits.h>
#include <sys/stat.h>
#include <openssl/sha.h>

#include "storage.h"

#define FDS_STORE_INITIAL 20

#define MAGIC "CFS0.1"
#define SIZE_START sizeof(MAGIC)
#define TOTAL_BLOCKS_START SIZE_START + sizeof(off_t)
#define BLOCK_START sizeof(off_t) * 2 + sizeof(MAGIC)
#define BLOCK_PAIR sizeof(off_t) + SHA_DIGEST_LENGTH

typedef struct {
    char path[PATH_MAX];
    off_t offset;
    off_t size;
    off_t total_blocks;
    int fd;
} cfs_file_t;

typedef struct {
    off_t index;
    size_t size;
    char data[4096];
} cfs_block_t ;

typedef struct {
    char *root;
    long max_fds;
    cfs_blk_store_t* storage;

    /* file state */
    cfs_file_t* files;
    int* fds;
    size_t n_fds;
    size_t fds_cap; /* current size of dynamic array */
    /* ----------- */
} cfs_state_t;

int cfs_init(cfs_state_t *state, const char* rootdir);
int cfs_destroy(cfs_state_t* state);
cfs_file_t* cfs_get_file(cfs_state_t* state, int fd);
int cfs_file_stat(cfs_state_t* state, const char* path, cfs_file_t* stat_buf);
int cfs_create_file(cfs_state_t* state, const char* path, mode_t mode);
int cfs_register_file(cfs_state_t* state, const char* path, const int fd);
int cfs_file_read_block(const cfs_state_t* state, cfs_file_t* file, const off_t index, cfs_block_t* buff);
int cfs_file_register_block(const cfs_state_t* state, cfs_file_t* file, const cfs_block_t* block);

#endif