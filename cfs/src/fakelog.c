#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FILE *log_open() {
    return fopen("/dev/random", "r");
}

void log_msg(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    vfprintf(stdout, format, ap);
}

int log_error(char *func)
{
    int ret = -errno;
    
    log_msg("    ERROR %s: %s\n", func, strerror(errno));
    
    return ret;
}

void log_retstat(char *func, int retstat)
{
    int errsave = errno;
    log_msg("    %s returned %d\n", func, retstat);
    errno = errsave;
}


int log_syscall(char *func, int retstat, int min_ret)
{
    log_retstat(func, retstat);

    if (retstat < min_ret) {
	log_error(func);
	retstat = -errno;
    }

    return retstat;
}


