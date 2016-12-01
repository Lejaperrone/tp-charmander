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

void proce_getattr(int clientSocket, char* path){
	file_attr* getAttr = malloc(sizeof(file_attr));
	getAttr->file_size=0;
	getAttr->state=0;

	int resultadoOsada = osada_getattr(path,getAttr);
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

void destructorDeNodoDirectorio(unsigned char* directorio){
	free(directorio);
}

void proce_readdir(int clientSocket, char* path){
	t_list* directorios=list_create();

	int resultadoOsada = osada_readdir(path, directorios);
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

void proce_readfile(int clientSocket, char* path){
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
}

void proce_create(int clientSocket, char* path){
	int resultadoOsada = 0;
	t_createFile* createFile = malloc(sizeof(t_createFile));

	recv(clientSocket,&(createFile->modo),sizeof(mode_t),0);
	resultadoOsada = osada_createFile(path, createFile->modo);

	send(clientSocket,&resultadoOsada,sizeof(int),0);


	free(createFile);
}

void proce_truncate(int clientSocket, char* path){
	int resultadoOsada = 0;
	t_truncateFile* truncateFile = malloc(sizeof(t_truncateFile));

	recv(clientSocket,&(truncateFile->offset),sizeof(off_t),0);
	resultadoOsada = osada_truncate(path, truncateFile->offset);

	send(clientSocket,&resultadoOsada,sizeof(int),0);


	free(truncateFile);
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
		char* newPath=string_new();
		recibirPath(clientSocket,&newPath);
		int resultadoOsada = osada_rename(path, newPath);
		sendInt(clientSocket,resultadoOsada);
}

void proce_write(int clientSocket, char* path){
	int resultadoOsada = 0;
	char* buffer = string_new();

	t_write* swrite = malloc(sizeof(t_write));

	recv(clientSocket,&(swrite->size),sizeof(size_t),0);
	recv(clientSocket,&(swrite->offset),sizeof(off_t),0);

	resultadoOsada = osada_write(path, buffer, swrite->size, swrite->offset);

	send(clientSocket,&resultadoOsada,sizeof(int),0);

	free(swrite);
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

void proce_removeFile(int clientSocket, char* path){
	int resultadoOsada = osada_removeFile(path);

	sendInt(clientSocket,resultadoOsada);
}

void proce_removeDir(int clientSocket, char* path){
	int resultadoOsada = osada_removeDir(path);

	sendInt(clientSocket,resultadoOsada);

}

void proce_open(int clientSocket, char* path){
	int resultadoOsada;
	resultadoOsada = osada_open(path);
	send(clientSocket,&resultadoOsada,sizeof(int),0);
}
