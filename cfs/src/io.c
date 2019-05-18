#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

off_t file_seek(int fd, int offset, int whence){
    int code = lseek(fd, ((off_t)offset), whence);
    if( code == -1 ){
        perror("lseek");
        return -1;
    }
    return code;
}

ssize_t s_read(int fd, void *buf, size_t count){
    ssize_t bytes_read;
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
            return -1;
        } else {
            bytes_left -= bytes_read;
            buf += bytes_read;
        }
     } while(bytes_left > 0);

    return bytes_read;
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

void write_line(int fd, char *line, int count){
    fileseek(fd, 0, SEEK_SET);
    swrite(fd, line, count);
}