#ifndef __CFS_UTIL__
#define __CFS_UTIL__

#if defined(WIN32)
#  define DIR_SEPARATOR '\\'
#else
#  define DIR_SEPARATOR '/'
#endif

#include <openssl/sha.h>

#define HASH_LENGTH SHA_DIGEST_LENGTH

void combine(char *destination, const char *path1, const char *path2);
int hexify(const unsigned char *in, const size_t in_size, char *out, const size_t out_size);
int calculate_hash(const char* data, const size_t length, unsigned char* buff);

#endif