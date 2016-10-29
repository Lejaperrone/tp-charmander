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
#include <commons/temporal.h>
#include "../commons/declarations.h"
#include "../commons/osada.h"
#include <errno.h>
#include "../functions/tabla_asignaciones.h"

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
	for(i=0;i<2048;i++){
		if(compare(i, nombre) && osada_drive.directorio[i].parent_directory == parent){
			return i;
		}
	}

	return -1;
}

//agrega a la lista de directorios los directorios de esa path
void osada_TA_obtenerDirectorios(u_int16_t parent, t_list* directorio){
	int i;
	for(i=0;i<2048;i++){
		//state=2
		if(osada_drive.directorio[i].parent_directory == parent && osada_drive.directorio[i].state!=0){
			list_add(directorio, osada_drive.directorio[i].fname);
		}
	}

	return;
}
void darDeAltaDirectorioEnTablaDeArchivos(char* nombre,t_list* listaDeBloques){
	int i;
	for (i=0;i<2048;i++){
		if(osada_drive.directorio[i].state==0){
			osada_drive.directorio[i].file_size=4096;
			osada_drive.directorio[i].first_block=(int)list_get(listaDeBloques,0);
			strcpy(osada_drive.directorio[i].fname,nombre);
			osada_drive.directorio[i].lastmod=temporal_get_string_time();
			// no sabemos si el directorio padre de un directorio nuevo es 0xFFFF u otro
			osada_drive.directorio[i].parent_directory=0xFFFF;
			osada_drive.directorio[i].state=2;
		}
	}
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
void osada_TA_setearAttr(u_int16_t indice, file_attr* attr){
	attr->file_size=0;
	attr->state=0;
}


int osada_TA_borrarArchivo(char* nombre, u_int16_t parent){
	int i;
	int subindiceParaBitmap;
	bool hayMasBloques=true;
	int pudeBorrar=0;
	for(i=0;i<2048;i++){
	if(compare(i, nombre) && osada_drive.directorio[i].parent_directory == parent){
		if(remove(nombre)==0){
			pudeBorrar=1;
			osada_TA_setearAttr(i, attr);
			subindiceParaBitmap=osada_drive.directorio[i].first_block;
			actualizarTablaDeAsignaciones_porBaja(&subindiceParaBitmap,&hayMasBloques);
		}else{
			perror("Error borrando archivo");
		}
	}
	}
	while (hayMasBloques){
	actualizarBitmap_porBaja(subindiceParaBitmap);
	actualizarTablaDeAsignaciones_porBaja(&subindiceParaBitmap, &hayMasBloques);
	}

	return pudeBorrar;
}

void borrarSubdirectoriosYArchivos(t_list*directorio){

}

void osada_TA_borrarDirectorio(u_int16_t parent, t_list* directorio){
	int i;
		for(i=0;i<2048;i++){
			if(osada_drive.directorio[i].parent_directory == parent && osada_drive.directorio[i].state!=0){
				borrarSubdirectoriosYArchivos(directorio);
				rmdir(directorio); // no estoy seguro si puede aplicarse el remove al directorio o solo se lo hace a archivos
			}
			}
}


void osada_TA_renombrarArchivo(char* nombre, u_int16_t parent,char* nuevoNombre){
	int i;
	for(i=0;i<2048;i++){
	if(compare(i, nombre) && osada_drive.directorio[i].parent_directory == parent){
		rename(nombre, nuevoNombre); //funciona como rename (const char *__old, const char *__new)
	}
	}
}
