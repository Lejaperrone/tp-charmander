/*
 * sockets.c
 *
 *  Created on: 10/12/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <commons/string.h>

#include "../commons/structures.h"
#include "../commons/definitions.h"

int sendBufferParaRead(int clientSocket, char* parameter, int size){

//	div_t divisionEnPartes = div(size, 32768);
//	int vecesQueHayQueHacerSend = divisionEnPartes.quot;
//
//	if(size <= 32768){
//		log_info(logPokedexServer, "IMPORTANTE - recibirBuffer - Envio al cliente la cant de bytes para el buffer: %d", size);
//		if(send(clientSocket, parameter, size, 0) == size){
//			return 1;
//		}
//
//	}else{
//		int i;
//		int desplazam = 0;
//		for(i=0 ; i<vecesQueHayQueHacerSend; i++){
//			char* bufferDe32kb = malloc(32768*sizeof(char));
//			memcpy(bufferDe32kb, parameter+desplazam, 32768);
//			desplazam += 32768;
//			log_info(logPokedexServer, "IMPORTANTE - recibirBuffer - La cant de veces que voy a enviar 32768 bytes al cliente es: %d", vecesQueHayQueHacerSend);
//			send(clientSocket, bufferDe32kb, 32768, 0);
//			free(bufferDe32kb);
//		}
//		if(divisionEnPartes.rem > 0){
//			char* bufferDelRestoDeBytes = malloc((divisionEnPartes.rem)*sizeof(char));
//			memcpy(bufferDelRestoDeBytes, parameter+desplazam, divisionEnPartes.rem);
//			log_info(logPokedexServer, "IMPORTANTE - recibirBuffer - Envio al cliente la cant de bytes para el buffer: %d", divisionEnPartes.rem);
//			send(clientSocket, parameter+desplazam, divisionEnPartes.rem, 0);
//			free(bufferDelRestoDeBytes);
//		}
//	}
//
//	return 0;

	send(clientSocket, parameter, size, 0);

	return 1;
}

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
int recvString(int clientSocket, char** string){
	char* sizeStr = malloc(sizeof(char)*11);
	if (recv(clientSocket, sizeStr, 11,  0) == 11){
		int size = atoi(sizeStr);
		free(sizeStr);
		char* temp=malloc(sizeof(char)*size);
		if (recv(clientSocket, temp, size,  0) == size){
			*string = string_substring(temp,0,size);
			free(temp);
			return 1;
		}else{
			free(temp);
		}
	}else{
		free(sizeStr);
	}

	return 0;
}

int sendValue(int clientSocket, char* parameter, int size){
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
int recvBufferParaWrite(int clientSocket, char** buffer, int size){

	char* bufAuxiliar = malloc(size);
	if(size <= 32768){
		recv(clientSocket, bufAuxiliar, size, 0);
		memcpy(*buffer, bufAuxiliar, size);
		free(bufAuxiliar);
	}else{
		div_t divisionEnPartes = div(size, 32768);
		int vecesQueHayQueHacerRecv = divisionEnPartes.quot;

		int i;
		int desplazam = 0;
		for(i=0 ; i<vecesQueHayQueHacerRecv; i++){
			char* bufferDe32kb = malloc(32768*sizeof(char));
			log_info(logPokedexServer, "IMPORTANTE - recibirBuffer - Las veces que hay que hacer recvs de 32768 es: %d",vecesQueHayQueHacerRecv);
			recv(clientSocket, bufferDe32kb, 32768, 0);
			memcpy(bufAuxiliar+desplazam, bufferDe32kb, 32768);
			desplazam += 32768;

			free(bufferDe32kb);
		}
		if(divisionEnPartes.rem > 0){
			char* bufferDelRestoDeBytes = malloc((divisionEnPartes.rem)*sizeof(char));
			log_info(logPokedexServer, "IMPORTANTE - recibirBuffer - Recibo del cliente la cant de bytes: %d", divisionEnPartes.rem);
			recv(clientSocket, bufferDelRestoDeBytes, divisionEnPartes.rem, 0);
			memcpy(bufAuxiliar+desplazam, bufferDelRestoDeBytes, divisionEnPartes.rem);
			free(bufferDelRestoDeBytes);
		}
		memcpy(*buffer, bufAuxiliar, size);
	}

	return 1;
}
int recvValue(int clientSocket, void* buffer){
	char* sizeStr = malloc(sizeof(char)*11);
	if (recv(clientSocket, sizeStr, 11,  0) == 11){
		int size = atoi(sizeStr);
		free(sizeStr);
		if (recv(clientSocket, buffer, size,  0) == size){
			return 1;
		}
	}
	free(sizeStr);
	return 0;
}

int recvInt(int clientSocket){
	char* sizeStr = malloc(sizeof(char)*11);
	if (recv(clientSocket, sizeStr, 11,  0) == 11){
		int size = atoi(sizeStr);
		free(sizeStr);
		return size;
	}
	free(sizeStr);
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
	if(recv(socketCliente, nombreFuncion, 5*sizeof(char),0)>0){
		return 1;
	}else{
		log_info(logPokedexServer,"POKEDEXSERVER - Se cerro la conexion");
		return 0;
	}

}
void recibirPath(int socketCliente,char** path){
	recvString(socketCliente, path);
}
