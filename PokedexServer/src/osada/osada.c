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
#include <commons/string.h>
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
	 return 1;
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
bool elBufferTieneDatosParaEscribir(char* buf){
return strlen(buf)>0;
}
void actualizarBuffer(char* buffer, char* bufUp, int bytesEscritos){
	int tamanioBufferOriginal=strlen(buffer);
	int i;
	int j=0;
	for (i=bytesEscritos;i<tamanioBufferOriginal;i++){
		bufUp[j]=buffer[i];
		j++;
	}
}

int actualizarBytesEscritos (int acum, int bytes){
	return acum+=bytes;
}
int osada_write(char* path,char* buf, size_t size, off_t offset){
	u_int16_t indice = osada_TA_obtenerUltimoHijoFromPath(path);
	int bytesEscritos=0;
	if (!superaTamanioArchivo(indice,offset,size)){
		int bloque=osada_drive.directorio[indice].first_block;
		double desplazamientoHastaElBloque=ceil(offset/OSADA_BLOCK_SIZE);
		int bloqueArranque=avanzarBloquesParaEscribir(bloque,desplazamientoHastaElBloque);
		int byteComienzoEscritura=offset-(desplazamientoHastaElBloque*OSADA_BLOCK_SIZE);
		char* bufUpdated=string_new();
		strcpy(bufUpdated,buf);
		while (elBufferTieneDatosParaEscribir(bufUpdated)){
				bitarray_set_bit(osada_drive.bitmap,bloqueArranque);
				memcpy(osada_drive.data[bloqueArranque*OSADA_BLOCK_SIZE+byteComienzoEscritura],buf,OSADA_BLOCK_SIZE-byteComienzoEscritura);
				actualizarBuffer(buf,bufUpdated,OSADA_BLOCK_SIZE-byteComienzoEscritura);
				actualizarBytesEscritos(bytesEscritos,OSADA_BLOCK_SIZE-byteComienzoEscritura);
				byteComienzoEscritura=0;
				bloqueArranque=avanzarBloquesParaEscribir(bloqueArranque,1);
		}
	}else{
		bytesEscritos=-ENOMEM;
	}
	return bytesEscritos;
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
	return strlen(buf);
	}else{
		return -ENOMEM;
	}


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

bool esUnArchivo(int subindice){
	return osada_drive.directorio[subindice].state==1;
}
bool estaBorrado(int subindice){
	return osada_drive.directorio[subindice].state==0;
}
bool esUnDirectorio(int subindice){
	return osada_drive.directorio[subindice].state==2;
}

int renombrarArchivo (int subindice, char* newFileName){
	int resultado;
	if (esUnArchivo(subindice)){
		if (strlen(strcpy((char*)osada_drive.directorio[subindice].fname,newFileName))==strlen(newFileName)){
				resultado= 1;
			}else{
				resultado= 0;
			}
	}else{
		if (estaBorrado(subindice)){
			resultado=ENOENT;
		}
		if (esUnDirectorio(subindice)){
			resultado=EISDIR;
		}
	}
	return resultado;

}
char* getFileNameFromPath(char* path, char** pathSplitteada, char* nombre){
		pathSplitteada=string_split(path,"/");
		nombre=pathSplitteada[strlen(*pathSplitteada)-2];
		return nombre;
}
int osada_rename(char* path, char* nuevaPath){
	int resultado;
	int subindice=osada_TA_obtenerUltimoHijoFromPath(path);
	char* nombre=string_new();
	char** pathSplitteada=(char**)malloc(sizeof(char*));
	getFileNameFromPath(nuevaPath,pathSplitteada, nombre);
	if (renombrarArchivo(subindice,nombre)==1){
		resultado= 1;
	}else{
		resultado=ENOMEM;
	}
	free(pathSplitteada);
	free(nombre);
	return resultado;
}
