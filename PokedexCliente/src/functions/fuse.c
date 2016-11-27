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

void sendBasicInfo(char* function, const char* path){
	send(pokedexServer, function, 5,0);
	log_info(archivoLog,"FUSE - Envio %s",function);
	char* sizePath=malloc(sizeof(char)*11);
	sprintf(sizePath,"%i",string_length((char*)path));
	send(pokedexServer, sizePath, 11, 0);
	send(pokedexServer, path, string_length((char*)path), 0);
	log_info(archivoLog,"FUSE - La path es %s",path);
}


void recvBasicInfo(int* resultadoOsada){
	recv(pokedexServer,resultadoOsada, sizeof(int), 0);
	log_info(archivoLog, "La respuesta recibida (int) desde osada es: ", *resultadoOsada);
}

void enviarBuffer(struct stat* stbuf){
	send(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
	log_info(archivoLog,"FUSE: Envie el primer parametro de stbuf");
}
void recibirBufferCompleto (struct stat* stbuf){
	recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
	log_info(archivoLog,"FUSE: Recibo el primer parametro de stbuf");
}

int chamba_getattr(char* path, struct stat* stbuf){
	int res = 0;
	int resultadoOsada;
	sendBasicInfo("GETAT", path);
//	recvBasicInfo(&resultadoOsada);
//
//	memset(stbuf, 0, sizeof(struct stat));
//
//	if (resultadoOsada==1){
//		recv(pokedexServer, &(stbuf->st_nlink),sizeof(stbuf->st_nlink),0); //Recibo el tipo del archivo reconocido por osada
//		log_info(archivoLog,"PokedexCliente: El tipo de archivo reconocido por osada_getattr es %d",stbuf->st_nlink);
//
//		//Si es un directorio
//		if ((stbuf->st_nlink)==2){
//			recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
//			log_info(archivoLog,"PokedexCliente: El peso del archivo (directorio) es %d",stbuf->st_size);
//			stbuf->st_mode=S_IFDIR | 0755;
//			stbuf->st_nlink = 2;
//		}
//		//Si es un archivo regular
//		else if (stbuf->st_nlink==1){
//			recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
//			log_info(archivoLog,"PokedexCliente: El peso del archivo (regular) es %d",stbuf->st_size);
//			stbuf->st_mode=S_IFREG | 0444;
//			stbuf->st_nlink = 1;
//		}
//		//Si es un archivo DELETED (el tipo es 0)
//		else{
//			res=-ENOENT;
//		}
//	}
//
//	free(resultadoOsada);
	return res;

}


int chamba_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

	sendBasicInfo("READD", path);
	return 0;

	/*(void) offset;
	(void) fi;

	return 0;*/
}

int chamba_open (const char * path, struct fuse_file_info * fi){
	sendBasicInfo("OPENF", path);

	int resultadoOsada;
//	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;

}

int chamba_read (const char * path, char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	sendBasicInfo("READF", path);
	return -ENOENT;

	/*char* mensaje = string_new();
	armarMensajeBasico("READF", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, buffer);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(size));
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
}

int chamba_create (const char * path, mode_t mode, struct fuse_file_info * fi){

	sendBasicInfo("CREAT", path);
	return -ENOENT;

	/*char* mensaje = string_new();
	armarMensajeBasico("CREAT", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(mode));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
}

int chamba_truncate (const char * path, off_t offset){

	sendBasicInfo("TRUNC", path);
	return -ENOENT;

	/*char* mensaje = string_new();
	armarMensajeBasico("TRUNC", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
}

int chamba_mkdir (const char * path, mode_t modo){
	sendBasicInfo("MKDIR", path);
	return -ENOENT;
	/*char* mensaje = string_new();
	armarMensajeBasico("MKDIR", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(modo));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
}

int chamba_rename (const char * path, const char * newPath){

	sendBasicInfo("RENAM", path);
	return -ENOENT;

	/*char* mensaje = string_new();
	armarMensajeBasico("RENAM", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, (char*)newPath);

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
}

int chamba_unlink (const char * path){

	sendBasicInfo("ULINK", path);
	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;
}

int chamba_rmdir (const char * path){

	sendBasicInfo("RMDIR", path);
	int resultadoOsada;
	recvBasicInfo(&resultadoOsada);

	return resultadoOsada;
}

int chamba_write (const char * path, const char * buffer, size_t size, off_t offset, struct fuse_file_info * fi){

	sendBasicInfo("WRITE", path);
	return -ENOENT;

	/*char* mensaje = string_new();
	armarMensajeBasico("WRITE", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, (char*)buffer);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(size));
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
}

int chamba_statfs (const char * path, struct statvfs * stats){

	sendBasicInfo("STATF", path);
	return -ENOENT;

	char* mensaje = string_new();
	armarMensajeBasico("STATF", (char*)path, &mensaje);
	//faltaria agregarle la estructura de stats?

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;
}



/*int main(int argc, char *argv[]){
        umask(0);
        return fuse_main(argc, argv, &chamba_oper, NULL);
}*/
