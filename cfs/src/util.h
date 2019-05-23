#ifndef __CFS_UTIL__
#define __CFS_UTIL__

#if defined(WIN32)
#  define DIR_SEPARATOR '\\'
#else
#  define DIR_SEPARATOR '/'
#endif

#include <openssl/sha.h>

#define HASH_LENGTH SHA_DIGEST_LENGTH

 #define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })




void combine(char *destination, const char *path1, const char *path2);
int hexify(const unsigned char *in, const size_t in_size, char *out, const size_t out_size);
int calculate_hash(const char* data, const size_t length, unsigned char* buff);

#endif