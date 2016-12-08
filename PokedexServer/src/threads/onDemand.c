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
	int result=0;

	char* bufParaElRead = malloc(1000000);
	while(recibirNombreDeLaFuncion(h->socket,nombreFuncion)){

		char* path=string_new();
		recibirPath(h->socket,&path);

		log_info(logPokedexServer,"POKEDEXSERVER - 1 - Funcion: %s", nombreFuncion);
		log_info(logPokedexServer,"POKEDEXSERVER - 2 - Path: %s",path);

		if(string_equals_ignore_case(nombreFuncion, "GETAT")){
			proce_getattr(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "READD")){
			proce_readdir(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "OPENF")){
			proce_open(h->socket, path);
		}else if(string_equals_ignore_case(nombreFuncion, "READF")){
			proce_readfile(h->socket, path, &bufParaElRead);
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
	free(bufParaElRead);
	free(nombreFuncion);

	return NULL;
}
