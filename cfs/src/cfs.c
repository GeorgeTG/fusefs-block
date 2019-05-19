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
#include "util.h"

int init_cfs(char *rootdir, cfs_state_t *state) {

    /* get the maximum number of file descriptors the systems is configured to have */
    state->max_fds = sysconf(_SC_OPEN_MAX);
    if (state->max_fds < 0) {
        return -1;
    }

    state->storage = (cfs_blk_store_t*)malloc(sizeof(cfs_blk_store_t));
    init_storage(state->storage, rootdir);

    return 0;
}

int destroy_cfs(cfs_state_t* state) {
    destroy_storage(state->storage);
}

int open_file()

int delete_file(int fd) {

}

