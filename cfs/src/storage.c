/*
    This part of the file system is responsible for storing and loading blocks
    based on hashes.

    No info about files is handled here.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "storage.h"
#include "io.h"
#include "util.h"
#include "log.h"

#define BLOCKS_DIRECTORY "/.BLOCKS"

ssize_t block_get_size( const cfs_blk_store_t* storage, const unsigned char* hash) {
    int fd;
    ssize_t ret;
    char path[storage->block_fname_size];
    char buff[HASH_LENGTH * 2 + 1];

    // convert hash to readable hex string
    hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);

    // create fullpath and open block file
    combine(path, storage->blocks_path, buff);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        log_error("Cannot open block");
        return -1;
    }

    s_lseek(fd, 0, SEEK_SET);
    ret = s_lseek(fd, 0, SEEK_END);
    close(fd);

    return ret;
}

int block_exists( const cfs_blk_store_t* storage, const unsigned char* hash) {
    char path[storage->block_fname_size];
    char buff[HASH_LENGTH * 2 + 1];
    
    // convert hash to readable hex string
    hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);
    combine(path, storage->blocks_path, buff);

    return file_exists(path);
}

int store_block(const cfs_blk_store_t* storage, const unsigned char* data, const size_t size, unsigned char* hash) {
    int fd, ret;
    char path[storage->block_fname_size];
    char buff[HASH_LENGTH * 2 + 1];

    // convert hash to readable hex string
    hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);

    // Create the file path and save the block
    combine(path, storage->blocks_path, buff);
    if (!file_exists(path)) {
        fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            log_error("Cannot write block");
            return -1;
        }

        ret = s_write(fd, (void*)data, size * sizeof(*data));
        if (ret != size) {
            close(fd);
            return -1;
        }

        close(fd);
        return ret;
    }
    
    return 0;
}

int load_block(const cfs_blk_store_t* storage, const unsigned char* hash, unsigned char* data, size_t* size) {
    int fd;
    ssize_t ret;
    char path[storage->block_fname_size];
    char buff[HASH_LENGTH * 2 + 1];

    // convert hash to readable hex string
    hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);

    combine(path, storage->blocks_path, buff);
    if (file_exists(path)) {
        fd = open(path, O_RDONLY);
        if (fd == -1) {
            log_error("Cannot read block");
            return -1;
        }

        ret = s_read(fd, data, 4096);
        if (ret <= 0) {
            close(fd);
            return -1;
        }
        
        *size = (size_t)ret;
        close(fd);
    } else {
        log_error("BLOCK NOT FOUND!");
    }
    
    return 0;
}

int init_storage(cfs_blk_store_t* storage, const char* root) {
    size_t root_len = strlen(root);

    // Calculate the filename size for all blocks
    storage->block_fname_size = root_len + 1 + sizeof(BLOCKS_DIRECTORY) +  1 + SHA_DIGEST_LENGTH * 2 + 2;

    storage->blocks_path = malloc(root_len + sizeof(BLOCKS_DIRECTORY) + 1);
    storage->root_path = malloc(root_len + 1);
    if (storage->blocks_path == NULL || storage->root_path == NULL) {
        perror("Storage: alloc paths");
        return -1;
    }

    /* store paths */
    strcpy(storage->root_path, root);
    combine(storage->blocks_path, root, BLOCKS_DIRECTORY);

    printf("\nBlock directory is: %s\n", storage->blocks_path);

    /* check if the blocks directory exists, otherwise create it */
    struct stat st = {0};
    if (stat(storage->blocks_path, &st) == -1) {
        mkdir(storage->blocks_path, 0700);
        printf("\n Creating blocks directory \n");
    } else {
        printf("\n Found blocks directory \n");
    }

    return 1;
}

void destroy_storage(cfs_blk_store_t* storage) {
    free(storage->blocks_path);
    free(storage->root_path);
}
