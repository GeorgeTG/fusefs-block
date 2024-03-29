/* Handles file operation for CFS
File structure

--------------------
** MAGIC **
off_t file_size
off_t total_blocks

off_t block_index - (20) hash  X total_blocks times
-----------------

*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <pthread.h>

#include <fuse.h>

#include "cfs.h"
#include "storage.h"
#include "io.h"
#include "util.h"
#include "log.h"


/*
    Initialise the CFS file system
*/
int cfs_init(cfs_state_t *state, const char* rootdir) {
    int i;

    pthread_mutex_init(&state->lock, NULL);

    /* get the maximum number of file descriptors the systems is configured to have */
    state->max_fds = sysconf(_SC_OPEN_MAX);
    if (state->max_fds < 0) {
        return -1;
    }

    state->storage = (cfs_blk_store_t*)malloc(sizeof(cfs_blk_store_t));
    init_storage(state->storage, rootdir);

    /* init file state map */
    state->files = malloc(sizeof(cfs_file_t) * FDS_STORE_INITIAL);
    state->fds = malloc(sizeof(int) * FDS_STORE_INITIAL);
    state->fds_cap = FDS_STORE_INITIAL;
    state->n_fds = 0;
    for (i=0; i<state->fds_cap; i++) {
        state->fds[i] = -1;
    }
    return 0;
}


/*
    Deinitialise the CFS file system.
*/
int cfs_destroy(cfs_state_t* state)
{
    pthread_mutex_destroy(&state->lock);
    free(state->files);
    free(state->fds);
    destroy_storage(state->storage);
}


/*
    Stat a CFS file.
    Path must contain root.
*/
int cfs_file_stat(cfs_state_t* state, const char* path, cfs_file_t* stat_buf)
{
    int fd, ret=0;
    char magic_buff[sizeof(MAGIC) + 1];
    ssize_t total;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    magic_buff[sizeof(MAGIC)] = '\0';
    total = s_read(fd, (void*)magic_buff, sizeof(MAGIC));
    if (total < sizeof(MAGIC) || (strcmp(magic_buff, MAGIC) != 0 )) {
        log_msg("\nCFS: file: %s is not a CFS file!\n", path);
        return -1;
    }
    ret |= s_read(fd, (void*)&(stat_buf->size), sizeof(off_t));
    ret |= s_read(fd, (void*)&(stat_buf->total_blocks), sizeof(off_t));
    if (ret > 0) {
        log_msg("\n CFS: File stat: %s, size: %d, blocks: %d\n", path, stat_buf->size, stat_buf->total_blocks);
        return ret;
    } else {
        log_error("\n CFS: Cant read file! \n");
        return -1;
    } 
}


int cfs_print_state(cfs_state_t* state) {
    int i = 0;
    log_msg("\nCFS STATE:\n");
    for (i=0; i<state->fds_cap; i++) {
        log_msg("[%d] -> %s\n", state->fds[i], state->files[i].path);
    }
    return -1;
}

/*
    Register a file to be manipulated with CFS.
    Equivalent to open without O_CREAT.
    Path must contain root
*/
int cfs_register_file(cfs_state_t* state, const char* path, const int fd) {
    int i = 0, ret;
    off_t buff;

    pthread_mutex_lock(&state->lock);

    if (state->n_fds >= state->fds_cap /2) {
        /* make the state map bigger*/
        i = state->fds_cap - 1; /* store old upper bound, to init the new memory */
        state->fds_cap = state->fds_cap * 2;
        state->files = realloc(state->files, sizeof(cfs_file_t) * state->fds_cap);
        state->fds = realloc(state->fds, sizeof(int) * state->fds_cap);
        for (; i<state->fds_cap; i++) {
            state->fds[i] = -1;
        }
    }

    /* find a free spot in the map */
    for (i=0; i<state->fds_cap; i++) {
        if (state->fds[i] == -1) {
            state->fds[i] = fd;
            state->files[i].offset = 0;
            state->files[i].fd = fd;
            strcpy(state->files[i].path, path);

            log_msg("\n CFS: registered file[FD: %d]: %s at index: %d \n", fd, path, i);
            state->n_fds++;

            /* read size and blocks */
            ret = s_lseek(fd, sizeof(MAGIC), SEEK_SET);
            ret |= s_read(fd, (void*)&(state->files[i].size), sizeof(off_t));
            ret |= s_read(fd, (void*)&(state->files[i].total_blocks), sizeof(off_t));
            if (ret < 0 ) {
                log_error("CFS: Register file");
                pthread_mutex_unlock(&state->lock);
                return ret;
            } else {
                log_msg("\n CFS: File [FD: %d]: %s is %lld bytes, %lld blocks \n",
                    fd, path,
                    state->files[i].size,
                    state->files[i].total_blocks);
    
                pthread_mutex_unlock(&state->lock);
                return fd;
            }
        }
    }

    pthread_mutex_unlock(&state->lock);
    return -1; 
}


/*
    Release a CFS file
*/
int cfs_release_file(cfs_state_t* state, const int fd) {
    int i;
    
    pthread_mutex_lock(&state->lock);
    for (i=0; i<state->fds_cap; i++) {
        if (state->fds[i] == fd) {
            log_msg("\n CFS: Released file %d at [%d] -> *%p\n", fd, i, &state->files[i]);
            state->fds[i] = -1;
            state->n_fds--;
            pthread_mutex_unlock(&state->lock);
            return 0;
        }
    }

    pthread_mutex_unlock(&state->lock);
    return -1;
}


/*
    Create a file and initialise its header.
    Equivalent to open with O_CREAT.
    Path must contain root.
*/
int cfs_create_file(cfs_state_t* state, const char* path, mode_t mode)
{
    int fd;
    off_t buff = 0;

    fd = log_syscall("cfs: open", open(path, O_CREAT | O_EXCL | O_WRONLY, mode), 0);
    if (fd >= 0) {
        s_write(fd, (void*)MAGIC, sizeof(MAGIC));
        // We have 0 file size and 0 total blocks
        s_write(fd, (void*)&buff, sizeof(off_t));
        s_write(fd, (void*)&buff, sizeof(off_t));
        log_msg("\n CFS: created file: %s\n", path);
    } else if (fd == -1 && errno == EEXIST) {
        log_msg("\n File exists: %s \n", path);
    } else {
        log_error("cfs_create_file");
    }
    
    return fd;
}


/*
    Get a CFS file by the underlying file descriptor
*/
cfs_file_t* cfs_get_file(cfs_state_t* state, int fd)
{
    int i;
    for (i=0; i<state->fds_cap; i++) {
        if (state->fds[i] == fd) {
            log_msg("\n CFS: Found file %d at [%d] -> *%p\n", fd, i, &state->files[i]);
            return &state->files[i];
        }
    }

    return NULL;
}


/*
    Helper function to find a block index in file, offset is set exactly after index
    and before the block hash
*/
int cfs_file_find_index(const cfs_state_t* state, cfs_file_t* file, const off_t index)
{
    int ret;
    unsigned char hash [HASH_LENGTH];
    ssize_t block_size;
    ssize_t bytes_read;
    off_t index_buff;

    if (file->total_blocks == 0) {
        return 0;
    }

    // start from the beginning
    s_lseek(file->fd, BLOCK_START, SEEK_SET);

    // find the index-hash pair in the file
    do {
        bytes_read = s_read(file->fd, (void*)&index_buff, sizeof(off_t));
        if (bytes_read == 0) {
            return 0;
        }else if (index_buff == index) {
            return 1;
        } else {
            // skip this hash
            s_lseek(file->fd, HASH_LENGTH, SEEK_CUR);
        }
    } while(1);

    return 0;
}


/*
    Register a *block* to *file*.
    Block is saved in block storage, if it doesn't already exist. 
*/
int cfs_file_register_block(cfs_state_t* state, cfs_file_t* file, const cfs_block_t* block)
{
    int ret;
    unsigned char hash [HASH_LENGTH];
    ssize_t old_size=0, bytes_read;

    calculate_hash(block->data, block->size, hash);
   
    pthread_mutex_lock(&state->lock);
   
    // try to store the block, 
    ret = store_block(state->storage, block->data, block->size, hash);
    if (ret < 0) {
        pthread_mutex_unlock(&state->lock);
        log_error("CFS: Cant store block!");
        pthread_mutex_unlock(&state->lock);
        return ret;
    }

    // check if we have a different block at this index
    ret = cfs_file_find_index(state, file, block->index);
    if (ret) {
        log_msg("CFS: other block found at index: %lld\n", block->index);
        /* block at this index already exists, find the old block's size*/
        ret = s_read(file->fd, (void*)hash, HASH_LENGTH);
        old_size = block_get_size(state->storage, hash);

        // replace the block hash at this index 
        s_lseek(file->fd, -HASH_LENGTH, SEEK_CUR); 
        s_write(file->fd, (void*)hash, HASH_LENGTH);
    } else {
        // append pair to the end of the file
        log_msg("CFS: registering new block [%d] for file %s", block->index, file->path);
        s_lseek(file->fd, 0, SEEK_END);
        s_write(file->fd, (void*)&(block->index), sizeof(block->index));
        s_write(file->fd, (void*)hash, HASH_LENGTH);
        file->total_blocks ++;
    }

    // update file size
    file->size = file->size - old_size + block->size;
    s_lseek(file->fd, SIZE_START, SEEK_SET);
    s_write(file->fd, (void*)&(file->size), sizeof(file->size));
    s_write(file->fd, (void*)&(file->total_blocks), sizeof(file->total_blocks));

    pthread_mutex_unlock(&state->lock);

    return 0;
}


/*
    Read a block from file
    Index is the block index, not to be confused with file offset
*/
int cfs_file_read_block(cfs_state_t* state, cfs_file_t* file, const off_t index, cfs_block_t* buff)
{
    int ret;
    unsigned char hash [HASH_LENGTH];
    ssize_t block_size;
    ssize_t bytes_read;
    off_t index_buff;

    pthread_mutex_lock(&state->lock);
    // start from the beginning
    s_lseek(file->fd, BLOCK_START, SEEK_SET);

    // find the index-hash pair in the file
    ret = cfs_file_find_index(state, file, index);
    if (!ret) {
        pthread_mutex_unlock(&state->lock);
        return 0;
    }
    bytes_read = s_read(file->fd, (void*)hash, HASH_LENGTH);


    ret = load_block(state->storage, hash, buff->data, &buff->size);
    if (ret != 0) {
        log_error("CFS: Cant read block!");
        pthread_mutex_unlock(&state->lock);
        return ret;
    }

    buff->index = index;
    pthread_mutex_unlock(&state->lock);
    return 1;
}
