#ifndef __CFS_STORAGE__
#define __CFS_STORAGE__

#include <sys/types.h>

typedef struct {
    char* root_path;
    char* blocks_path;
    size_t block_fname_size;
} cfs_blk_store_t;


#define BLOCKS_DIRECTORY "/.BLOCKS"
#define BLOCK_SIZE 4096

int init_storage(cfs_blk_store_t* storage, const char* root);
void destroy_storage(cfs_blk_store_t* storage);
int store_block(const cfs_blk_store_t* storage, const unsigned char* data, const size_t size, unsigned char* hash);
int load_block(const cfs_blk_store_t* storage, const unsigned char* hash, unsigned char* data, size_t* size);
ssize_t block_get_size( const cfs_blk_store_t* storage, const unsigned char* hash);
#endif