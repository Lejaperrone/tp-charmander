/*
 * primary_functions.c
 *
 *  Created on: 26/11/2016
 *      Author: utnso
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include "../socketLib.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../osada/osada.h"
#include <pthread.h>
#include <stdint.h>
#include <pthread.h>
#include "../commons/structures.h"
#include "../commons/definitions.h"
#include "auxiliar_functions.h"

int proce_getattr(int clientSocket, char* path){

	t_getAttr* getAttr =  malloc(sizeof(t_getAttr));
	int resultadoOsada = osada_getattr(path,(file_attr*)getAttr);
	enviarBufferLleno(clientSocket, getAttr);

	return resultadoOsada;
}

int proce_readdir(int clientSocket){
	char* path=string_new();

	recibirParametrosDeReadDir(clientSocket,path);
	t_list* directorios=list_create();
	return osada_readdir(path, directorios);
}

int proce_readfile(int clientSocket){
	char* path=string_new();
	char* buffer = string_new();
	recv(clientSocket,buffer,sizeof(buffer),0);
	t_readFile* readFile;
	recv(clientSocket,&(readFile->size),sizeof(readFile->size),0);
	recv(clientSocket,&(readFile->offset),sizeof(readFile->offset),0);
	return osada_read(path, buffer, readFile->size, readFile->offset);
}

int proce_create(int clientSocket){
	char* path = string_new();
	t_createFile* createFile;
	recv(clientSocket,&(createFile->modo),sizeof(createFile->modo),0);
	return osada_createFile(path, createFile->modo);
}

int proce_truncate(int clientSocket){
	char* path = string_new();
	t_truncateFile* truncateFile;
	recv(clientSocket,&(truncateFile->offset),sizeof(truncateFile->offset),0);
	return osada_truncate(path, truncateFile->offset);
}

int proce_mkdir(int clientSocket){
	char* path = string_new();
	t_makeDir* makeDir;
	recv(clientSocket,&(makeDir->mode),sizeof(makeDir->mode),0);
	return osada_createDir(path, obtenerNombreDelDirectorio(path), makeDir->mode);
}

int proce_rename(int clientSocket){
	char* path = string_new();
	char* nombreNuevo=string_new();
	recv(clientSocket, nombreNuevo,sizeof(nombreNuevo),0);
	return osada_rename(path, nombreNuevo);
}

int proce_write(int clientSocket){
	char* path = string_new();
	char* buffer = string_new();
	recv(clientSocket,buffer,sizeof(buffer),0);
	t_write* swrite;
	recv(clientSocket,&(swrite->size),sizeof(swrite->size),0);
	recv(clientSocket,&(swrite->offset),sizeof(swrite->size),0);
	return osada_write(path, buffer, swrite->size, swrite->offset);
}

int proce_statfs(int clientSocket){
	char* path = string_new();
	t_statfs* statfs;
	recv(clientSocket,&(statfs->__f_spare),sizeof(statfs->__f_spare),0);
	recv(clientSocket,&(statfs->f_bavail),sizeof(statfs->f_bavail),0);
	recv(clientSocket,&(statfs->f_bfree),sizeof(statfs->f_bfree),0);
	recv(clientSocket,&(statfs->f_blocks),sizeof(statfs->f_blocks),0);
	recv(clientSocket,&(statfs->f_bsize),sizeof(statfs->f_bsize),0);
	recv(clientSocket,&(statfs->f_favail),sizeof(statfs->f_favail),0);
	recv(clientSocket,&(statfs->f_ffree),sizeof(statfs->f_ffree),0);
	recv(clientSocket,&(statfs->f_files),sizeof(statfs->f_files),0);
	recv(clientSocket,&(statfs->f_flag),sizeof(statfs->f_flag),0);
	recv(clientSocket,&(statfs->f_frsize),sizeof(statfs->f_frsize),0);
	recv(clientSocket,&(statfs->f_fsid),sizeof(statfs->f_fsid),0);
	recv(clientSocket,&(statfs->f_namemax),sizeof(statfs->f_namemax),0);
	return osada_statfs(path,statfs);
}

int proce_removeFile(int clientSocket){
	char* path = string_new();
	return osada_removeFile(path);
}

int proce_removeDir(int clientSocket){
	char* path = string_new();
	return osada_removeDir(path);
}

int proce_open(int clientSocket){
	char* path = string_new();
	return osada_open(path);
}

