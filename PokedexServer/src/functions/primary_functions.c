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


	//t_getAttr* getAttr =  malloc(sizeof(t_getAttr));
	file_attr* getAttr = malloc(sizeof(file_attr));
	getAttr->file_size=0;
	getAttr->state=0;
	int resultadoOsada = 0;
	log_info(logPokedexServer, "POKEDEXSERVER - Invoco la funcion osada_getattr con el path: %s", path);
	resultadoOsada = osada_getattr(path,getAttr);
	log_info(logPokedexServer, "POKEDEXSERVER - El resultado de osada_getattr es: %d", resultadoOsada);
	send(clientSocket,&resultadoOsada,sizeof(int),0);

	//En base al resultado de osada le mando al servidor el tipo del archivo
	if(resultadoOsada==1){
		//Enviar el tipo del archivo que reconocio osada (0 es deleted, 1 es regular, 2 es directorio)
		log_info(logPokedexServer,"PokedexServer: El tipo de archivo reconocido por osada_getattr es %d",getAttr->state);
		log_info(logPokedexServer,"PokedexServer: El file_size de archivo reconocido por osada_getattr es %d",getAttr->file_size);
		//Enviar el size del archivo reconocido (enviarBufferLleno que envia??)
		enviarBufferLleno(clientSocket, getAttr);
	}
	else{
		log_info(logPokedexServer,"PokedexServer: osada_getattr devolvio el error: %d",resultadoOsada);
	}

	free(getAttr);
	return resultadoOsada;
}
char* agruparContenidoDeDirectorio(t_list* directorio){
	int i;
	char* vectorDeContenido=string_new();
	for (i=0;i<list_size(directorio);i++){
		string_append(&vectorDeContenido,(char*)list_get(directorio,i));

		if(i!= (list_size(directorio)-1)){
			string_append(&vectorDeContenido,",");
		}

	}
	//log_info(logPokedexServer,"El contenido de %s es: %s",path, vectorDeContenido);
	return vectorDeContenido;
}
int proce_readdir(int clientSocket, char* path){
	int tamanio=0;
	int resultadoOsada = 0;

	char* contenido=string_new();
	t_list* directorios=list_create();

	resultadoOsada = osada_readdir(path, directorios);

	contenido = string_duplicate(agruparContenidoDeDirectorio(directorios));
	tamanio = string_length(contenido);

	send(clientSocket,&resultadoOsada,sizeof(int),0);

	if(resultadoOsada != -1){
		send(clientSocket,&tamanio,sizeof(int),0);
		log_info(logPokedexServer,"Envie %d elementos del path %s",tamanio,path);
		send(clientSocket,contenido,tamanio,0);
		log_info(logPokedexServer,"Envie los %d elementos correctamente del path %s",tamanio, path);
	}

	return resultadoOsada;
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

int proce_rename(int clientSocket, char* path){
	int tamanioNuevoPath;
	int resultadoOsada = 0;
	recibirTamanioDelPath(clientSocket,&tamanioNuevoPath);

	char* newPath=string_new();
	recibirPath(clientSocket,&newPath,tamanioNuevoPath);

	resultadoOsada = osada_rename(path, newPath);

	send(clientSocket,&resultadoOsada,sizeof(int),0);

	return resultadoOsada;
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

int proce_removeFile(int clientSocket, char* path){
	int resultadoOsada = 0;

	resultadoOsada = osada_removeFile(path);
	send(clientSocket,&resultadoOsada,sizeof(int),0);

	return resultadoOsada;
}

int proce_removeDir(int clientSocket, char* path){
	int resultadoOsada = 0;

	resultadoOsada = osada_removeDir(path);
	send(clientSocket,&resultadoOsada,sizeof(int),0);

	return resultadoOsada;
}

int proce_open(int clientSocket, char* path){
	int resultadoOsada;
	resultadoOsada = osada_open(path);
	send(clientSocket,&resultadoOsada,sizeof(int),0);


	return resultadoOsada;
}
