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

int compare(int indice, char* test2){
	if(strcmp((char*)osada_drive.directorio[indice].fname, test2)==0){
		return 1;
	}
	return 0;
}
int osada_TA_buscarRegistroPorNombre(char* nombre, u_int16_t parent){
	if(parent>=0){
		int i;
		for(i=0;i<2048;i++){
			if(osada_drive.directorio[i].parent_directory == parent && compare(i, nombre)){
				return i;
			}else{
			}
		}
	}
	return -1;
}
int osada_TA_obtenerIndiceTA(char* path){
	u_int16_t childOf = 0xFFFF;
	int newChildOf;
	if (strcmp(path,"/")==0){
		return childOf;
	}else{
		char** dirc = string_split(path, "/");
		int i=0;
		while(dirc[i]!=NULL){
			newChildOf = osada_TA_buscarRegistroPorNombre(dirc[i], childOf);
			if(newChildOf>=0){
				childOf = newChildOf;
			}else{
				return -1;
			}
			free(dirc[i]);
			i++;
		}

		free(dirc);
		return childOf;
	}
}

void osada_TA_obtenerAttr(u_int16_t indice, file_attr* attr){
	if(indice>=0){
		attr->file_size = osada_drive.directorio[indice].file_size;
		attr->state = osada_drive.directorio[indice].state;
	}
}

void osada_TA_obtenerDirectorios(u_int16_t parent, t_list* directorio){
	int i;
	for(i=0;i<2048;i++){
		//state=2
		if(osada_drive.directorio[i].parent_directory == parent && osada_drive.directorio[i].state!=0){
			list_add(directorio, osada_drive.directorio[i].fname);
			log_info(logPokedexServer,"Agrego a la lista de directorios %s", osada_drive.directorio[i].fname);
		}
	}

}

int osada_TA_obtenerDirectorioLibre(){
	int i;
	for (i=0;i<2048;i++){
		if(osada_drive.directorio[i].state!=DIRECTORY && osada_drive.directorio[i].state!=REGULAR){
			return i;
		}
	}
	return -ENOMEM;
}
void osada_TA_splitPathAndName(char* path, char** name, char** pathFrom){
	char** vectorPath=string_split(path,"/");
	if (vectorPath[1]==NULL){
		string_append(pathFrom,"/");
		string_append(name,vectorPath[0]);
	}else{
		int i=0;
		while(vectorPath[i]!=NULL){
			if(vectorPath[i+1]==NULL){
				string_append(name,vectorPath[i]);
			}
			string_append(pathFrom,vectorPath[i]);
			string_append(pathFrom,"/");
		}
	}
}
void osada_TA_createNewDirectory(char* path, int posicionEnTablaArchivos){
	time_t timer=time(0);
	char* fileName=string_new();
	char* directoryName=string_new();
	osada_TA_splitPathAndName(path,&fileName, &directoryName);

	log_info(logPokedexServer, "OSADA - Generacion nuevo archivo: El nombre del archivo es: %s\n",fileName);
	osada_drive.directorio[posicionEnTablaArchivos].file_size=0;
	log_info(logPokedexServer,"El archivo %s de la path %s ocupa %d bytes",fileName,directoryName,osada_drive.directorio[posicionEnTablaArchivos].file_size);
	osada_drive.directorio[posicionEnTablaArchivos].first_block=0xFFFF;
	log_info(logPokedexServer,"El nombre inicial es %s",osada_drive.directorio[posicionEnTablaArchivos].fname);
	char* barrasCero=string_repeat('\0',17);
	memcpy(osada_drive.directorio[posicionEnTablaArchivos].fname,barrasCero,17);
	log_info(logPokedexServer,"El nombre inicial despues del memcpy es %s",osada_drive.directorio[posicionEnTablaArchivos].fname);
	osada_drive.directorio[posicionEnTablaArchivos].parent_directory=osada_TA_obtenerIndiceTA(directoryName);
	log_info(logPokedexServer,"El directorio padre de %s es %d",fileName,osada_drive.directorio[posicionEnTablaArchivos].parent_directory);
	int i;
	for (i=0;i<17;i++){
		osada_drive.directorio[posicionEnTablaArchivos].fname[i]=fileName[i];
	}
	log_info(logPokedexServer,"El nombre final es %s",osada_drive.directorio[posicionEnTablaArchivos].fname);
	osada_drive.directorio[posicionEnTablaArchivos].state=REGULAR;
	osada_drive.directorio[posicionEnTablaArchivos].lastmod=timer;
	free(fileName);
	free(directoryName);
}

//FUNCIONES Q SE USAN

//agrega a la lista de directorios los directorios de esa path

int darDeAltaDirectorioEnTablaDeArchivos(char* nombre,int indice){
	int i;
	int yaLoGuarde=0;
	for (i=0;i<2048;i++){
		if(yaLoGuarde==0 && osada_drive.directorio[i].state==0){
			yaLoGuarde=1;
			char* fecha=string_new();
			time_t timer=time(0);
			struct tm *tlocal = localtime(&timer);
			osada_drive.directorio[i].file_size=0;
			strcpy((char*)osada_drive.directorio[i].fname, nombre);
			strftime(fecha,128,"%d/%m/%y %H:%M:%S",tlocal);
			osada_drive.directorio[i].lastmod=atoi(fecha);
			osada_drive.directorio[i].parent_directory=indice;
			osada_drive.directorio[i].state=2;
			osada_drive.directorio[i].first_block=0xFFFF;
		}
	}
	return yaLoGuarde;

}





void osada_TA_setearAttr(u_int16_t indice, file_attr* attr){
	attr->file_size=0;
	attr->state=0;
}


int osada_TA_borrarArchivo(u_int16_t parent){
	int subindice;
	strcpy((char*)osada_drive.directorio[parent].fname,"");
	log_info(logPokedexServer, "El fname del directorio del bloque %d es -%s-", parent, osada_drive.directorio[parent].fname);
	osada_drive.directorio[parent].state=0;
	osada_drive.directorio[parent].lastmod=time(NULL);
	u_int16_t fin = 0xFFFF;
	subindice=osada_drive.directorio[parent].first_block;
	log_info(logPokedexServer, "El subindice (primer bloque del parent) es %d", subindice);
	while (subindice!=fin && subindice != -1){
		bitarray_clean_bit(osada_drive.bitmap,subindice);
		obtenerProximoBloque(&subindice);
		log_info(logPokedexServer, "Ahora el subindice es: %d", subindice);
	}
	log_info(logPokedexServer, "Se pudieron limpiar los bits del bitarray correctamente");
	return 1;
}



void osada_TA_borrarDirectorio(u_int16_t parent){
	pthread_mutex_lock(&mutexTablaArchivos);
	osada_drive.directorio[parent].state=0;
	pthread_mutex_unlock(&mutexTablaArchivos);
}

