bin_PROGRAMS = bbfs
bbfs_SOURCES = bbfs.c log.c log.h  params.h storage.c storage.h io.c io.h util.c util.h cfs.h cfs.c
AM_CFLAGS = @FUSE_CFLAGS@
LDADD = @FUSE_LIBS@ -lcrypto
