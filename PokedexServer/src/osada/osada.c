/*
 * osada.c
 *
 *  Created on: 20/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <commons/collections/list.h>
#include "commons/declarations.h"
#include "functions/tabla_archivos.h"
#include "commons/osada.h"
#include "osada.h"
#include "mapp.h"

int osada_init(char* path){
	initOsada (path);
	return 1;
}

int osada_readdir(char* path, t_list* directorios){
	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
		u_int16_t parent = osada_TA_obtenerUltimoHijoFromPath(path);
	//Obtengo los directorios
		osada_TA_obtenerDirectorios(parent, directorios);
	//Return
		return 1;


}

int osada_getattr(char* path, file_attr* attrs){
	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);

	if(indice>=0){
		osada_TA_obtenerAttr(indice, attrs);
		return 1;
	}else if(indice){
		attrs->file_size = 0;
		attrs->state = 2;
		return 1;
	}else{
		return -ENOENT;
	}
}

int osada_read(char *path, char *buf, size_t size, off_t offset){
	/*u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);
	if(strcmp(path, "/") != 0){
		return 1;
	}else{
		return -ENOENT;
	}*/
}

int osada_open(char* path){
	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
		u_int16_t child = osada_TA_obtenerUltimoHijoFromPath(path);
		if(child>=0){
			if(osada_drive.directorio[child].state ==2){
				return -EACCES;
			}
		}

	return -ENOENT;
}
