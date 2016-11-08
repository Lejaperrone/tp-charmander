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
#include <time.h>

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
void darDeAltaDirectorioEnTablaDeArchivos(char* nombre,int indice){
	int i;
	int yaLoGuarde=0;
	for (i=0;i<2048;i++){
		if(yaLoGuarde==0 && osada_drive.directorio[i].state==0){
			yaLoGuarde=1;
			char* fecha=string_new();
			time_t timer=time(0);
			struct tm *tlocal = localtime(&timer);
			osada_drive.directorio[i].file_size=0;
			//osada_drive.directorio[i].first_block=NULL;
			strcpy((char*)osada_drive.directorio[i].fname,nombre);
			strftime(fecha,128,"%d/%m/%y %H:%M:%S",tlocal);
			osada_drive.directorio[i].lastmod=atoi(fecha);
			// no sabemos si el directorio padre de un directorio nuevo es 0xFFFF u otro
			//parent directory es el subindice del ultimo hijo
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


int osada_TA_borrarArchivo(u_int16_t parent){
	int subindice;
	u_int16_t fin = 0xFFFF;
	subindice=osada_drive.directorio[parent].first_block;
	while (subindice!=fin){
		bitarray_clean_bit(osada_drive.bitmap,subindice);
		obtenerProximoBloque(&subindice);
	}
	return 1;

}


void osada_TA_borrarDirectorio(u_int16_t parent){
	osada_drive.directorio[parent].state=0;
}


void osada_TA_renombrarArchivo(char* nombre, u_int16_t parent,char* nuevoNombre){
	int i;
	for(i=0;i<2048;i++){
	if(compare(i, nombre) && osada_drive.directorio[i].parent_directory == parent){
		rename(nombre, nuevoNombre); //funciona como rename (const char *__old, const char *__new)
	}
	}
}
