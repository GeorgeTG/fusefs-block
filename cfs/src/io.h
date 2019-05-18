#ifndef __CFS_IO__
#define __CFS_IO__
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

off_t file_seek(int fd, int offset, int whence);
ssize_t s_read(int fd, void *buf, size_t count);
ssize_t s_write(int fd, void *buf, size_t count);
void write_line(int fd, char *line, int count);

#endif