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

	/*osada_drive.directorio[0].fname[0] = 't';
	osada_drive.directorio[0].fname[1] = 'e';
	osada_drive.directorio[0].fname[2] = 's';
	osada_drive.directorio[0].fname[3] = 't';
	osada_drive.directorio[0].fname[4] = '1';
	osada_drive.directorio[0].fname[5] = '\0';
	osada_drive.directorio[0].state = 2;
	osada_drive.directorio[0].parent_directory = 0xFFFF;
	osada_drive.directorio[1].fname[0] = 't';
	osada_drive.directorio[1].fname[1] = 'e';
	osada_drive.directorio[1].fname[2] = 's';
	osada_drive.directorio[1].fname[3] = 't';
	osada_drive.directorio[1].fname[4] = '2';
	osada_drive.directorio[1].fname[5] = '\0';
	osada_drive.directorio[1].state = 2;
	osada_drive.directorio[1].parent_directory = 0;
	osada_drive.directorio[2].fname[0] = 't';
	osada_drive.directorio[2].fname[1] = 'e';
	osada_drive.directorio[2].fname[2] = 's';
	osada_drive.directorio[2].fname[3] = 't';
	osada_drive.directorio[2].fname[4] = '3';
	osada_drive.directorio[2].fname[5] = '\0';
	osada_drive.directorio[2].state = 2;
	osada_drive.directorio[2].parent_directory = 0xFFFF;


	t_list * l = list_create();
	if(osada_readdir("/test1", l) == 0){
		int j;
		for(j=0; j<list_size(l); j++){
				char* pokenest = (char*)list_get(l, j);
				printf("%s\n",pokenest);
		}
	}*/


}

int osada_readdir(char* path, t_list* directorios){
	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
		u_int16_t parent = osada_TA_obtenerUltimoHijoFromPath(path);
		if(parent>=0){
			//Obtengo los directorios
				osada_TA_obtenerDirectorios(parent, directorios);
			//Return
				return 0;
		}else{
			//Return
				return parent;
		}

}

int osada_getattr(char* path, file_attr* attrs){
	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);

		if(strcmp(path, "/") != 0){
			osada_TA_obtenerAttr(indice, attrs);
			return 1;
		}else{
			return -ENOENT;
		}
}

int osada_read(char *path, char *buf, size_t size, off_t offset){
	return -ENOENT;
}

int osada_open(char* path){
	return -ENOENT;
}
