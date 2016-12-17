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
#include "../osada/functions/bitmap.h"
#include "../osada/basura.h"
#include <pthread.h>
#include <stdint.h>
#include <pthread.h>
#include "../commons/structures.h"
#include "../commons/definitions.h"
#include "../functions/sockets.h"
#include <errno.h>

void proce_getattr(int clientSocket, char* path){ //LEAK!! en todos los sendInt
	file_attr* getAttr = malloc(sizeof(file_attr));
	getAttr->file_size=0;
	getAttr->state=0;
	getAttr->lastmod=time(NULL);

	int resultadoOsada = osada_getattr(path,getAttr);
	sendInt(clientSocket, resultadoOsada);
	log_info(logPokedexServer, "ResultadoOsada: %d", resultadoOsada);

	if(resultadoOsada==1){
		sendInt(clientSocket, getAttr->state);
		log_info(logPokedexServer, "getAttr->state: %d", getAttr->state);

		sendInt(clientSocket, (int)getAttr->file_size);
		log_info(logPokedexServer, "getAttr->file_size: %d", getAttr->file_size);

		send(clientSocket, &(getAttr->lastmod), sizeof(time_t), 0);
	//	sendInt(clientSocket, getAttr->lastmod);
		log_info(logPokedexServer, "Fecha: %s", ctime(&getAttr->lastmod));

	}

	free(path);
	free(getAttr);
}

void proce_readdir(int clientSocket, char* path){
	t_list* directorios=list_create();

	int resultadoOsada = osada_readdir(path, directorios);
	sendInt(clientSocket, resultadoOsada);
	log_info(logPokedexServer, "ResultadoOsada: %d", resultadoOsada);

	if(resultadoOsada != -1){
		sendInt(clientSocket, list_size(directorios));
		log_info(logPokedexServer, "list_size: %d", list_size(directorios));

		int i;
		for(i=0;i<list_size(directorios);i++){
			char* directory = (char*)list_get(directorios, i);

			sendString(clientSocket, directory, string_length(directory));
			log_info(logPokedexServer, "Directorio: %s", directory);
		}

	}
	free(path);
	list_destroy(directorios);
}

void proce_open(int clientSocket, char* path){
	int resultadoOsada = osada_open(path);
	sendInt(clientSocket,resultadoOsada);
	log_info(logPokedexServer, "ResultadoOsada: %d", resultadoOsada);
	free(path);
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
//		int i;
//		log_info(logPokedexServer, "---------IMPRIMO EL BUFFER EN PROCE_READFILE---------");
//		for (i=0; i<resultadoOsada; i++){
//			log_info(logPokedexServer, "%d", bufParaElRead[i]);
//		}

	//	sendString(clientSocket, bufParaElRead, resultadoOsada);
		sendBufferParaRead(clientSocket, bufParaElRead, resultadoOsada);
	}
	free(bufParaElRead);
	free(path);
}

void proce_create(int clientSocket, char* path){
	int resultadoOsada = osada_createFile(path);
	sendInt(clientSocket, resultadoOsada);
	log_info(logPokedexServer,"ResultadoOsada %d",resultadoOsada);
	free(path);
}

void proce_truncate(int clientSocket, char* path){
	off_t offset;
	recvValue(clientSocket,&offset);
	log_info(logPokedexServer, "Offset: %d", offset);

	int resultadoOsada = osada_truncate(path, offset);
	sendInt(clientSocket, resultadoOsada);
	log_info(logPokedexServer,"ResultadoOsada %d",resultadoOsada);

	free(path);
}

void proce_mkdir(int clientSocket, char* path){
	int resultadoOsada = osada_createDir(path);
	sendInt(clientSocket,resultadoOsada);
	log_info(logPokedexServer,"ResultadoOsada: %d",resultadoOsada);

	free(path);
}

void proce_rename(int clientSocket, char* path){
	char* newPath=string_new();
	recvString(clientSocket, &newPath);
	log_info(logPokedexServer, "NewPath: %s", newPath);

	int resultadoOsada = osada_rename(path, newPath);
	sendInt(clientSocket,resultadoOsada);
	log_info(logPokedexServer, "ResultadoOsada: %d", resultadoOsada);

	free(newPath);
	free(path);
}

void proce_removeFile(int clientSocket, char* path){
	int resultadoOsada = osada_removeFile(path);
	sendInt(clientSocket,resultadoOsada);
	log_info(logPokedexServer, "ResultadoOsada: %d", resultadoOsada);

	free(path);
}

void proce_removeDir(int clientSocket, char* path){
	int resultadoOsada = osada_removeDir(path);
	sendInt(clientSocket,resultadoOsada);
	log_info(logPokedexServer, "ResultadoOsada: %d", resultadoOsada);

	free(path);
}

void proce_write(int clientSocket, char* path){
	size_t size;
	off_t offset;


	recvValue(clientSocket,&size);
	recvValue(clientSocket,&offset);

	char* bufParaElWrite = malloc(size);
//	recvString(clientSocket,&bufParaElWrite);

	recvBufferParaWrite(clientSocket, &bufParaElWrite, size);

	log_info(logPokedexServer,"El size_t que me llega para WRITE es: %d",size);
	log_info(logPokedexServer,"El off_t que me llega para WRITE es: %d",offset);
//	log_info(logPokedexServer,"----------IMPRIMO EL BUFFER RECIBIDO EN PROCE_WRITE----------");
//	int i;
//	for (i=0; i<size; i++){
//		log_info(logPokedexServer,"%d", bufParaElWrite[i]);
//	}


	int resultadoOsada = osada_write(path, &bufParaElWrite, size, offset);
	sendInt(clientSocket, resultadoOsada);
	log_info(logPokedexServer,"ResultadoOsada: %d",resultadoOsada);

	free(bufParaElWrite);
	free(path);
}

void proce_statfs(int clientSocket, char* path){
	sendInt(clientSocket, 1);

	sendInt(clientSocket, osada_B_cantBloquesLibres());
	sendInt(clientSocket, OSADA_BLOCK_SIZE);
	sendInt(clientSocket, bitarray_get_max_bit(osada_drive.bitmap));
	sendInt(clientSocket, osada_TA_cantRegistrosLibres());
	sendInt(clientSocket, OSADA_FILENAME_LENGTH);

	free(path);
}

void* procesarPeticiones(t_hilo* h){
	printf("Se conecto %d\n", h->id);
	char* nombreFuncion=string_new(); //LEAK!! a pesar de que este liberado

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
	log_info(logPokedexServer, "Se cerro la conexion.");
	printf("Se cerro la conexion");
	free(nombreFuncion);
	return NULL;
}



