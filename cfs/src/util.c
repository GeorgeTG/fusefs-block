#include <string.h>

#include "util.h"

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


int calculate_hash(const char* data, const size_t length, unsigned char* buff) {
    SHA1(data, length, buff);
    return 1;
}

int hexify(const unsigned char *in, const size_t in_size, char *out, const size_t out_size) {
    /* convert a byte buffer to a hex string */
    if (in_size == 0 || out_size == 0) return 0;

    char map[16+1] = "0123456789abcdef";

    int bytes_written = 0;
    size_t i = 0;
    while(i < in_size && (i*2 + (2+1)) <= out_size)
    {
        unsigned char high_nibble = (in[i] & 0xF0) >> 4;
        *out = map[high_nibble];
        out++;

        unsigned char low_nibble = in[i] & 0x0F;
        *out = map[low_nibble];
        out++;

        i++;

        bytes_written += 2;
    }
    *out = '\0';

    return bytes_written;
}

