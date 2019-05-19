#ifndef __CFS_STORAGE__
#define __CFS_STORAGE__

#include <openssl/sha.h>

#define HASH_LENGTH SHA_DIGEST_LENGTH * 2 + 1

typedef struct {
    char* root_path;
    char* blocks_path;
    size_t block_fname_size;
} block_storage;


int init_storage(block_storage* storage, const char* root);
int store_block(const block_storage* storage, const unsigned char* data, const size_t size, char* hash);
int load_block(const block_storage* storage, const char* hash, unsigned char* data, size_t* size);
#endif