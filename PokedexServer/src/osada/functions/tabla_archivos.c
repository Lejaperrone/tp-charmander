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
#include "../../commons/definitions.h"
#include "../commons/osada.h"
#include <errno.h>
#include "../functions/tabla_asignaciones.h"
#include <time.h>
#include <pthread.h>

extern pthread_mutex_t mutexTablaArchivos;
extern pthread_mutex_t mutexBitmap;

int compare(int indice, char* test2){
	int i;
	for(i=0; i<OSADA_FILENAME_LENGTH; i++){
		log_info(logPokedexServer, "%c - %c\n", osada_drive.directorio[indice].fname[i], *test2);
		if(osada_drive.directorio[indice].fname[i] == '\0' && *test2=='\0'){
			log_info(logPokedexServer, "Va a retornar 1");
			return 1;
		}else{
			if(osada_drive.directorio[indice].fname[i] != *test2){
				log_info(logPokedexServer, "Va a retornar 0");
				return 0;
			}
		}

		test2++;
	}
	log_info(logPokedexServer, "Va a retornar 0");
	return 0;
}

u_int16_t osada_TA_buscarRegistroPorNombre(char* nombre, u_int16_t parent){
	int i;
	for(i=0;i<2048;i++){
		if(compare(i, nombre) && osada_drive.directorio[i].parent_directory == parent){
			log_info(logPokedexServer, "El indice es: %s", i);
			return i;
		}
	}

	return -1;
}

//agrega a la lista de directorios los directorios de esa path
void osada_TA_obtenerDirectorios(u_int16_t parent, t_list* directorio){
	int i;
	pthread_mutex_lock(&mutexTablaArchivos);
	for(i=0;i<2048;i++){
		//state=2
		if(osada_drive.directorio[i].parent_directory == parent && osada_drive.directorio[i].state!=0){
			list_add(directorio, osada_drive.directorio[i].fname);
		}
		pthread_mutex_unlock(&mutexTablaArchivos);
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
	char** dirc = string_split(path, "/");
	u_int16_t child = 0xFFFF;
	int i=0;
	log_info(logPokedexServer, "OSADA - Se va a recorrer el vector de strings separados del path");
	while(dirc[i]!=NULL){
		if(string_length(dirc[i]) != 0){
			log_info(logPokedexServer, "OSADA - Se va a buscar el registro por nombre");
			child = osada_TA_buscarRegistroPorNombre(dirc[i], child);
			if(child == -1){
				log_info(logPokedexServer, "obtenerUltimoHijoFromPath devuelve ENOENT");
				return -ENOENT;
			}
		}
		i++;
	}

	return child;
}

void osada_TA_obtenerAttr(u_int16_t indice, file_attr* attr){

	log_info(logPokedexServer, "OSADA - El file_size de la estructura attr que me llega es: %d", attr->file_size);
	attr->file_size = osada_drive.directorio[indice].file_size;
	log_info(logPokedexServer, "OSADA - Ahora el file_size de la estructura attr es: %d", attr->file_size);
	attr->state = osada_drive.directorio[indice].state;

}
void osada_TA_setearAttr(u_int16_t indice, file_attr* attr){
	attr->file_size=0;
	attr->state=0;
}


int osada_TA_borrarArchivo(u_int16_t parent){
	int subindice;
	u_int16_t fin = 0xFFFF;
	pthread_mutex_lock(&mutexTablaArchivos);
	subindice=osada_drive.directorio[parent].first_block;
	pthread_mutex_unlock(&mutexTablaArchivos);
	while (subindice!=fin){
		pthread_mutex_lock(&mutexBitmap);
		bitarray_clean_bit(osada_drive.bitmap,subindice);
		pthread_mutex_unlock(&mutexBitmap);
		obtenerProximoBloque(&subindice);
	}
	return 1;
}


void osada_TA_borrarDirectorio(u_int16_t parent){
	pthread_mutex_lock(&mutexTablaArchivos);
	osada_drive.directorio[parent].state=0;
	pthread_mutex_unlock(&mutexTablaArchivos);
}


void osada_TA_renombrarArchivo(char* nombre, u_int16_t parent,char* nuevoNombre){
	int i;
	for(i=0;i<2048;i++){
	if(compare(i, nombre) && osada_drive.directorio[i].parent_directory == parent){
		rename(nombre, nuevoNombre); //funciona como rename (const char *__old, const char *__new)
	}
	}
}
