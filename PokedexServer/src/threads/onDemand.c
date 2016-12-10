/*
 * onDemand.c
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
#include "../functions/sockets.h"
#include <errno.h>

void proce_getattr(int clientSocket, char* path){
	file_attr* getAttr = malloc(sizeof(file_attr));
	getAttr->file_size=0;
	getAttr->state=0;

	int resultadoOsada = osada_getattr(path,getAttr);
	free(path);
	log_info(logPokedexServer, "POKEDEXSERVER  - 3 - resultadoOsada: %d", resultadoOsada);
	sendInt(clientSocket, resultadoOsada);

	if(resultadoOsada==1){
		log_info(logPokedexServer, "POKEDEXSERVER  - 4 - getAttr->state: %d", getAttr->state);
		log_info(logPokedexServer, "POKEDEXSERVER  - 5 - getAttr->file_size: %d", getAttr->file_size);

		sendInt(clientSocket, getAttr->state);
		sendInt(clientSocket, (int)getAttr->file_size);
	}

	free(getAttr);
}

void proce_readdir(int clientSocket, char* path){
	t_list* directorios=list_create();

	int resultadoOsada = osada_readdir(path, directorios);
	free(path);
	log_info(logPokedexServer, "resultadoOsada: %d", resultadoOsada);
	sendInt(clientSocket, resultadoOsada);

	if(resultadoOsada != -1){
		log_info(logPokedexServer, "list_size: %d", list_size(directorios));
		sendInt(clientSocket, list_size(directorios));
		int i;
		for(i=0;i<list_size(directorios);i++){
			char* directory = (char*)list_get(directorios, i);
			log_info(logPokedexServer, "Directorio: %s", directory);
			if(sendString(clientSocket, directory, string_length(directory))==0){
				log_info(logPokedexServer, "No envie valor");
			}
		}

	}


}

void proce_open(int clientSocket, char* path){

	int resultadoOsada = osada_open(path);

	sendInt(clientSocket,resultadoOsada);
}

void proce_readfile(int clientSocket, char* path){


	size_t size;
	off_t offset;
	char* bufParaElRead = malloc(OSADA_BLOCK_SIZE);

	recvValue(clientSocket,&size);
	recvValue(clientSocket,&offset);

	log_info(logPokedexServer,"El size_t que me llega para READF es: %d",size);
	log_info(logPokedexServer,"El off_t que me llega para READF es: %d",offset);

	int resultadoOsada = osada_read(path, &bufParaElRead, size, offset);
	sendInt(clientSocket, resultadoOsada);


	if(resultadoOsada > 0){
		log_info(logPokedexServer,"El tamanio (devuelto por resultadoOsada) es %d",resultadoOsada);
		log_info(logPokedexServer, "Voy a enviar como size del buf %d con el contenido %s", resultadoOsada, bufParaElRead);
		sendString(clientSocket, bufParaElRead, resultadoOsada);

	}

//	free(buf);
}

void proce_create(int clientSocket, char* path){

	mode_t modo = 0;
//	recvValue(clientSocket, &modo);

	int resultadoOsada = osada_createFile(path, modo);

	sendInt(clientSocket, resultadoOsada);

}

void proce_truncate(int clientSocket, char* path){

	off_t offset;
	recvValue(clientSocket,&offset);
	log_info(logPokedexServer, "Recibi el offset: %d", offset);

	int resultadoOsada = osada_truncate(path, offset);

	sendInt(clientSocket, resultadoOsada);
}

void proce_mkdir(int clientSocket, char* path){
	log_info(logPokedexServer,"Se quieren crear el directorio %s",path);
	int resultadoOsada = 0;
	//int tamanio = string_length(*string_split(path,"/"));
	resultadoOsada = osada_createDir(path);
	log_info(logPokedexServer,"Recibo el resultado %d de OSADA",resultadoOsada);
	//send(clientSocket,&resultadoOsada,sizeof(int),0);
	sendInt(clientSocket,resultadoOsada);
	log_info(logPokedexServer,"Envie el resultado igual a %d de OSADA",resultadoOsada);

}

void proce_rename(int clientSocket, char* path){
	int resultadoOsada = 0;

	char* newPath=string_new();
	recvString(clientSocket, &newPath);
	log_info(logPokedexServer, "El nuevoPath recibido es: %s", newPath);

	resultadoOsada = osada_rename(path, newPath);

	sendInt(clientSocket,resultadoOsada);


	free(newPath);
}

void proce_removeFile(int clientSocket, char* path){
	int resultadoOsada = osada_removeFile(path);

	sendInt(clientSocket,resultadoOsada);
}

void proce_removeDir(int clientSocket, char* path){
	int resultadoOsada = osada_removeDir(path);

	sendInt(clientSocket,resultadoOsada);

}

void proce_write(int clientSocket, char* path){
	size_t size;
	off_t offset;
	char* bufParaElWrite;

	recvValue(clientSocket,&size);
	recvValue(clientSocket,&offset);
	recvString(clientSocket,&bufParaElWrite);

	log_info(logPokedexServer,"El size_t que me llega para WRITE es: %d",size);
	log_info(logPokedexServer,"El off_t que me llega para WRITE es: %d",offset);
	log_info(logPokedexServer, "El buffer que me llega para WRITE es: %s",bufParaElWrite);

	// int indice = osada_TA_obtenerIndiceTA(path);

	// char* buf = malloc(osada_drive.directorio[indice].file_size);

	int resultadoOsada = osada_write(path, &bufParaElWrite, size, offset);
	sendInt(clientSocket, resultadoOsada);


	if(resultadoOsada > 0){
		log_info(logPokedexServer,"El tamanio (devuelto por resultadoOsada) es %d",resultadoOsada);
	}
}

void proce_statfs(int clientSocket, char* path){
	t_statfs* statfs = malloc(sizeof(t_statfs));

	int resultadoOsada = osada_statfs(path,statfs);
	sendInt(clientSocket, resultadoOsada);

	if(resultadoOsada == 1){
		sendInt(clientSocket, contarBloquesLibresTotales());
		sendInt(clientSocket, OSADA_BLOCK_SIZE);
		sendInt(clientSocket, osada_drive.header->fs_blocks);
		sendInt(clientSocket, contarOsadaFilesLibres());
		sendInt(clientSocket, OSADA_FILENAME_LENGTH);
	}

	free(statfs);
}

void* procesarPeticiones(t_hilo* h){
	printf("Se conecto %d\n", h->id);
	char* nombreFuncion=string_new();

	while(recibirNombreDeLaFuncion(h->socket,nombreFuncion)){
		log_info(logPokedexServer,"POKEDEXSERVER - 1 - Funcion: %s", nombreFuncion);

		char* path=string_new();
		recibirPath(h->socket,&path);
		log_info(logPokedexServer,"POKEDEXSERVER - 2 - Path: %s",path);

		if(string_equals_ignore_case(nombreFuncion, "GETAT")){
			proce_getattr(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "READD")){
			proce_readdir(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "OPENF")){
			proce_open(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "READF")){
			proce_readfile(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "CREAT")){
			proce_create(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "TRUNC")){
			proce_truncate(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "MKDIR")){
			proce_mkdir(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "RENAM")){
			proce_rename(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "ULINK")){
			proce_removeFile(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "RMDIR")){
			proce_removeDir(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "WRITE")){
			proce_write(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "STATF")){
			proce_statfs(h->socket, path);
		}else{
			log_info(logPokedexServer, "Funcion desconocida");
		}

	}
	free(nombreFuncion);

	return NULL;
}
