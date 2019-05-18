#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>

#include <fuse.h>
#include <openssl/sha.h>

#include "cfs.h"

long max_fds;
long file_descriptors;
struct cfs_file_state* files;

int init_cfs(char *rootdir, struct cfs_state *state) {

    /* get the maximum number of file descriptors the systems is configured to have */
    max_fds = sysconf(_SC_OPEN_MAX);
    if (max_fds < 0) {
        return -1;
    }

    state->root = strncat(CFS_ROOT, rootdir, PATH_MAX);

    /* check if the root directory exists, otherwise create it */
    struct stat st = {0};
    if (stat(state->root, &st) == -1) {
        mkdir(state->root, 0700);
    }

    return 0;
}

int read_block(int fd, long index, struct cfs_file_block* buffer) {

}

int remove_block_at(int fd, off_t offset) {

}

int remove_blocks_after(int fd, off_t offset) {

}

int delete_file(int fd) {

}

