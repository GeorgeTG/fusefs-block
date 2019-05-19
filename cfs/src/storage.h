#ifndef __CFS_STORAGE__
#define __CFS_STORAGE__

#include <openssl/sha.h>

#define HASH_LENGTH SHA_DIGEST_LENGTH * 2 + 1

typedef struct {
    char* root_path;
    char* blocks_path;
    size_t block_fname_size;
} cfs_blk_store_t;


int init_storage(cfs_blk_store_t* storage, const char* root);
void destroy_storage(cfs_blk_store_t* storage);
int store_block(const cfs_blk_store_t* storage, const unsigned char* data, const size_t size, char* hash);
int load_block(const cfs_blk_store_t* storage, const char* hash, unsigned char* data, size_t* size);
#endif