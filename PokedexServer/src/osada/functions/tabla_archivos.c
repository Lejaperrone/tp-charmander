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
#include "mutex.h"


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
			if(osada_drive.directorio[i].state!=DELETED && osada_drive.directorio[i].parent_directory == parent && osada_TA_compareNameToIndex(i, nombre)){
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
	}else if(strcmp(path, "") == 0){
		return -1;
	}else{
		char** dirc = string_split(path, "/");
		int i=0;
		while(dirc[i]!=NULL){
			newChildOf = osada_TA_buscarRegistroPorNombre(dirc[i], childOf);
			if(newChildOf>=0){
				childOf = newChildOf;
			}else{
				free(dirc[i]);	//ESTE FREE ESTA BIEN QUE TAMBIEN VAYA ACA? Porque antes no estaba
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
		pthread_mutex_lock(&osada_mutex.directorio[indice]);
		attr->file_size = osada_drive.directorio[indice].file_size;
		attr->state = osada_drive.directorio[indice].state;
		attr->lastmod = osada_drive.directorio[indice].lastmod;
		pthread_mutex_unlock(&osada_mutex.directorio[indice]);
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
	int fileExist = osada_TA_obtenerIndiceTA(path);
	log_info(logPokedexServer, "El indice del fileExist es: %d", fileExist);
	if(fileExist == -1){
		char* fileName=string_new();
		char* directoryName=string_new();
		osada_TA_splitPathAndName(path,&fileName, &directoryName);

		int guardado= 0;
		if(string_length(fileName)<=17){
			log_info(logPokedexServer, "El string_length del fileName es: %d", string_length(fileName));
			int i;
			for (i=0;(i<2048 && guardado==0);i++){
				if(osada_drive.directorio[i].state==0){
					pthread_mutex_lock(&osada_mutex.directorio[i]);
					osada_drive.directorio[i].file_size=0;
					osada_drive.directorio[i].first_block=0xFFFF;
					strcpy((char*)osada_drive.directorio[i].fname, fileName);
					time_t currentTime;
					osada_drive.directorio[i].lastmod=(int)time(&currentTime);
					log_info(logPokedexServer, "Fecha: %s", ctime(&currentTime));
					osada_drive.directorio[i].parent_directory=osada_TA_obtenerIndiceTA(directoryName);
					osada_drive.directorio[i].state=state;
					pthread_mutex_unlock(&osada_mutex.directorio[i]);
					guardado=1;
				}
			}
		}

		free(fileName);
		free(directoryName);

		log_info(logPokedexServer, "La variable guardado es: %d", guardado);
		return guardado;
	}
	return 0;
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

void osada_TA_deleteDirectory(u_int16_t indice, osada_file_state state){
	if(state == REGULAR){
		mutex_lockFile(indice);
		int block = osada_drive.directorio[indice].first_block;
		int nextBlock;
		while(block != 0xFFFF && block != -1){
			nextBlock = osada_drive.asignaciones[block];
			bitarray_clean_bit(osada_drive.bitmap,block);
			block=nextBlock;
		}
	}
	osada_drive.directorio[indice].state=0;
	osada_drive.directorio[indice].lastmod = (int)time(NULL);
	mutex_unlockFile(indice);
}

