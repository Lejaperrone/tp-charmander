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
#include <commons/bitarray.h>
#include <sys/stat.h>
#include <math.h>
#include "functions/tabla_asignaciones.h"
int osada_init(char* path){
	initOsada (path);
	return 1;
}

int osada_removeDir(char* path){
	int pudeBorrar=1;
	t_list* directoriosQueComponenElActual=list_create();
	u_int16_t parent = osada_TA_obtenerUltimoHijoFromPath(path);
	osada_TA_obtenerDirectorios(parent, directoriosQueComponenElActual);
	if (list_is_empty(directoriosQueComponenElActual)){
		osada_TA_borrarDirectorio(parent);
	}else{
		perror("NO se pudo remover el directorio porque no esta vacio");
		pudeBorrar=0;
	}
	list_destroy(directoriosQueComponenElActual);
	return pudeBorrar;
}

int osada_removeFile(char* path){
	u_int16_t parent=osada_TA_obtenerUltimoHijoFromPath(path);
	 osada_TA_borrarArchivo(parent);
}
int osada_readdir(char* path, t_list* directorios){
	//Verifico si  el path que me pasan existe y obtengo el indice del ultimo hijo
		u_int16_t parent = osada_TA_obtenerUltimoHijoFromPath(path);
	//Obtengo los directorios
		osada_TA_obtenerDirectorios(parent, directorios);
	//Return
		return 1;
}

/*int osada_createFile(char* path, char* nombreArchivo){

}
int osada_listarArchivos(char* path){

}*/

int osada_getattr(char* path, file_attr* attrs){
	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);

	if(indice>=0){
		//el indice es numero de posicion en la que esta en la tabla de archivos, si indice vale 6, esta en la
		//posicion 6 de la tabla de achivos
		osada_TA_obtenerAttr(indice, attrs);
		return 1;
	}else if(strcmp(path,"/")){
		attrs->file_size = 0;
		attrs->state = 2;
		return 1;
	}else{
		return -ENOENT;
	}
}
//Path es la ruta al archivo
//Buf viene vacio y se llena con la info que hay en la path
//size es cuantos bytes del archivo hay que leer
//offset es desde donde tengo que leer

bool superaTamanioArchivo (int indice, off_t offset, size_t size){
	return size>(osada_drive.directorio[indice].file_size-offset);
}

int osada_read(char *path, char *buf, size_t size, off_t offset){
	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);
	if (!superaTamanioArchivo(indice,offset,size)){
	//con el indice voy a TA y busco el FB
	int bloque=osada_drive.directorio[indice].first_block;
	//offset/TAMBLQ= R ,rrdondearlo para arriba y restarle 1-->2
	double desplazamientoHastaElBloque=ceil(offset/OSADA_BLOCK_SIZE);
	//Voy a FB y avanzo 2 dentro de Tasignaciones
	int bloqueArranque=avanzarBloquesParaLeer(bloque,desplazamientoHastaElBloque);
	//RDO=ofsset-(RxBSIZE)=cuando llegue al bloque solicitado hago *data (en declarations.h) y me muevo (se sumo) RDO
	int byteComienzoLectura=offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
	while (bloqueArranque!=0xFFFFFFFF){
		//falta chequear inicio
		memcpy(buf,osada_drive.data[bloqueArranque*OSADA_BLOCK_SIZE+byteComienzoLectura],OSADA_BLOCK_SIZE-byteComienzoLectura);
		bloqueArranque=osada_drive.asignaciones[bloque];
		byteComienzoLectura=0;
	}
	}else{
		return -ENOMEM;
	}
	return strlen(buf);

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
int hayBloquesLibres(t_list* listaDeBloques, int bloquesNecesarios){
	bool noMeAlcanzan=true;
	int tam=bitarray_get_max_bit(osada_drive.bitmap);

	int i;
	while(noMeAlcanzan){
			int posicionEnBitmap=0;
			if(bitarray_test_bit(osada_drive.bitmap,posicionEnBitmap)){
				if (posicionEnBitmap>=tam){
					//perror("No hay bloques libres en el bitmap");
					//el for se hace por si no me alcanzan los bloques, los libero
					for (i=0;i<list_size(listaDeBloques);i++){
						bitarray_clean_bit(osada_drive.bitmap,(int)list_get(listaDeBloques,i));
					}
					return -ENOSPC;
				}else{
				posicionEnBitmap++;
				}
			}else{
				list_add(listaDeBloques,(void*)posicionEnBitmap);
				bitarray_set_bit(osada_drive.bitmap,posicionEnBitmap);
				if(list_size(listaDeBloques)==bloquesNecesarios){
				noMeAlcanzan=false;
				}

			}
		}
	return 1;
}

int osada_createDir(char* path, char* name){
	int subindice=osada_TA_obtenerUltimoHijoFromPath(path);
//aca hay que obtener el hijo del ultimo path/ parametro es el path
		darDeAltaDirectorioEnTablaDeArchivos(name, subindice);
}
