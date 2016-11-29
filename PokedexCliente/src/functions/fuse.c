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
	log_info(archivoLog,"PokedexCliente: Envio %s",function);
	char* sizePath=malloc(sizeof(char)*11);
	sprintf(sizePath,"%i",string_length((char*)path));
	send(pokedexServer, sizePath, 11, 0);
	send(pokedexServer, path, string_length((char*)path), 0);
	log_info(archivoLog,"PokedexCliente: La path es %s",path);
}


void recvBasicInfo(int* resultadoOsada, char* nombreFuncion, char* path){
	recv(pokedexServer,resultadoOsada, sizeof(int), 0);
	log_info(archivoLog, "La respuesta recibida (int) desde osada para %s con el path %s es: %d", nombreFuncion, path, *resultadoOsada);
}

void enviarNombreDeLaFuncion(char* nom){
	if (send(pokedexServer,nom,5*sizeof(char),0) >0){
	log_info(archivoLog,"FUSE: Envie %s al servidor",nom);
	}
	else{
		log_info(archivoLog, "FUSE: No se pudo enviar el nombre de la funcion");
	}
}

void enviarTamanioDelPath(const char* path){
	char* sizePath=malloc(sizeof(char)*11);
	sprintf(sizePath,"%i",string_length((char*)path));
	send(pokedexServer,sizePath,11,0);
	log_info(archivoLog,"FUSE: Envie %d al servidor",strlen(path));
}
void enviarPath(const char* path){
	send(pokedexServer,path,strlen(path),0);
	log_info(archivoLog,"FUSE: Envie %s al servidor",path);
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
	int* resultadoOsada = malloc(sizeof(int));
	sendBasicInfo("GETAT", path);
	recvBasicInfo(resultadoOsada, "GETAT", path);

	memset(stbuf, 0, sizeof(struct stat));

	if (*resultadoOsada==1){
		int tipoDeArchivo;
		recv(pokedexServer, &tipoDeArchivo,sizeof(int),0); //Recibo el tipo del archivo reconocido por osada
		log_info(archivoLog,"PokedexCliente: El tipo de archivo reconocido por osada_getattr para GETAT con el path %s es %d", path, tipoDeArchivo);

		//Si es un directorio
		if (tipoDeArchivo==2){
			recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
			stbuf->st_mode=S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			log_info(archivoLog,"PokedexCliente: El peso del archivo del path %s es %d", path, stbuf->st_size);
		}
		//Si es un archivo regular
		else if (tipoDeArchivo==1){
			recv(pokedexServer,&(stbuf->st_size),sizeof(stbuf->st_size),0);
			log_info(archivoLog,"PokedexCliente: El peso del archivo del path %s es %d", path, stbuf->st_size);
			stbuf->st_mode=S_IFREG | 0444;
			stbuf->st_nlink = 1;
		}
		//Si es un archivo DELETED (el tipo es 0)
		else{
			res=-ENOENT;
		}
	}

	free(resultadoOsada);
	return res;

}


int chamba_readdir(const char* path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

	sendBasicInfo("READD", path);
	return -ENOENT;

	/*(void) offset;
	(void) fi;

	char* mensaje = string_new();
	armarMensajeBasico("READD", (char*)path, &mensaje);
	string_append(&mensaje, ",");
	string_append(&mensaje, string_itoa(offset));


	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);


	return 0;*/
}

int chamba_open (const char * path, struct fuse_file_info * fi){

	int* resultadoOsada = malloc(sizeof(int));
	sendBasicInfo("OPENF", path);
	recvBasicInfo(resultadoOsada, "OPENF", (char*)path);


	return *resultadoOsada; //Desde el servidor ya me devuelve un 1 o un ENOENT
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
	return -ENOENT;

	/*char* mensaje = string_new();
	armarMensajeBasico("ULINK", (char*)path, &mensaje);

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
}

int chamba_rmdir (const char * path){

	sendBasicInfo("RMDIR", path);
	return -ENOENT;

	/*char* mensaje = string_new();
	armarMensajeBasico("RMDIR", (char*)path, &mensaje);

	char* respuesta = string_new();
	conectarConServidorYRecibirRespuesta(pokedexServer, mensaje, &respuesta);

	return 0;*/
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
