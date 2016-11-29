/*
 * osada.h
 *
 *  Created on: 20/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include "commons/declarations.h"
#include <signal.h>

#ifndef OSADA_OSADA_H_
#define OSADA_OSADA_H_

int osada_init(char* path);
int osada_readdir(char* path, t_list* directorios);
int osada_getattr(char* path, file_attr* attrs);
int osada_read(char *path, char* buf, size_t size, off_t offset);
int osada_write(char* path,char* buf, size_t size, off_t offset);
int osada_open(char* path);
int osada_removeDir(char* path);
int osada_removeFile(char* path);
int osada_createFile(char* path, mode_t mode);
int osada_createDir(char* path, char* name, mode_t mode);
int osada_rename(char* path, char* nuevaPath);
int hayBloquesDesocupadosEnElBitmap (int n);
int osada_truncate(char* path, off_t offset);
int osada_fallocate(const char* path, int amount, off_t sizef, off_t sizeh);
int osada_statfs(const char* path, struct statvfs* stats);


#endif /* OSADA_OSADA_H_ */
