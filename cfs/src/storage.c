/*
	This part of the file system is responsible for storing and loading blocks
	based on hashes.

	No info about files is handled here.

	Format:
	------------------
	size_t ref_counter
	BLOCK_SIZE data
 */

#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "storage.h"
#include "io.h"
#include "util.h"
#include "log.h"

#define REF_START 0
#define REF_SIZE sizeof(size_t)
#define DATA_START REF_START + REF_SIZE


size_t block_get_refs( const cfs_blk_store_t* storage, const unsigned char* hash) {
	int fd;
	ssize_t ret;
	char path[storage->block_fname_size];
	char buff[HASH_LENGTH * 2 + 1];

	// convert hash to readable hex string
	hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);

	// create fullpath and open block file
	combine(path, storage->blocks_path, buff);
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		log_error("Cannot open block");
		return -1;
	}

	struct flock fl;
	memset(&fl, 0, sizeof(fl));

	// lock refs region
	fl.l_type = F_RDLCK;
	fl.l_whence = SEEK_SET; 
	fl.l_start = REF_START;
	fl.l_len = REF_SIZE;

	// F_SETLKW specifies blocking mode
	if (fcntl(fd, F_SETLKW, &fl) == -1) {
		exit(1);
	}
}


int block_dec_ref( const cfs_blk_store_t* storage, const unsigned char* hash) {
	int fd;
	ssize_t ret;
	size_t refs;
	char path[storage->block_fname_size];
	char buff[HASH_LENGTH * 2 + 1];

	// convert hash to readable hex string
	hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);

	// create fullpath and open block file
	combine(path, storage->blocks_path, buff);
	fd = open(path, O_RDWR);
	if (fd == -1) {
		log_error("Cannot open block");
		return -1;
	}

	struct flock fl;
	memset(&fl, 0, sizeof(fl));

	// lock refs region
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET; 
	fl.l_start = REF_START;
	fl.l_len = REF_SIZE;

	if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
		// TODO: EINTR
		close(fd);
		return -1;
	}

	// read refs
	s_lseek(fd, REF_START, SEEK_SET);
	if ( s_read(fd, (void*)(&refs), REF_SIZE) != REF_SIZE)  {
		close(fd);
		return -1;
	}

	// decrease and write refs or delete the file
	refs--;
	if (refs == 0 ) {
		unlink(path);
	} else {
		s_lseek(fd, REF_START, SEEK_SET);
		if ( s_write(fd, (void*)(&refs), REF_SIZE) != REF_SIZE) {
			close(fd);
			return -1;
		}
	}

	fl.l_type = F_UNLCK;
	if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
		// TODO: EINTR
		return -1;
	}

	close(fd);
	return refs;
}


int block_inc_ref( const cfs_blk_store_t* storage, const unsigned char* hash) {
	int fd;
	ssize_t ret;
	size_t refs;
	char path[storage->block_fname_size];
	char buff[HASH_LENGTH * 2 + 1];

	// convert hash to readable hex string
	hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);

	// create fullpath and open block file
	combine(path, storage->blocks_path, buff);
	fd = open(path, O_RDWR);
	if (fd == -1) {
		log_error("Cannot open block");
		return -1;
	}

	struct flock fl;
	memset(&fl, 0, sizeof(fl));

	// lock refs region
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET; 
	fl.l_start = REF_START;
	fl.l_len = REF_SIZE;

	if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
		// TODO: EINTR
		close(fd);
		return -1;
	}

	// read refs
	s_lseek(fd, REF_START, SEEK_SET);
	if ( s_read(fd, (void*)(&refs), REF_SIZE) != REF_SIZE) {
		close(fd);
		return -1;
	}

	// increase and write refs
	refs++;
	s_lseek(fd, 0, SEEK_SET);
	if ( s_write(fd, (void*)(&refs), REF_SIZE) != REF_SIZE) {
		close(fd);
		return -1;
	}

	fl.l_type = F_UNLCK;
	if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
		// TODO: EINTR
		close(fd);
		return -1;
	}

	close(fd);
	return refs;
}


ssize_t block_get_size( const cfs_blk_store_t* storage, const unsigned char* hash) {
	int fd;
	ssize_t ret;
	char path[storage->block_fname_size];
	char buff[HASH_LENGTH * 2 + 1];

	// convert hash to readable hex string
	hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);

	// create fullpath and open block file
	combine(path, storage->blocks_path, buff);
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		log_error("Cannot open block");
		return -1;
	}

	struct flock fl;
	memset(&fl, 0, sizeof(fl));

	// lock the whole file
	fl.l_type = F_RDLCK;
	fl.l_whence = SEEK_SET; 
	fl.l_start = 0;
	fl.l_len = 0;

	if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
		// TODO: EINTR
		log_error("Cannot lock block");
		close(fd);
		return -1;
	}

	s_lseek(fd, DATA_START, SEEK_SET);
	ret = s_lseek(fd, 0, SEEK_END);

	fl.l_type = F_ULOCK;
	if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
		// TODO: EINTR
		close(fd);
		log_error("Cannot unlock block");
		return -1;
	}

	close(fd);
	return ret;
}

int block_exists( const cfs_blk_store_t* storage, const unsigned char* hash) {
	char path[storage->block_fname_size];
	char buff[HASH_LENGTH * 2 + 1];
	
	// convert hash to readable hex string
	hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);
	combine(path, storage->blocks_path, buff);

	return file_exists(path);
}

int store_block(const cfs_blk_store_t* storage, const unsigned char* data, const size_t size, unsigned char* hash) {
	int fd, ret;
	char path[storage->block_fname_size];
	char buff[HASH_LENGTH * 2 + 1];
	const size_t refs = 1;

	// convert hash to readable hex string
	hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);

	// Create the file path and save the block
	combine(path, storage->blocks_path, buff);
	if (!file_exists(path)) {
		fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if (fd == -1) {
			log_error("Cannot write block");
			return -1;
		}

		struct flock fl;
		memset(&fl, 0, sizeof(fl));
		
		// lock the whole file
		fl.l_type = F_WRLCK;
		fl.l_whence = SEEK_SET; 
		fl.l_start = 0;
		fl.l_len = 0;

		if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
			// TODO: EINTR
			log_error("Cannot lock block");
			close(fd);
			return -1;
		}

		ret = s_write(fd,  (void*)(&refs), REF_SIZE);
		if (ret != REF_SIZE) {
			log_error("Cannot write refs");
			close(fd);
			return -1;
		}

		ret = s_write(fd, (void*)data, size * sizeof(*data));
		if (ret != size) {
			log_error("Cannot write data");
			close(fd);
			return -1;
		}

		fl.l_type = F_ULOCK;
		if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
			// TODO: EINTR
			log_error("Cannot lock block");
			close(fd);
			return -1;
		}

		close(fd);
		return ret;
	}
	
	return 0;
}

int load_block(const cfs_blk_store_t* storage, const unsigned char* hash, unsigned char* data, size_t* size, size_t* refs) {
	int fd;
	ssize_t ret;
	char path[storage->block_fname_size];
	char buff[HASH_LENGTH * 2 + 1];

	// convert hash to readable hex string
	hexify(hash, HASH_LENGTH, buff, HASH_LENGTH * 2);

	combine(path, storage->blocks_path, buff);
	if (file_exists(path)) {
		fd = open(path, O_RDONLY);
		if (fd == -1) {
			log_error("Cannot read block");
			return -1;
		}
		struct flock fl;
		memset(&fl, 0, sizeof(fl));

		// lock the whole file
		fl.l_type = F_RDLCK;
		fl.l_whence = SEEK_SET; 
		fl.l_start = 0;
		fl.l_len = 0;

		if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
			// TODO: EINTR
			log_error("Cannot lock block");
			close(fd);
			return -1;
		}

		s_lseek(fd, 0, SEEK_SET);
		ret = s_read(fd, refs, REF_SIZE);
		if (ret <= 0) {
			log_error("Cannot read block");
			close(fd);
			return -1;
		}

		s_lseek(fd, DATA_START, SEEK_SET);
		ret = s_read(fd, data, BLOCK_SIZE);
		if (ret <= 0) {
			log_error("Cannot read block");
			close(fd);
			return -1;
		}

		*size = (size_t)ret;
		log_msg("\n CFS: Storage: Loaded block %s, size: %ld, refs: %u\n", buff, size, *refs);

		fl.l_type = F_UNLCK;
		if (fcntl(fd, F_OFD_SETLKW, &fl) == -1) {
			// TODO: EINTR
			log_error("Cannot unlock block");
			close(fd);
			return -1;
		}

		close(fd);
	} else {
		log_msg("\n CFS: BLOCK NOT FOUND %s\n", buff);
		return -EEXIST;
	}
	
	return 0;
}

int init_storage(cfs_blk_store_t* storage, const char* root) {
	size_t root_len = strlen(root);

	// Calculate the filename size for all blocks
	storage->block_fname_size = root_len + 1 + sizeof(BLOCKS_DIRECTORY) +  1 + SHA_DIGEST_LENGTH * 2 + 2;

	storage->blocks_path = malloc(root_len + sizeof(BLOCKS_DIRECTORY) + 1);
	storage->root_path = malloc(root_len + 1);
	if (storage->blocks_path == NULL || storage->root_path == NULL) {
		perror("Storage: alloc paths");
		return -1;
	}

	/* store paths */
	strcpy(storage->root_path, root);
	combine(storage->blocks_path, root, BLOCKS_DIRECTORY);

	/* check if the blocks directory exists, otherwise create it */
	struct stat st = {0};
	if (stat(storage->blocks_path, &st) == -1) {
		mkdir(storage->blocks_path, 0700);
	}

	return 1;
}

void destroy_storage(cfs_blk_store_t* storage) {
	free(storage->blocks_path);
	free(storage->root_path);
}
