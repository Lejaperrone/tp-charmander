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
		log_info(logPokedexServer,"PokedexServer: Recibo %s", nombreFuncion);
		return 1;
	}else{
		log_info(logPokedexServer,"PokedexServer: Se cerro la conexion");
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
	log_info(logPokedexServer,"PokedexServer: Recibo el tamanio del path: %d",*tamanio);
}

void recibirPath(int socketCliente,char* path, int tamanioPath){
	recv(socketCliente,path,tamanioPath,0);
	log_info(logPokedexServer,"PokedexServer: Recibo el path %s",path);
}

void recibirBuffer(int socketCliente, t_getAttr* getAttr){
	recv(socketCliente,&(getAttr->primerP),sizeof(getAttr->primerP),0);
}

void enviarBufferLleno(int socketCliente, t_getAttr* getAttr){
	send(socketCliente,&(getAttr->primerP),sizeof(getAttr->primerP),0);
	log_info(logPokedexServer,"PokedexServer: Envio buffer lleno");
}
