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
	string_append(mensaje, ",");
	string_append(mensaje, path);
}

void conectarConServidorYRecibirRespuesta(int pokedexServer, char* mensaje, char** respuesta){
	if(send(pokedexServer, &mensaje, sizeof(mensaje), 0)){
		recv(pokedexServer, respuesta, 1024, 0);
	}
}

int chamba_getattr(const char* path, struct stat* stbuf, struct fuse_file_info *fi){

	char* mensaje = string_new();
	armarMensajeBasico("GETAT", (char*)path, &mensaje);

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);


	memset(stbuf, 0, sizeof(struct stat));


	return 0;
}


int chamba_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;

	char* mensaje = string_new();
	armarMensajeBasico("READD", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));


	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);


	return 0;
}

int chamba_open (const char * path, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("OPENF", (char*)path, &mensaje);

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_read (const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("READF", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, buffer);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(size));
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_create (const char * path, mode_t mode, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("CREAT", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(mode));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_truncate (const char * path, off_t offset){

	char* mensaje = string_new();
	armarMensajeBasico("TRUNC", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_mkdir (const char * path, mode_t modo){

	char* mensaje = string_new();
	armarMensajeBasico("MKDIR", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(modo));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_rename (const char * path, const char * newPath){

	char* mensaje = string_new();
	armarMensajeBasico("RENAM", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, (char*)newPath);

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_unlink (const char * path){

	char* mensaje = string_new();
	armarMensajeBasico("ULINK", (char*)path, &mensaje);

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_rmdir (const char * path){

	char* mensaje = string_new();
	armarMensajeBasico("RMDIR", (char*)path, &mensaje);

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_write (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("WRITE", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, (char*)buffer);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(size));
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_statfs (const char * path, struct statvfs * stats){

	char* mensaje = string_new();
	armarMensajeBasico("STATF", (char*)path, &mensaje);
	//faltaria agregarle la estructura de stats?

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_release (const char * path, struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("RLEAS", (char*)path, &mensaje);

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}

int chamba_fallocate (const char * path, int amount, off_t sizeh, off_t sizef,  struct fuse_file_info * fi){

	char* mensaje = string_new();
	armarMensajeBasico("FALOC", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(amount));
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(sizeh));
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(sizef));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}



/*int main(int argc, char *argv[]){
        umask(0);
        return fuse_main(argc, argv, &chamba_oper, NULL);
}*/
