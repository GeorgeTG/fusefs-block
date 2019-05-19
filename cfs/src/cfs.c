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

#include <fuse.h>

#include "cfs.h"
#include "storage.h"
#include "io.h"
#include "util.h"
#include "log.h"


int cfs_init(cfs_state_t *state, const char* rootdir) {
    int i;
    /* get the maximum number of file descriptors the systems is configured to have */
    state->max_fds = sysconf(_SC_OPEN_MAX);
    if (state->max_fds < 0) {
        return -1;
    }

    state->storage = (cfs_blk_store_t*)malloc(sizeof(cfs_blk_store_t));
    init_storage(state->storage, rootdir);

    /* init file state map */
    state->files = malloc(sizeof(cfs_file_state_t) * FDS_STORE_INITIAL);
    state->fds = malloc(sizeof(int) * FDS_STORE_INITIAL);
    state->fds_cap = FDS_STORE_INITIAL;
    state->n_fds = 0;
    for (i=0; i<state->fds_cap; i++) {
        state->fds[i] = -1;
    }
    return 0;
}

int cfs_destroy(cfs_state_t* state) {
    destroy_storage(state->storage);
}

int cfs_register_file(cfs_state_t* state, const char* path, const int fd, const int flags) {
    int i = 0;

    if (state->n_fds >= state->fds_cap /2) {
        /* make the state map bigger*/
        i = state->fds_cap; /* store old upper bound, to init the new memory */
        state->fds_cap = state->fds_cap * 2;
        state->files = realloc(state->files, sizeof(cfs_file_state_t) * state->fds_cap);
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
            strcpy(state->files[i].path, path);
        }
    }
    return -1; 
}

int cfs_create_file(cfs_state_t* state, const char* path, mode_t mode) {
    /* note path contains root */
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
    Get a file state by file descriptor
*/
cfs_file_state_t* cfs_get_file_state(cfs_state_t* state, int fd) {
    int i;
    for (i=0; i<state->fds_cap; i++) {
        if (state->fds[i] == fd) {
            return &state->files[i];
        }
    }

    return NULL;
}

int cfs_file_register_block(cfs_state_t* state, cfs_file_state_t* file) {

}


int delete_file(int fd) {

}

