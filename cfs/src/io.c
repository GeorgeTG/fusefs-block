#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


int file_exists(const char* path) {
    return access(path, F_OK) != -1;
}

off_t s_lseek(int fd, off_t offset, int whence){
    int code;
    do {
        code = lseek(fd, offset, whence);
        if( code == -1 ){
            if (errno == EINTR) {
                continue;
            }
            perror("lseek");
        }
        return code;
    } while (1);

    return code;
}

ssize_t s_read(int fd, void *buf, size_t count){
    ssize_t bytes_read = 0;
    ssize_t bytes_left = count;

    do {
        bytes_read = read(fd, buf, bytes_left);
        if (bytes_read == -1){
            if (errno == EINTR) {
                continue;
            } else {
                perror("read");
                return -1;
            }
        } else {
            bytes_left -= bytes_read;
            buf += bytes_read;
        }
     } while(bytes_left > 0 && bytes_read != 0);

    return count-bytes_left;
}

ssize_t s_write(int fd, void *buf, size_t count){
    ssize_t bytes_written;
    ssize_t bytes_left = count;

    do {
        bytes_written = write(fd, buf, bytes_left);
        if( bytes_written == -1 ){
            if (errno == EINTR) {
                continue;
            } else {
                perror("read");
                return -1;
            }
        } else{
            bytes_left -= bytes_written;
            buf += bytes_written;
        }
    } while( bytes_left > 0 );

    return bytes_written;
}


