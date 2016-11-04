/*
 * fuse.c
 *
 *  Created on: 1/11/2016
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
#include <tad_items.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stddef.h>
#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include "../commons/structures.h"

void armarMensajeBasico(char* nombreFuncion, char* path, char** mensaje){
	string_append(mensaje, nombreFuncion);
	string_append(mensaje, path);
}

int chamba_getattr (const char* path, struct stat* stbuf, struct fuse_file_info *fi){
	int res = 0;
	char* mensaje = string_new();
	armarMensajeBasico("GETAT", (char*)path, &mensaje);


	if(send(pokedexServer, &mensaje, sizeof(mensaje), 0)){
		char* resp;
		recv(pokedexServer, &resp, 1024, 0);
	}
	memset(stbuf, 0, sizeof(struct stat));

	//Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, "Default file path") == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen("Default file name");
	} else {
		res = -ENOENT;
	}
	return 0;
}


int chamba_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;

	char* mensaje = string_new();
	armarMensajeBasico("READD", (char*)path, &mensaje);
	string_append(&mensaje, string_itoa(offset));


	if(send(pokedexServer, &mensaje, sizeof(mensaje), 0)){
		char* resp;
		recv(pokedexServer, &resp, 1024, 0);
	}

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, "Default File Name", NULL, 0);

	return 0;
}

int chamba_open (const char * path, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("OPENF", (char*)path, &mensaje);

	return 0;
}

int chamba_read (const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("READF", (char*)path, &mensaje);
	string_append(&mensaje, string_itoa(size));
	string_append(&mensaje, string_itoa(offset));
	//falta agregarle el buffer?

	return 0;
}

int chamba_create (const char * path, mode_t mode, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("CREAT", (char*)path, &mensaje);
	string_append(&mensaje, string_itoa(mode));

	return 0;
}

int chamba_truncate (const char * path, off_t offset){

	char* mensaje = string_new();
	armarMensajeBasico("TRUNC", (char*)path, &mensaje);
	string_append(&mensaje, string_itoa(offset));

	return 0;
}

int chamba_mkdir (const char * path, mode_t modo){

	char* mensaje = string_new();
	armarMensajeBasico("MKDIR", (char*)path, &mensaje);

	return 0;
}

int chamba_rename (const char * path, const char * newPath){

	char* mensaje = string_new();
	armarMensajeBasico("RENAM", (char*)path, &mensaje);
	string_append(&mensaje, (char*)newPath);

	return 0;
}

int chamba_unlink (const char * path){

	char* mensaje = string_new();
	armarMensajeBasico("ULINK", (char*)path, &mensaje);

	return 0;
}

int chamba_rmdir (const char * path){

	char* mensaje = string_new();
	armarMensajeBasico("RMDIR", (char*)path, &mensaje);

	return 0;
}

int chamba_write (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("WRITE", (char*)path, &mensaje);
	string_append(&mensaje, string_itoa(offset));
	//falta agregarle mas cosas al mensaje que vamos a mandar

	return 0;
}

int chamba_statfs (const char * path, struct statvfs * stats){

	char* mensaje = string_new();
	armarMensajeBasico("STATF", (char*)path, &mensaje);
	//faltaria agregarle la estructura de stats?

	return 0;
}

int chamba_release (const char * path, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("RLEAS", (char*)path, &mensaje);

	return 0;
}

int chamba_fallocate (const char * path, int amount, off_t sizeh, off_t sizef,  struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("FALOC", (char*)path, &mensaje);
	string_append(&mensaje, string_itoa(amount));
	string_append(&mensaje, string_itoa(sizeh));
	string_append(&mensaje, string_itoa(sizef));

	return 0;
}



/*int main(int argc, char *argv[]){
        umask(0);
        return fuse_main(argc, argv, &chamba_oper, NULL);
}*/
