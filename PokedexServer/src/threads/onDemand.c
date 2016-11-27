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
#include "../functions/primary_functions.h"
#include "../functions/auxiliar_functions.h"
#include <errno.h>

void* procesarPeticiones(t_hilo* h){
	printf("Se conecto %d\n", h->id);
	char* nombreFuncion=string_new();
	int result=-ENOENT;

	while(recibirNombreDeLaFuncion(h->socket,nombreFuncion)){
		int tamanioPath;
		recibirTamanioDelPath(h->socket,&tamanioPath);

		//char* path=malloc(sizeof(char)*tamanioPath);
		char* path=string_new();
		recibirPath(h->socket,&path,tamanioPath);
		log_info(logPokedexServer,"Funcion: %s\n",nombreFuncion);
		log_info(logPokedexServer,"Path: %s\n",path);
		//printf("Funcion: %s\nPath:%s\n", nombreFuncion, path);
		//recibirPath(h->socket,path,tamanioPath);

		//printf("Funcion: %s\nPath:%s\nTamanio:%d", nombreFuncion, path, sizeof(path));



		if(string_equals_ignore_case(nombreFuncion, "GETAT")){
			//result=proce_getattr(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "READD")){
			//result = proce_readdir(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "OPENF")){
			//result = proce_open(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "READF")){
			//result = proce_readfile(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "CREAT")){
			//result = proce_create(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "TRUNC")){
			//result  = proce_truncate(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "MKDIR")){
			//result = proce_mkdir(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "RENAM")){
			//result = proce_rename(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "ULINK")){
			//result = proce_removeFile(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "RMDIR")){
			//result = proce_removeDir(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "WRITE")){
			//result = proce_write(*socket);
		}else if(string_equals_ignore_case(nombreFuncion, "STATF")){
			//result=proce_statfs(*socket);
		}

		//devolverResultadoAlCliente(result,h->socket);
	}

	return NULL;
}
