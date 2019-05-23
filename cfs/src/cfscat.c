#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "storage.h"
#include "cfs.h"
#include "log.h"
#include "util.h"
#include "io.h"

int main(int argc, char* argv[]) {
    char* root;
    char file[PATH_MAX];
    char hex_buf[HASH_LENGTH * 2 + 1];
    int fd;
    int i;
    cfs_state_t state;
    cfs_file_t* cfs_file;
    FILE* log;

    off_t index_buf;
    unsigned char hash_buf[HASH_LENGTH];

    root = realpath(argv[1], NULL);
    cfs_init(&state, root);
    log = log_open(); 

    combine(file, root, argv[2]);
    fd = open(file, O_RDONLY);
    if (fd<0) {
        perror("Cannot open file!");
    }
    cfs_register_file(&state, file, fd);

    cfs_file = cfs_get_file(&state, fd);
    printf("File size: %ld, total blocks: %d \n BLOCKS:\n", cfs_file->size, cfs_file->total_blocks);

    s_lseek(fd, BLOCK_START, SEEK_SET);
    for (i=0; i<cfs_file->total_blocks; i++) {
        s_read(fd, (void*)&index_buf, sizeof(off_t));
        s_read(fd, (void*)hash_buf, HASH_LENGTH);
        hexify(hash_buf, HASH_LENGTH, hex_buf, HASH_LENGTH*2 + 1);
        printf("\t%d -> %s\n", index_buf, hex_buf);
    }


    cfs_destroy(&state);
    fclose(log);
}