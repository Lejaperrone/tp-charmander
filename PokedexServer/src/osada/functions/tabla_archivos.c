/*
 * tabla_archivos.c
 *
 *  Created on: 20/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "../commons/declarations.h"
#include "../commons/osada.h"
#include <errno.h>

int compare(int indice, char* test2){
	int i;
	for(i=0; i<OSADA_FILENAME_LENGTH; i++){
		printf("%c - %c\n", osada_drive.directorio[indice].fname[i], *test2);
		if(osada_drive.directorio[indice].fname[i] == '\0' && *test2=='\0'){
			return 1;
		}else{
			if(osada_drive.directorio[indice].fname[i] != *test2){
				return 0;
			}
		}

		test2++;
	}

	return 1;
}

u_int16_t osada_TA_buscarRegistroPorNombre(char* nombre, u_int16_t parent){
	int i;
	for(i=0;i<1024;i++){
		if(compare(i, nombre) && osada_drive.directorio[i].parent_directory == parent){
			return i;
		}
	}

	return -1;
}

void osada_TA_obtenerDirectorios(u_int16_t parent, t_list* directorio){
	int i;
	for(i=0;i<1024;i++){
		if(osada_drive.directorio[i].parent_directory == parent && osada_drive.directorio[i].state!=0){
			list_add(directorio, osada_drive.directorio[i].fname);
		}
	}

	return;
}

int osada_TA_obtenerUltimoHijoFromPath(char* path){
	char ** dirc = string_split(path, "/");
	u_int16_t child = 0xFFFF;
	int i=0;
	while(dirc[i]!=NULL){
		if(sizeof(dirc[i]) != 0){
			child = osada_TA_buscarRegistroPorNombre(dirc[i], child);
			if(child == -1){
				return -ENOENT;
			}
		}
		i++;
	}

	return child;
}

void osada_TA_obtenerAttr(u_int16_t indice, file_attr* attr){
	attr->file_size = osada_drive.directorio[indice].file_size;
	attr->state = osada_drive.directorio[indice].state;
}
