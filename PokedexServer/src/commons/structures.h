/*
 * structures.h
 *
 *  Created on: 26/11/2016
 *      Author: utnso
 */

#ifndef COMMONS_STRUCTURES_H_
#define COMMONS_STRUCTURES_H_

struct hilo{
	int id;
	int socket;
}typedef t_hilo;



struct readFile{
	__SIZE_TYPE__ size;
	__off_t  offset;
}typedef t_readFile;

struct createFile{
	__mode_t modo;
}typedef t_createFile;

struct truncateFile{
	off_t offset;
}typedef t_truncateFile;

struct makeDir{
	mode_t mode;
}typedef t_makeDir;

struct write{
	size_t size;
	off_t offset;
}typedef t_write;

struct statvfsNuestra{
	    unsigned long int f_bsize;
	    unsigned long int f_frsize;
	#ifndef __USE_FILE_OFFSET64
	    __fsblkcnt_t f_blocks;
	    __fsblkcnt_t f_bfree;
	    __fsblkcnt_t f_bavail;
	    __fsfilcnt_t f_files;
	    __fsfilcnt_t f_ffree;
	    __fsfilcnt_t f_favail;
	#else
	    __fsblkcnt64_t f_blocks;
	    __fsblkcnt64_t f_bfree;
	    __fsblkcnt64_t f_bavail;
	    __fsfilcnt64_t f_files;
	    __fsfilcnt64_t f_ffree;
	    __fsfilcnt64_t f_favail;
	#endif
	    unsigned long int f_fsid;
	#ifdef _STATVFSBUF_F_UNUSED
	    int __f_unused;
	#endif
	    unsigned long int f_flag;
	    unsigned long int f_namemax;
	    int __f_spare[6];
}typedef t_statfs;

struct falloc{
	int amoun;
	off_t sizeh;
	off_t sizef;
}typedef t_falloc;

#endif /* COMMONS_STRUCTURES_H_ */
