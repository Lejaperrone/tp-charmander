/*
 * mutex.c
 *
 *  Created on: 11/12/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "../commons/declarations.h"

void mutex_init(){
	osada_mutex.countBlocks = osada_drive.header->fs_blocks;

	int i;
	for(i=0; i<2048; i++){
		pthread_mutex_init(&osada_mutex.directorio[i],NULL);
	}

	osada_mutex.block = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*osada_mutex.countBlocks);
	for(i=0; i<osada_mutex.countBlocks; i++){
		pthread_mutex_init(&osada_mutex.block[i],NULL);
	}
}

void mutex_lockFile(int indice){
	if(indice != 0xFFFF && indice !=-1){
		pthread_mutex_lock(&osada_mutex.directorio[indice]);
		int indiceBloque = osada_drive.directorio[indice].first_block;

		while(indiceBloque != 0xFFFF && indiceBloque != -1){
			pthread_mutex_lock(&osada_mutex.block[indiceBloque]);
			indiceBloque = osada_drive.asignaciones[indiceBloque];
		}
	}
}

void mutex_unlockFile(int indice){
	if(indice != 0xFFFF && indice !=-1){
		int indiceBloque = osada_drive.directorio[indice].first_block;

		while(indiceBloque != 0xFFFF && indiceBloque != -1){
			pthread_mutex_unlock(&osada_mutex.block[indiceBloque]);
			indiceBloque = osada_drive.asignaciones[indiceBloque];
		}

		pthread_mutex_unlock(&osada_mutex.directorio[indice]);
	}
}

void mutex_lockFileTA(int indice){
	if(indice != 0xFFFF && indice !=-1){
		pthread_mutex_lock(&osada_mutex.directorio[indice]);
	}
}

void mutex_unlockFileTA(int indice){
	if(indice != 0xFFFF && indice !=-1){
		pthread_mutex_unlock(&osada_mutex.directorio[indice]);
	}
}
