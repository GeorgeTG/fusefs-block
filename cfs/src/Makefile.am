bin_PROGRAMS = bbfs cfscat
bbfs_SOURCES = bbfs.c log.c log.h  params.h storage.c storage.h io.c io.h util.c util.h cfs.h cfs.c
cfscat_SOURCES = cfscat.c fakelog.c log.h  storage.c storage.h io.c io.h util.c util.h cfs.h cfs.c
AM_CFLAGS = @FUSE_CFLAGS@
LDADD = @FUSE_LIBS@ -lcrypto
