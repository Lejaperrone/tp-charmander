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

int osada_TA_compareNameToIndex(int indice, char* test2){
	if(strcmp((char*)osada_drive.directorio[indice].fname, test2)==0){
		return 1;
	}
	return 0;
}
int osada_TA_buscarRegistroPorNombre(char* nombre, u_int16_t parent){
	log_info(logPokedexServer,"Nombre vale %s", nombre);
	if(parent>=0){
		int i;
		for(i=0;i<2048;i++){
			if(osada_drive.directorio[i].parent_directory == parent && osada_TA_compareNameToIndex(i, nombre)){
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
			log_info(logPokedexServer,"dirc[i] vale %s",dirc[i]);
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
			pthread_mutex_lock(&osada_mutex.directorio[i]);
			list_add(directorio, osada_drive.directorio[i].fname);
			pthread_mutex_unlock(&osada_mutex.directorio[i]);
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
			}else{
				string_append(pathFrom,"/");
				string_append(pathFrom,vectorPath[i]);
			}
			i++;
		}
	}
}
int osada_TA_createNewDirectory(char* path, osada_file_state state){
	char* fileName=string_new();
	char* directoryName=string_new();
	osada_TA_splitPathAndName(path,&fileName, &directoryName);

	int guardado= 0;
	if(string_length(fileName)<=17){
		int i;
		for (i=0;(i<2048 && guardado==0);i++){
			if(osada_drive.directorio[i].state==0){
				log_info(logPokedexServer,"BLOQUEO EL ELEMENTO %d DE TA",i);
				//pthread_mutex_lock(&osada_mutex.directorio[i]);
				printf("BLOQUEE EL EMENE");
				osada_drive.directorio[i].file_size=0;
				osada_drive.directorio[i].first_block=0xFFFF;
				strcpy((char*)osada_drive.directorio[i].fname, fileName);
				osada_drive.directorio[i].lastmod=(int)time(NULL);
				osada_drive.directorio[i].parent_directory=osada_TA_obtenerIndiceTA(directoryName);
				osada_drive.directorio[i].state=state;
				log_info(logPokedexServer,"Salgo del semaforo");
				//pthread_mutex_unlock(&osada_mutex.directorio[i]);
				log_info(logPokedexServer,"Ya sali del semaforo para %d",i);
				guardado=1;
			}
		}
	}

	free(fileName);
	free(directoryName);

	return guardado;
}

bool osada_TA_TArchivo(int subindice){
	return osada_drive.directorio[subindice].state==1;
}

bool osada_TA_TBorrado(int subindice){
	return osada_drive.directorio[subindice].state==0;
}

bool osada_TA_TDirectorio(int subindice){
	return osada_drive.directorio[subindice].state==2;
}

int osada_TA_cantRegistrosLibres(){
	int i, tot=0;
	for (i=0;i<2048;i++){
		if(osada_TA_TBorrado(i)){
			tot++;
		}
	}
	return tot;
}



//FUNCIONES Q SE USAN

//agrega a la lista de directorios los directorios de esa path





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

