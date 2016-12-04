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

int sendString(int clientSocket, char* parameter, int size){
	char* sizeStr=malloc(sizeof(char)*11);
	sprintf(sizeStr,"%i",size);
	if(send(clientSocket, sizeStr, 11, 0) == 11){
		if(send(clientSocket, parameter, size, 0) == size){
			free(sizeStr);
			return 1;
		}
	}
	free(sizeStr);
	return 0;
}
int recvValue(int clientSocket, void* buffer){
	char* sizeStr = malloc(sizeof(char)*11);
	if (recv(clientSocket, sizeStr, 11,  0) == 11){
		int size = atoi(sizeStr);
		if (recv(clientSocket, buffer, size,  0) == size){
			return 1;
		}
	}
	return 0;
}
int recvString(int clientSocket, char** string){
	char* sizeStr = malloc(sizeof(char)*11);
	if (recv(clientSocket, sizeStr, 11,  0) == 11){
		int size = atoi(sizeStr);
		char* temp=malloc(sizeof(char)*size);
		if (recv(clientSocket, temp, size,  0) == size){
			*string = string_substring(temp,0,size);
			return 1;
		}
	}
	return 0;
}
int recvInt(int clientSocket){
	char* sizeStr = malloc(sizeof(char)*11);
	if (recv(clientSocket, sizeStr, 11,  0) == 11){
		int size = atoi(sizeStr);
		return size;
	}

	return -1;
}
int sendInt(int clientSocket, int number){
	char* numberStr=malloc(sizeof(char)*11);

	sprintf(numberStr,"%i",number);
	if(send(clientSocket, numberStr, 11, 0) == 11){
		free(numberStr);
		return 1;
	}
	free(numberStr);
	return 0;
}

int recibirNombreDeLaFuncion(int socketCliente, char* nombreFuncion){
	if(recv(socketCliente, nombreFuncion, 5*sizeof(char),0)>=0){
		return 1;
	}else{
		log_info(logPokedexServer,"POKEDEXSERVER - Se cerro la conexion");
		return 0;
	}

}
void recibirPath(int socketCliente,char** path){
	recvString(socketCliente, path);
}
