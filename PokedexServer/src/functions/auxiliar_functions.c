/*
 * auxiliar_functions.c
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

char* obtenerNombreDelDirectorio(char* path){
	int tamanio = strlen(*string_split(path,"/"));
	return string_split(path,"/")[tamanio-1];
}

void devolverResultadoAlCliente(int resultadoDeOsada,int socketCliente){
	if(send(socketCliente,&resultadoDeOsada,sizeof(int),0)>=1){
		printf("Se envia correctamente el resultado al cliente\n");
	}
}

int recibirNombreDeLaFuncion(int socketCliente, char* nombreFuncion){
	if(recv(socketCliente, nombreFuncion, 5*sizeof(char),0)>=0){
		log_info(logPokedexServer,"POKEDEXSERVER - Recibo el nombre de la funcion %s", nombreFuncion);
		return 1;
	}else{
		log_info(logPokedexServer,"POKEDEXSERVER - Se cerro la conexion");
		return 0;
	}

}

void recibirParametrosDeReadDir(int socketCliente,char* path){
	recv(socketCliente,path,sizeof(path),0);
}

void recibirTamanioDelPath(int socketCliente, int* tamanio){
	char* size=(char*)malloc(11*sizeof(char));
	recv(socketCliente,size,11,0);
	*tamanio=atoi(size);
	log_info(logPokedexServer,"POKEDEXSERVER - Recibo el tamanio del path: %d",*tamanio);
}

void recibirPath(int socketCliente,char** path, int tamanioPath){
	char* pathPiloto=malloc(sizeof(char)*tamanioPath);
	recv(socketCliente,pathPiloto,tamanioPath,0);
	*path=string_substring(pathPiloto,0,tamanioPath);
	string_append(path,"\0");
	free(pathPiloto);
}

void recibirBuffer(int socketCliente, file_attr* getAttr){
	recv(socketCliente,&(getAttr->file_size),sizeof(getAttr->file_size),0);
}

void enviarBufferLleno(int socketCliente, file_attr* getAttr){
	send(socketCliente,&(getAttr->state),sizeof(getAttr->state),0);
	send(socketCliente,&(getAttr->file_size),sizeof(getAttr->file_size),0);
	log_info(logPokedexServer,"POKEDEXSERVER - Envio buffer con estado y size");
}
