/*
 * fuse.c
 *
 *  Created on: 1/11/2016
 *      Author: utnso
 */
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

int chamba_getattr (const char* path, struct stat* stbuf, struct fuse_file_info *fi){
	//TODO
	return 0;
}

static int chamba_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	//TODO
	return 0;
}

int chamba_open (const char * path, struct fuse_file_info * fi){
	//TODO
	return 0;
}

int chamba_read (const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){
	//TODO
	return 0;
}

int chamba_create (const char * path, mode_t mode, struct fuse_file_info * fi){
	//TODO
	return 0;
}

int chamba_truncate (const char * path, off_t offset){
	//TODO
	return 0;
}

int chamba_mkdir (const char * path, mode_t modo){
	//TODO
	return 0;
}

int chamba_rename (const char * path, const char * newPath){
	//TODO
	return 0;
}

int chamba_unlink (const char * path){
	//TODO
	return 0;
}

int chamba_rmdir (const char * path){
	//TODO
	return 0;
}

int chamba_write (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){
	//TODO
	return 0;
}

int chamba_statfs (const char * path, struct statvfs * stats){
	//TODO
	return 0;
}

int chamba_release (const char * path, struct fuse_file_info * fi){
	//TODO
	return 0;
}

int chamba_fallocate (const char * path, int amount, off_t sizeh, off_t sizef,  struct fuse_file_info * fi){
	//TODO
	return 0;
}

/*int main(int argc, char *argv[]){
        umask(0);
        return fuse_main(argc, argv, &chamba_oper, NULL);
}*/
