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

int proce_readfile(int clientSocket, char* path){
	int resultadoOsada = 0;
	int tamanio = 0;
	char* buf = string_new();
	t_readFile* readFile = malloc(sizeof(t_readFile));

	recv(clientSocket,&(readFile->size),sizeof(size_t),0);
	recv(clientSocket,&(readFile->offset),sizeof(off_t),0);

	resultadoOsada = osada_read(path, buf, readFile->size, readFile->offset);

	send(clientSocket,&resultadoOsada,sizeof(int),0);

	tamanio = string_length(buf);

	if(resultadoOsada == 1){
		send(clientSocket,&tamanio,sizeof(int),0);
		log_info(logPokedexServer,"Envie %d elementos del path %s",tamanio,path);
		send(clientSocket,buf,tamanio,0);
		log_info(logPokedexServer,"Envie los %d elementos correctamente del path %s",tamanio, path);
	}

	free(readFile);
	return resultadoOsada;
}

int proce_create(int clientSocket, char* path){
	int resultadoOsada = 0;
	t_createFile* createFile = malloc(sizeof(t_createFile));

	recv(clientSocket,&(createFile->modo),sizeof(mode_t),0);
	resultadoOsada = osada_createFile(path, createFile->modo);

	send(clientSocket,&resultadoOsada,sizeof(int),0);


	free(createFile);
	return resultadoOsada;
}

int proce_truncate(int clientSocket, char* path){
	int resultadoOsada = 0;
	t_truncateFile* truncateFile = malloc(sizeof(t_truncateFile));

	recv(clientSocket,&(truncateFile->offset),sizeof(off_t),0);
	resultadoOsada = osada_truncate(path, truncateFile->offset);

	send(clientSocket,&resultadoOsada,sizeof(int),0);


	free(truncateFile);
	return resultadoOsada;
}

int proce_mkdir(int clientSocket, char* path){
	int resultadoOsada = 0;
	t_makeDir* makeDir = malloc(sizeof(t_makeDir));
	recv(clientSocket,&(makeDir->mode),sizeof(mode_t),0);

	resultadoOsada = osada_createDir(path, obtenerNombreDelDirectorio(path), makeDir->mode);

	send(clientSocket,&resultadoOsada,sizeof(int),0);

	free(makeDir);
	return resultadoOsada;
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

int proce_write(int clientSocket, char* path){
	int resultadoOsada = 0;
	char* buffer = string_new();

	t_write* swrite = malloc(sizeof(t_write));

	recv(clientSocket,&(swrite->size),sizeof(size_t),0);
	recv(clientSocket,&(swrite->offset),sizeof(off_t),0);

	resultadoOsada = osada_write(path, buffer, swrite->size, swrite->offset);

	send(clientSocket,&resultadoOsada,sizeof(int),0);

	free(swrite);
	return resultadoOsada;
}

int proce_statfs(int clientSocket, char* path){
	int resultadoOsada = 0;
	t_statfs* statfs = malloc(sizeof(t_statfs));

	resultadoOsada = osada_statfs(path,statfs);
	send(clientSocket,&resultadoOsada,sizeof(int),0);

	if(resultadoOsada == 1){
		send(clientSocket,&(statfs->__f_spare),sizeof(int),0);
		send(clientSocket,&(statfs->f_bavail),sizeof(__fsblkcnt_t),0);
		send(clientSocket,&(statfs->f_bfree),sizeof(__fsblkcnt_t),0);
		send(clientSocket,&(statfs->f_blocks),sizeof(__fsblkcnt_t),0);
		send(clientSocket,&(statfs->f_bsize),sizeof(unsigned long int),0);
		send(clientSocket,&(statfs->f_favail),sizeof(__fsfilcnt_t),0);
		send(clientSocket,&(statfs->f_ffree),sizeof(__fsfilcnt_t),0);
		send(clientSocket,&(statfs->f_files),sizeof(__fsfilcnt_t),0);
		send(clientSocket,&(statfs->f_flag),sizeof(unsigned long int),0);
		send(clientSocket,&(statfs->f_frsize),sizeof(unsigned long int),0);
		send(clientSocket,&(statfs->f_fsid),sizeof(unsigned long int),0);
		send(clientSocket,&(statfs->f_namemax),sizeof(unsigned long int),0);
	}

	free(statfs);
	return resultadoOsada;
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
