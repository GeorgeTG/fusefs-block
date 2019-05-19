/* contains functions to handle block storage to disk */
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/sha.h>

#include "storage.h"
#include "io.h"
#include "log.h"

#define MAGIC "st0r4g3v0.1"
#define BLOCKS_DIRECTORY "/.BLOCKS"



#if defined(WIN32)
#  define DIR_SEPARATOR '\\'
#else
#  define DIR_SEPARATOR '/'
#endif

void combine(char *destination, const char *path1, const char *path2) {
    /* Combines paths */
    if (path1 && *path1) {
        size_t len = strlen(path1);
        strcpy(destination, path1);

        if (destination[len - 1] == DIR_SEPARATOR) {
            if (path2 && *path2) {
                strcpy(destination + len, (*path2 == DIR_SEPARATOR) ? (path2 + 1) : path2);
            }
        }
        else {
            if (path2 && *path2) {
                if (*path2 == DIR_SEPARATOR) {
                    strcpy(destination + len, path2);
                } else {
                    destination[len] = DIR_SEPARATOR;
                    strcpy(destination + len + 1, path2);
                }
            }
        }
    } else if (path2 && *path2) {
        strcpy(destination, path2);
    } else {
        destination[0] = '\0';
    }
}

int hexify(const unsigned char *in, const size_t in_size, char *out, const size_t out_size) {
    if (in_size == 0 || out_size == 0) return 0;

    char map[16+1] = "0123456789abcdef";

    int bytes_written = 0;
    size_t i = 0;
    while(i < in_size && (i*2 + (2+1)) <= out_size)
    {
        uint8_t high_nibble = (in[i] & 0xF0) >> 4;
        *out = map[high_nibble];
        out++;

        uint8_t low_nibble = in[i] & 0x0F;
        *out = map[low_nibble];
        out++;

        i++;

        bytes_written += 2;
    }
    *out = '\0';

    return bytes_written;
}

int store_block(const cfs_blk_store_t* storage, const unsigned char* data, const size_t size, char* hash) {
    int fd, ret;
    char path[storage->block_fname_size];
    unsigned char buff[SHA_DIGEST_LENGTH];

    SHA1(data, size, buff);
    hexify(buff, SHA_DIGEST_LENGTH, hash, SHA_DIGEST_LENGTH * 2);

    combine(path, storage->blocks_path, hash);
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

int load_block(const cfs_blk_store_t* storage, const char* hash, unsigned char* data, size_t* size) {
    int fd;
    ssize_t ret;
    char path[storage->block_fname_size];

    combine(path, storage->blocks_path, hash);
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
